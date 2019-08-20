#include <Windows.h>
#include <assert.h>
#include <emmintrin.h>
#include <immintrin.h>
#include <atomic>
#include <cstring>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <tuple>
#undef CreateWindow
#undef min
#undef max
using QWORD = __int64;
#include "MemoryLeakDetection.h"
#include "SmallSizeUtility.h"

inline namespace MemoryGlobals
{
        constexpr auto KiB        = 1024;
        constexpr auto MiB        = KiB * 1024;
        constexpr auto GiB        = MiB * 1024;
        constexpr auto CACHE_LINE = std::hardware_destructive_interference_size;
} // namespace MemoryGlobals

inline namespace RandomGlobals
{
        inline thread_local std::mt19937 g_tl_defaultRandomEngine;
}
inline namespace RandomInternal
{
        std::tuple<uint32_t, uint32_t> FeistelCoder(std::tuple<uint32_t, uint32_t> v)
        {
                uint32_t k[4]{0xA341316C, 0xC8013EA4, 0xAD90777D, 0x7E95761E};
                uint32_t sum   = 0;
                uint32_t delta = 0x9E3779B9;
                for (unsigned i = 0; i < 4; i++)
                {
                        sum += delta;
                        std::get<0>(v) +=
                            ((std::get<1>(v) << 4) + k[0]) ^ (std::get<1>(v) + sum) ^ ((std::get<1>(v) >> 5) + k[1]);
                        std::get<1>(v) +=
                            ((std::get<0>(v) << 4) + k[2]) ^ (std::get<0>(v) + sum) ^ ((std::get<0>(v) >> 5) + k[3]);
                }
                return v;
        }
} // namespace RandomInternal

inline namespace TagIntegrals
{
        template <typename... Ts>
        struct percent
        {
                std::tuple<Ts...> floats;
                percent(Ts... ts)
                {
                        floats = std::tuple(ts...);
                }
        };
        template <typename T1>
        percent(T1 t1)->percent<float>;
        template <typename T1, typename T2>
        percent(T1 t1, T2 t2)->percent<float, float>;
        template <typename T1, typename T2, typename T3>
        percent(T1 t1, T2 t2, T3 t3)->percent<float, float, float>;
} // namespace TagIntegrals

