#include <Windows.h>
#include <assert.h>
#include <emmintrin.h>
#include <immintrin.h>
#include <atomic>
#include <cstring>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <thread>
#include <tuple>
#undef CreateWindow
#undef min
#undef max
using QWORD = __int64;
#include "Utility/Math.h"
#include "Utility/RawInput.h"
//#include "MemoryLeakDetection.h"
//#include "SmallSizeUtility.h"


namespace std
{} // namespace std

inline namespace MemoryGlobals
{
        constexpr auto KiB        = 1024;
        constexpr auto MiB        = KiB * 1024;
        constexpr auto GiB        = MiB * 1024;
        constexpr auto CACHE_LINE = std::hardware_destructive_interference_size;
} // namespace MemoryGlobals

inline namespace Random
{
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
} // namespace Random

inline namespace TaggedIntegrals
{
        inline namespace Interface
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
        } // namespace Interface
} // namespace TaggedIntegrals

inline namespace WindowsProcs
{
        inline LRESULT CALLBACK g_mainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
}
inline namespace Windows
{
        inline namespace Globals
        {
                inline HINSTANCE g_hInstance    = 0;
                inline LPSTR     g_pCmdLine     = 0;
                inline int       g_nCmdShow     = 0;
                inline auto      g_screenWidth  = GetSystemMetrics(SM_CXSCREEN);
                inline auto      g_screenHeight = GetSystemMetrics(SM_CYSCREEN);
        } // namespace Globals
        inline namespace Internal
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
                        HBRUSH      hbrBackground;
                };
        } // namespace Internal
        inline namespace Interface
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
                                windowCreationDescriptor.windowProc = g_mainWindowProc;
                                windowCreationDescriptor.style      = WS_OVERLAPPED;
                                windowCreationDescriptor.x          = CW_USEDEFAULT;
                                windowCreationDescriptor.y          = CW_USEDEFAULT;
                                windowCreationDescriptor.width      = CW_USEDEFAULT;
                                windowCreationDescriptor.height     = CW_USEDEFAULT;
                                windowCreationDescriptor.style      = WS_OVERLAPPED;
                                strcpy_s(windowCreationDescriptor.WindowClassName, std::to_string(__COUNTER__).c_str());
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
                        CreateWindow& BackgroundColor(unsigned r, unsigned g, unsigned b)
                        {
                                COLORREF rgb                           = 0 | r | (g << 1) | (b << 2);
                                windowCreationDescriptor.hbrBackground = CreateSolidBrush(rgb);
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
                                wc.style             = CS_HREDRAW | CS_VREDRAW;
                                wc.hbrBackground     = windowCreationDescriptor.hbrBackground;


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
        } // namespace Interface

} // namespace Windows

inline namespace Console
{
        inline namespace Interface
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
        } // namespace Interface
} // namespace Console