inline namespace WindowsGlobals
{
        inline HINSTANCE g_hInstance = 0;
        inline LPSTR     g_pCmdLine  = 0;
        inline int       g_nCmdShow  = 0;
        inline LRESULT CALLBACK g_defaultWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        inline auto             g_screenWidth  = GetSystemMetrics(SM_CXSCREEN);
        inline auto             g_screenHeight = GetSystemMetrics(SM_CYSCREEN);
} // namespace WindowsGlobals
inline namespace WindowsInternal
{
        using pWindowProc = LRESULT (*)(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        struct WindowCreationDescriptor
        {
                char        WindowTitle[32];
                char        WindowClassName[32];
                HWND        hwnd;
                pWindowProc windowProc;
                int         x;
                int         y;
                int         width;
                int         height;
                HWND        parent;
                HMENU       menu;
                DWORD       style;
                DWORD       exstyle;
        };
} // namespace WindowsInternal
inline namespace Windows
{
        struct Window
        {
            private:
                HWND hwnd;

            public:
                Window(HWND hwnd) : hwnd(hwnd)
                {}
                void Show()
                {
                        ShowWindow(hwnd, SW_SHOWDEFAULT);
                }
        };
        struct CreateWindow
        {
            private:
                WindowCreationDescriptor windowCreationDescriptor;

            public:
                CreateWindow()
                {
                        memset(&windowCreationDescriptor, 0, sizeof(windowCreationDescriptor));
                        windowCreationDescriptor.windowProc = g_defaultWindowProc;
                        windowCreationDescriptor.style      = WS_OVERLAPPED;
                        windowCreationDescriptor.x          = CW_USEDEFAULT;
                        windowCreationDescriptor.y          = CW_USEDEFAULT;
                        windowCreationDescriptor.width      = CW_USEDEFAULT;
                        windowCreationDescriptor.height     = CW_USEDEFAULT;
                        windowCreationDescriptor.style      = WS_OVERLAPPED;
                        strcpy_s(windowCreationDescriptor.WindowClassName, std::to_string(g_tl_defaultRandomEngine()).c_str());
                }
                CreateWindow& Title(const char* title)
                {
                        strcpy_s(windowCreationDescriptor.WindowTitle, title);
                        return *this;
                }
                CreateWindow& Position(int x, int y)
                {
                        windowCreationDescriptor.x = x;
                        windowCreationDescriptor.y = y;
                        return *this;
                }
                CreateWindow& Position(percent<float, float> pos)
                {
                        windowCreationDescriptor.x = std::lroundf((std::get<0>(pos.floats) / 100) * g_screenWidth);
                        windowCreationDescriptor.y = std::lroundf((std::get<1>(pos.floats) / 100) * g_screenHeight);
                        return *this;
                }
                CreateWindow& WindProc(pWindowProc wndProc)
                {
                        windowCreationDescriptor.windowProc = wndProc;
                        return *this;
                }
                CreateWindow& Size(int width, int height)
                {
                        windowCreationDescriptor.width  = width;
                        windowCreationDescriptor.height = height;
                        return *this;
                }
                CreateWindow& Size(percent<float, float> size)
                {
                        windowCreationDescriptor.width  = (std::get<0>(size.floats) / 100) * g_screenWidth;
                        windowCreationDescriptor.height = (std::get<1>(size.floats) / 100) * g_screenHeight;
                        return *this;
                }
                Window Finalize()
                {
                        WNDCLASS wc          = {};
                        wc.lpfnWndProc       = windowCreationDescriptor.windowProc;
                        wc.hInstance         = g_hInstance;
                        wc.lpszClassName     = windowCreationDescriptor.WindowClassName;
                        auto standard_cursor = LoadCursor(0, IDC_ARROW);
                        wc.hCursor           = standard_cursor;


                        DWORD err;
                        if (!RegisterClassA(&wc))
                                err = GetLastError();
                        int pause = 0;

                        auto hwnd = CreateWindowExA(windowCreationDescriptor.exstyle,         // Optional window styles.
                                                    windowCreationDescriptor.WindowClassName, // Window class
                                                    windowCreationDescriptor.WindowTitle,     // Window text
                                                    WS_OVERLAPPEDWINDOW,                      // Window style

                                                    windowCreationDescriptor.x,
                                                    windowCreationDescriptor.y,
                                                    windowCreationDescriptor.width,
                                                    windowCreationDescriptor.height,

                                                    windowCreationDescriptor.parent, // Parent window
                                                    windowCreationDescriptor.menu,   // Menu
                                                    g_hInstance,                     // Instance handle
                                                    0                                // Additional application data
                        );
                        return Window(hwnd);
                }
        };
} // namespace Windows

inline namespace Console
{
        inline void Initialize()
        {
                AllocConsole();
                auto success = freopen("CONOUT$", "w", stdout);
        }
        inline void DisableQuickEdit()
        {
                HANDLE hConsole = GetStdHandle(STD_INPUT_HANDLE);
                DWORD  mode;
                if (!GetConsoleMode(hConsole, &mode))
                {
                        // error getting the console mode. Exit.
                        return;
                }
                mode = mode & ~(ENABLE_QUICK_EDIT_MODE | ENABLE_EXTENDED_FLAGS);
                if (!SetConsoleMode(hConsole, mode))
                {
                        // error setting console mode.
                }
        }
        inline void EnableQuickEdit()
        {
                auto  conHandle = GetStdHandle(STD_INPUT_HANDLE);
                DWORD mode;
                if (!GetConsoleMode(conHandle, &mode))
                {
                        // error getting the console mode. Exit.
                        return;
                }
                mode = mode | (ENABLE_QUICK_EDIT_MODE | ENABLE_EXTENDED_FLAGS);
                if (!SetConsoleMode(conHandle, mode))
                {
                        // error setting console mode.
                }
        }
        inline void Shutdown()
        {
                fclose(stdout);
                FreeConsole();
        }
} // namespace Console

#define DBGRawMouseInput
inline namespace RawInput
{
        inline namespace RawInputGlobals
        {
                constexpr auto MICKEY = KiB * 64;
        }
        inline namespace RawInputInterface
        {
                struct InputFrame;
        } // namespace RawInputInterface
        inline namespace RawInputInternal
        {
                // for initializing previous mouse x and y only
                POINT GetCursorInitialPos()
                {
                        POINT p;
                        GetCursorPos(&p);
                        return p;
                }
                int MickeyToScreenPosX(int value)
                {
                        return (static_cast<float>(value) / MICKEY) * g_screenWidth;
                }
                int MickeyToScreenPosY(int value)
                {
                        return (static_cast<float>(value) / MICKEY) * g_screenHeight;
                }
        } // namespace RawInputInternal
#ifdef DBGRawMouseInput
        inline namespace RawInputDebugGlobals
        {
                uint64_t g_dbg_prevInputFrameCounter = -1;
                uint64_t g_dbg_inputFrameCounter     = 0;
        } // namespace RawInputDebugGlobals
        inline namespace RawInputDebug
        {
                void DbgIncrementInputFrame();
                void DbgStoreInputFrameCounter(InputFrame*);
                void DbgDumpFrameData(InputFrame*, int, int);
        } // namespace RawInputDebug_ACTIVE
#else
        inline namespace RawInputDebug_INACTIVE
        {
                void DbgIncrementInputFrame()
                {}
                void DbgStoreInputFrameCounter(InputFrame*)
                {}
                void DbgDumpFrameData(InputFrame*, int, int)
                {}
        } // namespace RawInputDebug_INACTIVE
#endif
        inline namespace RawInputGlobals
        {
                POINT cursorInitialPos = GetCursorInitialPos();
                int   g_prevX          = cursorInitialPos.x;
                int   g_prevY          = cursorInitialPos.y;
        } // namespace RawInputGlobals
        inline namespace RawInputInterface
        {
                struct alignas(CACHE_LINE) InputFrame
                {
                        static constexpr auto paddingSize = CACHE_LINE - sizeof(RAWINPUT) - sizeof(uint64_t);
                        RAWINPUT              rawInput;
                        uint64_t              frameNumber;
                        char                  padding[paddingSize];

                        bool inline IsMouseInput()
                        {
                                return rawInput.header.dwType == RIM_TYPEMOUSE;
                        }
                };
                // single producer -> single consumer  buffer
                struct InputBuffer
                {
                    private:
                        static constexpr uint64_t m_BUFFER_SIZE = 16384ULL;
                        static constexpr unsigned m_NUM_BUFFERS = 2;
                        // m_NUM_BUFFERS must be power of 2, Swap() uses mersenne prime modulation

                        volatile InputFrame* m_buffers;
                        volatile uint64_t    m_bufferSizes[m_NUM_BUFFERS];
                        unsigned             m_readBufferIndex  = 0;
                        unsigned             m_writeBufferIndex = 1;

                        void Swap()
                        {
                                m_readBufferIndex  = m_writeBufferIndex;
                                m_writeBufferIndex = (m_writeBufferIndex + 1) & (m_NUM_BUFFERS - 1);
                        }

                        bool TryPushWrite(LPARAM& RawInputlParam)
                        {
                                if (m_bufferSizes[m_writeBufferIndex] == m_BUFFER_SIZE)
                                        return false;
                                assert(m_bufferSizes[m_writeBufferIndex] < m_BUFFER_SIZE);

                                UINT dwSize = 0;

                                InputFrame& inputFrameAlias = const_cast<InputFrame&>(
                                    m_buffers[m_writeBufferIndex * m_BUFFER_SIZE + m_bufferSizes[m_writeBufferIndex]]);

                                GetRawInputData((HRAWINPUT)RawInputlParam, RID_INPUT, 0, &dwSize, sizeof(RAWINPUTHEADER));
                                GetRawInputData(
                                    (HRAWINPUT)RawInputlParam, RID_INPUT, &inputFrameAlias, &dwSize, sizeof(RAWINPUTHEADER));

                                int _x;
                                int _y;
                                if (inputFrameAlias.IsMouseInput())
                                {
                                        if (inputFrameAlias.rawInput.data.mouse.usFlags == MOUSE_MOVE_RELATIVE)
                                        {
                                                _x = inputFrameAlias.rawInput.data.mouse.lLastX;
                                                _y = inputFrameAlias.rawInput.data.mouse.lLastY;
                                                g_prevX += inputFrameAlias.rawInput.data.mouse.lLastX;
                                                g_prevY += inputFrameAlias.rawInput.data.mouse.lLastY;
                                        }
                                        else
                                        {
                                                auto _screenX = MickeyToScreenPosX(inputFrameAlias.rawInput.data.mouse.lLastX);
                                                auto _screenY = MickeyToScreenPosY(inputFrameAlias.rawInput.data.mouse.lLastY);
                                                _x            = _screenX - g_prevX;
                                                _y            = _screenY - g_prevY;
                                                g_prevX       = _screenX;
                                                g_prevY       = _screenY;
                                        }
                                        DbgStoreInputFrameCounter(&inputFrameAlias);
                                        DbgDumpFrameData(&inputFrameAlias, _x, _y);
                                }

                                m_bufferSizes[m_writeBufferIndex]++;
                                return true;
                        }

                    public:
                        // single consumer functions:
                        void ProcessReads()
                        {
                                auto sz = m_bufferSizes[m_readBufferIndex];
                                if (!sz)
                                        return;

                                for (unsigned i = 0; i < m_bufferSizes[m_readBufferIndex]; i++)
                                {
                                        InputFrame* thisFrame =
                                            const_cast<InputFrame*>(&m_buffers[m_readBufferIndex * m_BUFFER_SIZE + i]);
                                }

                                m_bufferSizes[m_readBufferIndex] = 0;
                        }

                        // single producer functions:
                        void Write(LPARAM& RawInputLParam)
                        {
                                if (!TryPushWrite(RawInputLParam))
                                {
                                        while (m_bufferSizes[m_readBufferIndex] != 0)
                                        {
                                                Sleep(0);
                                        }
                                        // TODO // Run a job while we wait?

                                        //
                                        Swap();
                                        TryPushWrite(RawInputLParam);
                                        return;
                                };
                                if (m_bufferSizes[m_readBufferIndex] == 0)
                                        Swap();

                                Sleep(0);
                        }

                        void Initialize()
                        {
                                m_buffers = (volatile InputFrame*)_aligned_malloc(
                                    m_BUFFER_SIZE * sizeof(InputFrame) * m_NUM_BUFFERS, 8);
                        }

                        void Shutdown()
                        {
                                _aligned_free(const_cast<InputFrame*>(m_buffers));
                        }
                };
        } // namespace RawInputInterface
        inline namespace RawInputGlobals
        {
                InputBuffer g_inputBuffer;
        } // namespace RawInputGlobals
        inline namespace RawInputInterface
        {
                void Initialize()
                {
                        RAWINPUTDEVICE Rid[2];

                        Rid[0].usUsagePage = 0x01;
                        Rid[0].usUsage     = 0x02;
                        Rid[0].dwFlags     = 0;
                        // RIDEV_NOLEGACY; // adds HID mouse and also ignores legacy mouse messages
                        Rid[0].hwndTarget = 0;

                        Rid[1].usUsagePage = 0x01;
                        Rid[1].usUsage     = 0x06;
                        Rid[1].dwFlags     = 0;
                        // RIDEV_NOLEGACY; // adds HID keyboard and also ignores legacy keyboard messages
                        Rid[1].hwndTarget = 0;

                        if (RegisterRawInputDevices(Rid, 2, sizeof(Rid[0])) == FALSE)
                        {
                                // registration failed. Call GetLastError for the cause of the error
                        }

                        g_inputBuffer.Initialize();
                }
                void ShutDown()
                {
                        g_inputBuffer.Shutdown();
                }
        } // namespace RawInputInterface

#ifdef DBGRawMouseInput
        inline namespace RawInputDebug
        {
                void DbgIncrementInputFrame()
                {
                        g_dbg_inputFrameCounter++;
                }
                void DbgStoreInputFrameCounter(InputFrame* thisFrame)
                {
                        thisFrame->frameNumber = g_dbg_inputFrameCounter;
                }
                void DbgDumpFrameData(InputFrame* thisFrame, int x, int y)
                {
                        if (thisFrame->IsMouseInput())
                        {
                                std::cout << "Frame:\t\t\t" << thisFrame->frameNumber << std::endl;

                                // int x = thisFrame->rawInput.data.mouse.lLastX;
                                // int y = thisFrame->rawInput.data.mouse.lLastY;

                                if (thisFrame->rawInput.data.mouse.usFlags == MOUSE_MOVE_ABSOLUTE)
                                {
                                        // x = MickeyToScreenPosX(x) - g_prevX;
                                        // y = MickeyToScreenPosX(y) - g_prevY;
                                }

                                std::cout << "X:\t\t\t" << x << std::endl;
                                std::cout << "Y:\t\t\t" << y << std::endl;

                                std::cout << "Buttons:\t\t\t" << thisFrame->rawInput.data.mouse.ulButtons << std::endl;
                                std::cout << "ButtonData:\t\t\t" << thisFrame->rawInput.data.mouse.usButtonData << std::endl;
                                std::cout << "ButtonFlags:\t\t\t" << thisFrame->rawInput.data.mouse.usButtonFlags << std::endl;
                                std::cout << "Flags:\t\t\t" << thisFrame->rawInput.data.mouse.usFlags << std::endl;
                                std::cout << "Extra:\t\t\t" << thisFrame->rawInput.data.mouse.ulExtraInformation << std::endl;
                                std::cout << std::endl << std::endl;
                        }
                        g_dbg_prevInputFrameCounter = thisFrame->frameNumber;
                }
        } // namespace RawInputDebug_ACTIVE
#endif
} // namespace RawInput

inline namespace WindowsMessagesGlobals
{
        inline bool g_windowsMessageShutdown = false;
}
inline namespace WindowsMessagesInternal
{
        inline void ProcessWindowsMessages()
        {
                MSG msg{};
                while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
                {
                        TranslateMessage(&msg);
                        DispatchMessageA(&msg);
                }
        }
} // namespace WindowsMessagesInternal
inline namespace WindowsMessages
{
        inline void LaunchMessageLoop()
        {
                while (!g_windowsMessageShutdown)
                {
                        ProcessWindowsMessages();
                }
        }
        inline void RequestMessageLoopExit()
        {
                g_windowsMessageShutdown = true;
        }
} // namespace WindowsMessages

inline namespace EngineGlobals
{
        volatile bool g_engineShutdown = false;
        uint64_t      g_frameCounter   = 0;
        std::thread   g_engineThread;
} // namespace EngineGlobals
inline namespace EngineInternal
{
        inline void EngineMain()
        {
                while (!g_engineShutdown)
                {
                        g_inputBuffer.ProcessReads();
                }
        }
} // namespace EngineInternal
inline namespace Engine
{
        inline void Initialize()
        {
                g_engineThread = std::thread(EngineMain);
        }
        inline void Shutdown()
        {
                g_engineShutdown = true;
                g_engineThread.join();
        }
} // namespace Engine

void Accumulate(unsigned i)
{}


constexpr unsigned buffer32Sz = 8;
__m256i            buffer32[buffer32Sz];

void Invalidate()
{
        for (auto& itr : buffer32)
                itr = _mm256_set1_epi32(0xdeadbeef);
}
void ResetBuffer()
{
        for (auto& itr : buffer32)
                itr = _mm256_setzero_si256();
}
unsigned Accumulate(__m256i* buffer, unsigned bufferSz)
{
        __m256i accumulate = buffer[0];
        for (unsigned i = 1; i < bufferSz; i++)
                accumulate = _mm256_add_epi32(accumulate, buffer[i]);

        return accumulate.m256i_i32[0] + accumulate.m256i_i32[1] + accumulate.m256i_i32[2] + accumulate.m256i_i32[3] +
               accumulate.m256i_i32[4] + accumulate.m256i_i32[5] + accumulate.m256i_i32[6] + accumulate.m256i_i32[7];
}

int WINAPI WinMain(_In_ HINSTANCE _hInstance, _In_opt_ HINSTANCE, _In_ LPSTR _pCmdLine, _In_ int _nCmdShow)
{
        Invalidate();
        ResetBuffer();

        buffer32->m256i_i32[0] = 1;
        buffer32->m256i_i32[1] = 7;
        buffer32->m256i_i32[2] = 15;
        buffer32->m256i_i32[3] = 2;
        buffer32->m256i_i32[4] = 7;
        buffer32->m256i_i32[5] = -15;

        auto result = Accumulate(buffer32, buffer32Sz);

        g_hInstance = _hInstance;
        g_pCmdLine  = _pCmdLine;
        g_nCmdShow  = _nCmdShow;
        RawInput::Initialize();
        Console::Initialize();
        Console::DisableQuickEdit();
        Engine::Initialize();

        auto MyWindow = CreateWindow().Title("DarkSeer").Size(percent(50, 50)).Position(percent(25, 25)).Finalize();
        MyWindow.Show();

        WindowsMessages::LaunchMessageLoop();

        Engine::Shutdown();
        RawInput::ShutDown();

        return 0;
}

inline namespace WindowsGlobals
{
        LRESULT CALLBACK g_defaultWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
        {
                switch (uMsg)
                {
                        case WM_DESTROY:
                                PostQuitMessage(0);
                                WindowsMessages::RequestMessageLoopExit();
                                break;

                        case WM_INPUT:
                        {
                                g_inputBuffer.Write(lParam);
                                DbgIncrementInputFrame();
                                break;
                        }
                }
                return DefWindowProcA(hwnd, uMsg, wParam, lParam);
        }
} // namespace WindowsGlobals