inline namespace RawInput
{
        inline namespace DebugGlobals
        {
                uint64_t g_dbg_prevInputFrameCounter = -1;
                uint64_t g_dbg_inputFrameCounter     = 0;
        } // namespace DebugGlobals

        inline namespace Constexpr
        {
                constexpr auto MICKEY = KiB * 64;
        }

        // Forward Declares
        inline namespace Interface
        {
                struct InputFrame;
        } // namespace Interface
        inline namespace Debug
        {
                void DbgIncrementInputFrame()
                {
                        g_dbg_inputFrameCounter++;
                }
                void DbgStoreInputFrameCounter(InputFrame*)
                {}
                void DbgDumpFrameData()
                {}
        } // namespace Debug

        inline namespace Internal
        {
                // for initializing previous mouse x and y only
                POINT GetCursorInitialPos()
                {
                        POINT p;
                        GetCursorPos(&p);
                        return p;
                }
        } // namespace Internal
        inline namespace Globals
        {
                POINT cursorInitialPos = GetCursorInitialPos();
                int   g_prevX          = cursorInitialPos.x;
                int   g_prevY          = cursorInitialPos.y;
        } // namespace Globals
        inline namespace Interface
        {
                struct InputFrame
                {
                        static constexpr long m_BUTTON_PRESS_FLAG = 0x80000000;
                        RAWMOUSE              rm;
                        union
                        {
                                std::tuple<long, long> MouseDelta;
                                uint64_t               OtherData;
                                struct
                                {
                                        bool ButtonPress;
                                        char padding[7];
                                };
                        };
                };

                // single producer -> single consumer  buffer
                struct InputBuffer
                {
                    private:
                        static constexpr uint64_t m_BUFFER_SIZE      = 16384ULL;
                        static constexpr unsigned m_NUM_SWAP_BUFFERS = 2;
                        // m_NUM_SWAP_BUFFERS must be power of 2, swap() uses mersenne prime modulation

                        volatile __m256i*                m_inputFramesM256;
                        volatile std::tuple<long, long>* m_inputFrames;

                        volatile uint64_t m_bufferSizes[m_NUM_SWAP_BUFFERS];
                        unsigned          m_readSwapBufferIndex  = 0;
                        unsigned          m_writeSwapBufferIndex = 1;

                        void swap()
                        {
                                m_readSwapBufferIndex  = m_writeSwapBufferIndex;
                                m_writeSwapBufferIndex = (m_writeSwapBufferIndex + 1) & (m_NUM_SWAP_BUFFERS - 1);
                        }

                        bool try_push(LPARAM& RawInputlParam)
                        {
                                const auto bufferWriteIndex = m_bufferSizes[m_writeSwapBufferIndex];

                                if (bufferWriteIndex == m_BUFFER_SIZE)
                                        return false;
                                assert(bufferWriteIndex < m_BUFFER_SIZE);

                                UINT dwSize = 0;


                                RAWINPUT rawInputFrame;
                                GetRawInputData((HRAWINPUT)RawInputlParam, RID_INPUT, 0, &dwSize, sizeof(RAWINPUTHEADER));
                                GetRawInputData(
                                    (HRAWINPUT)RawInputlParam, RID_INPUT, &rawInputFrame, &dwSize, sizeof(RAWINPUTHEADER));

                                if (rawInputFrame.header.dwType = RIM_TYPEMOUSE)
                                {
                                        const auto lastX = rawInputFrame.data.mouse.lLastX;
                                        const auto lastY = rawInputFrame.data.mouse.lLastY;

                                        std::tuple<long, long>& mouseInputElementAlias = const_cast<std::tuple<long, long>*>(
                                            m_inputFrames)[m_writeSwapBufferIndex * m_BUFFER_SIZE + bufferWriteIndex];

                                        for (unsigned i = 0; i <= bufferWriteIndex; i++)
                                        {
                                                std::cout << "[" << i << "]\t"
                                                          << std::get<0>(const_cast<std::tuple<long, long>*>(m_inputFrames)[i])
                                                          << ","
                                                          << std::get<1>(const_cast<std::tuple<long, long>*>(m_inputFrames)[i])
                                                          << std::endl;
                                                if (i > 0)
                                                        int pause = 0;
                                        }
                                        std::cout << "\n\n";


                                        mouseInputElementAlias = std::tuple<long, long>(lastX, lastY);
                                        int pause              = 0;
                                        m_bufferSizes[m_writeSwapBufferIndex]++;
                                }

                                return true;
                        }

                    public:
                        // single consumer functions:
                        void ProcessReads()
                        {
                                auto sz = m_bufferSizes[m_readSwapBufferIndex];
                                if (!sz)
                                        return;

                                for (unsigned i = 0; i < m_bufferSizes[m_readSwapBufferIndex]; i++)
                                {
                                        auto thisFrame =
                                            const_cast<__m256i*>(&m_inputFramesM256[m_readSwapBufferIndex * m_BUFFER_SIZE + i]);
                                }
                                Sleep(160);
                                m_bufferSizes[m_readSwapBufferIndex] = 0;
                        }

                        // single producer functions:
                        void Write(LPARAM& RawInputLParam)
                        {
                                if (!try_push(RawInputLParam))
                                {
                                        while (m_bufferSizes[m_readSwapBufferIndex] != 0)
                                        {
                                                Sleep(0);
                                        }
                                        // TODO // Run a job while we wait?

                                        //
                                        swap();
                                        try_push(RawInputLParam);
                                        return;
                                };
                                if (m_bufferSizes[m_readSwapBufferIndex] == 0)
                                        swap();

                                Sleep(0);
                        }

                        void Initialize()
                        {
                                m_inputFramesM256 = (volatile __m256i*)_aligned_malloc(
                                    m_BUFFER_SIZE * sizeof(__m256i) * m_NUM_SWAP_BUFFERS, alignof(__m256i));
                                m_inputFrames = reinterpret_cast<decltype(m_inputFrames)>(m_inputFramesM256);
                                assert(m_inputFrames);
                                for (unsigned i = 0; i < m_BUFFER_SIZE * m_NUM_SWAP_BUFFERS; i++)
                                {
                                        const_cast<std::tuple<long, long>&>(m_inputFrames[i]) = std::tuple(0L, 0L);
                                }
                        }

                        void Shutdown()
                        {
                                _aligned_free(const_cast<__m256i*>(m_inputFramesM256));
                        }
                };
        } // namespace Interface
        inline namespace Globals
        {
                InputBuffer g_inputBuffer;
        } // namespace Globals
        inline namespace Interface
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
        } // namespace Interface
} // namespace RawInput

inline namespace WindowsMessages
{
        inline namespace Globals
        {
                inline bool g_windowsMessageShutdown = false;
        }
        inline namespace Internal
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
        } // namespace Internal
        inline namespace Interface
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
        } // namespace Interface
} // namespace WindowsMessages


inline namespace Engine
{
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
                                // g_inputBuffer.ProcessReads();
                                g_inputBufferE.process_writes();
                        }
                }
        } // namespace EngineInternal
        inline namespace Interface
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
        } // namespace Interface
} // namespace Engine


inline namespace WindowsProcs
{
        LRESULT CALLBACK g_mainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
        {
                InputFrameE inputFrame;
                memset(((char*)&inputFrame) + sizeof(PressState), 0, sizeof(InputFrameE) - sizeof(PressState));
                inputFrame.m_pressState = g_inputBufferE.m_prevPressState;

                switch (uMsg)
                {
                        case WM_INPUT:
                        {
                                RAWINPUT rawInputFrame;
                                UINT     dwSize;
                                GetRawInputData((HRAWINPUT)lParam, RID_INPUT, 0, &dwSize, sizeof(RAWINPUTHEADER));
                                GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &rawInputFrame, &dwSize, sizeof(RAWINPUTHEADER));

                                if (rawInputFrame.header.dwType == RIM_TYPEKEYBOARD)
                                {
                                        uint16_t        button_signature_flag = rawInputFrame.data.keyboard.Flags >> 1;
                                        TransitionState transition_state =
                                            (TransitionState)(rawInputFrame.data.keyboard.Flags & 1);
                                        inputFrame.m_buttonSignature =
                                            (ButtonSignature)(rawInputFrame.data.keyboard.MakeCode +
                                                              button_signature_flag * INPUT_NUM_KEYBOARD_SCANCODES);
                                        inputFrame.m_transitionState = transition_state;

                                        // update inputFrame press states
                                        switch (inputFrame.m_transitionState)
                                        {
                                                case TransitionState::INPUT_transitionStateUp:
                                                        inputFrame.m_pressState.KeyUp(inputFrame.m_buttonSignature);
                                                        break;
                                                case TransitionState::INPUT_transitionStateDown:
                                                        inputFrame.m_pressState.KeyDown(inputFrame.m_buttonSignature);
                                                        break;
                                        }
                                        g_inputBufferE.m_prevPressState = inputFrame.m_pressState;

                                        g_inputBufferE.write(inputFrame);
                                }
                                else if (rawInputFrame.header.dwType == RIM_TYPEMOUSE)
                                {
                                        // relative mouse movement
                                        if (!(rawInputFrame.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE))
                                        {
                                                inputFrame.m_mouseDeltas = std::tuple{rawInputFrame.data.mouse.lLastX,
                                                                                      rawInputFrame.data.mouse.lLastY};
                                                g_inputBufferE.write(inputFrame);
                                        }
                                        // absolute mouse movment
                                        else if (rawInputFrame.data.mouse.usFlags & MOUSE_VIRTUAL_DESKTOP)
                                        {
                                                const int width  = GetSystemMetrics(SM_CXVIRTUALSCREEN);
                                                const int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);

                                                int x = static_cast<int>((float(rawInputFrame.data.mouse.lLastX) / 65535.0f) *
                                                                         width);
                                                int y = static_cast<int>((float(rawInputFrame.data.mouse.lLastY) / 65535.0f) *
                                                                         height);

                                                inputFrame.m_mouseDeltas = std::tuple{x, y};
                                                g_inputBufferE.write(inputFrame);
                                        }
                                }
                                break;
                        }
                        case WM_LBUTTONDOWN:
                        {
                                inputFrame.m_buttonSignature = INPUT_mouseLeft;
                                g_inputBufferE.write(inputFrame);
                                break;
                        }
                        case WM_LBUTTONUP:
                        {
                                inputFrame.m_buttonSignature = INPUT_mouseLeft;
                                inputFrame.m_transitionState = INPUT_transitionStateUp;
                                g_inputBufferE.write(inputFrame);
                                break;
                        }
                        case WM_RBUTTONDOWN:
                        {
                                inputFrame.m_buttonSignature = INPUT_mouseRight;
                                g_inputBufferE.write(inputFrame);
                                break;
                        }
                        case WM_RBUTTONUP:
                        {
                                inputFrame.m_buttonSignature = INPUT_mouseRight;
                                inputFrame.m_transitionState = INPUT_transitionStateUp;
                                g_inputBufferE.write(inputFrame);
                                break;
                        }
                        case WM_MBUTTONDOWN:
                        {
                                inputFrame.m_buttonSignature = INPUT_mouseMiddle;
                                g_inputBufferE.write(inputFrame);
                                break;
                        }
                        case WM_MBUTTONUP:
                        {
                                inputFrame.m_buttonSignature = INPUT_mouseMiddle;
                                inputFrame.m_transitionState = INPUT_transitionStateUp;
                                g_inputBufferE.write(inputFrame);
                                break;
                        }
                        case WM_MOUSEWHEEL:
                        {
                                std::underlying_type<TransitionState>::type mouseDelta = GET_WHEEL_DELTA_WPARAM(wParam);
                                inputFrame.m_buttonSignature                           = INPUT_mouseScrollVertical;
                                (std::underlying_type<TransitionState>::type&)inputFrame.m_transitionState = mouseDelta;
                                g_inputBufferE.write(inputFrame);
                                break;
                        }
                        case WM_XBUTTONDOWN:
                        {
                                switch (GET_XBUTTON_WPARAM(wParam))
                                {
                                        case XBUTTON1:
                                        {
                                                inputFrame.m_buttonSignature = INPUT_mouseForward;
                                                g_inputBufferE.write(inputFrame);
                                                break;
                                        }
                                        case XBUTTON2:
                                        {
                                                inputFrame.m_buttonSignature = INPUT_mouseBack;
                                                g_inputBufferE.write(inputFrame);
                                                break;
                                        }
                                }
                                break;
                        }
                        case WM_XBUTTONUP:
                        {
                                switch (GET_XBUTTON_WPARAM(wParam))
                                {
                                        case XBUTTON1:
                                        {
                                                inputFrame.m_buttonSignature = INPUT_mouseForward;
                                                inputFrame.m_transitionState = INPUT_transitionStateUp;
                                                g_inputBufferE.write(inputFrame);
                                                break;
                                        }
                                        case XBUTTON2:
                                        {
                                                inputFrame.m_buttonSignature = INPUT_mouseBack;
                                                inputFrame.m_transitionState = INPUT_transitionStateUp;
                                                g_inputBufferE.write(inputFrame);
                                                break;
                                        }
                                }
                                break;
                        }
                        case WM_KILLFOCUS:
                        {
								// set all press states to released
                                memset(&inputFrame.m_pressState, 0, sizeof(inputFrame.m_pressState));
                                g_inputBufferE.m_prevPressState = inputFrame.m_pressState;
                                g_inputBufferE.write(inputFrame);
                                break;
						}

                        case WM_DESTROY:
                        {
                                PostQuitMessage(0);
                                WindowsMessages::RequestMessageLoopExit();
                                break;
                        }
                }

                return DefWindowProcA(hwnd, uMsg, wParam, lParam);
        }
        LRESULT CALLBACK g_titleWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
        {
                switch (uMsg)
                {
                        case WM_CLOSE:
                                CloseWindow(hwnd);
                                break;
                        default:
                                return DefWindowProcA(hwnd, uMsg, wParam, lParam);
                }
                return 0;
        }
} // namespace WindowsProcs

int WINAPI WinMain(_In_ HINSTANCE _hInstance, _In_opt_ HINSTANCE, _In_ LPSTR _pCmdLine, _In_ int _nCmdShow)
{
        RawInputE::PressState pressState;
        memset(&pressState, 0, sizeof(pressState));


        Console::Initialize();
        Console::DisableQuickEdit();

        g_hInstance = _hInstance;
        g_pCmdLine  = _pCmdLine;
        g_nCmdShow  = _nCmdShow;

        RawInput::Initialize();
        g_inputBufferE.initialize();
        Engine::Initialize();

        auto MyWindow = CreateWindow().Title("DarkSeer").Size(percent(50, 50)).Position(percent(25, 25)).Finalize();

        MyWindow.Show();

        WindowsMessages::LaunchMessageLoop();
        g_inputBufferE.shutdown();
        Engine::Shutdown();
        RawInput::ShutDown();

        return 0;
}
