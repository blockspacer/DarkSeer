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

template <typename... SubSettings>
struct ConfigurationSettings : public SubSettings...
{};

template <typename... SubSettingsLhs, typename... SubSettingsRhs>
constexpr auto operator+(const ConfigurationSettings<SubSettingsLhs...>& lhs,
                         const ConfigurationSettings<SubSettingsRhs...>& rhs)
{
        return ConfigurationSettings<SubSettingsLhs..., SubSettingsRhs...>{};
}

inline namespace Math
{
        void Invalidate(__m256i* buffer, unsigned sz)
        {
                for (unsigned i = 0; i < sz; i++)
                        buffer[i] = _mm256_set1_epi32(0xdeadbeef);
        }

        void ResetBuffer(__m256i* buffer, unsigned sz)
        {
                for (unsigned i = 0; i < sz; i++)
                        buffer[i] = _mm256_setzero_si256();
        }

        unsigned Accumulate(const __m256i* const buffer, unsigned sz)
        {
                __m256i accumulate = buffer[0];
                for (unsigned i = 1; i < sz; i++)
                        accumulate = _mm256_add_epi32(accumulate, buffer[i]);

                return accumulate.m256i_i32[0] + accumulate.m256i_i32[1] + accumulate.m256i_i32[2] + accumulate.m256i_i32[3] +
                       accumulate.m256i_i32[4] + accumulate.m256i_i32[5] + accumulate.m256i_i32[6] + accumulate.m256i_i32[7];
        }

        template <typename T1, typename T2>
        constexpr auto divideRoundUp(T1 dividend, T2 divisor)
        {
                assert(divisor != 0);
                return (dividend + divisor - 1) / divisor;
        }
} // namespace Math

template <typename... Ts, unsigned I>
constexpr void plusequalsimpl(std::tuple<Ts...>& lhs, const std::tuple<Ts...>& rhs, std::index_sequence<I>)
{
        std::get<I>(lhs) += std::get<I>(rhs);
}
template <typename... Ts, unsigned I, unsigned... Is>
constexpr typename std::enable_if<sizeof...(Is)>::type plusequalsimpl(std::tuple<Ts...>&       lhs,
                                                                      const std::tuple<Ts...>& rhs,
                                                                      std::index_sequence<I, Is...>)
{
        std::get<I>(lhs) += std::get<I>(rhs);
        plusequalsimpl(lhs, rhs, std::index_sequence<Is...>{});
}

namespace std
{
        template <typename... Ts>
        constexpr std::tuple<Ts...>& operator+=(std::tuple<Ts...>& lhs, const std::tuple<Ts...>& rhs)
        {
                plusequalsimpl(lhs, rhs, std::make_index_sequence<sizeof...(Ts)>{});
                return lhs;
        }
} // namespace std

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
#ifndef DBGRawMouseInput
#define DBGRawMouseInput false
#endif
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
                void DbgIncrementInputFrame();
                void DbgStoreInputFrameCounter(InputFrame*);
                void DbgDumpFrameData();
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
                int MickeyToScreenPosX(int value)
                {
                        return (static_cast<float>(value) / MICKEY) * g_screenWidth;
                }
                int MickeyToScreenPosY(int value)
                {
                        return (static_cast<float>(value) / MICKEY) * g_screenHeight;
                }
                bool IsMouseInput(RAWINPUT* rawInput)
                {
                        return rawInput->header.dwType == RIM_TYPEMOUSE;
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
                        static constexpr uint64_t m_BUFFER_SIZE = 16384ULL;
                        static constexpr unsigned m_NUM_BUFFERS = 2;
                        // m_NUM_BUFFERS must be power of 2, Swap() uses mersenne prime modulation

                        volatile __m256i*                m_mouseDeltaBuffers;
                        volatile std::tuple<long, long>* m_mouseDeltaBuffersUnsignedView;

                        volatile uint64_t m_bufferSizes[m_NUM_BUFFERS];
                        unsigned          m_readSwapBufferIndex  = 0;
                        unsigned          m_writeSwapBufferIndex = 1;

                        void Swap()
                        {
                                m_readSwapBufferIndex  = m_writeSwapBufferIndex;
                                m_writeSwapBufferIndex = (m_writeSwapBufferIndex + 1) & (m_NUM_BUFFERS - 1);
                        }

                        bool TryPushWrite(LPARAM& RawInputlParam)
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

                                if (IsMouseInput(&rawInputFrame))
                                {
                                        const auto lastX = rawInputFrame.data.mouse.lLastX;
                                        const auto lastY = rawInputFrame.data.mouse.lLastY;

                                        std::tuple<long, long>& mouseInputElementAlias = const_cast<std::tuple<long, long>*>(
                                            m_mouseDeltaBuffersUnsignedView)[m_writeSwapBufferIndex * m_BUFFER_SIZE +
                                                                             bufferWriteIndex];

                                        for (unsigned i = 0; i <= bufferWriteIndex; i++)
                                        {
                                                // std::cout << "[" << i << "]\t"
                                                //          << std::get<0>(const_cast<std::tuple<long, long>*>(
                                                //                 m_mouseDeltaBuffersUnsignedView)[i])
                                                //          << ","
                                                //          << std::get<1>(const_cast<std::tuple<long, long>*>(
                                                //                 m_mouseDeltaBuffersUnsignedView)[i])
                                                //          << std::endl;
                                                if (i > 0)
                                                        int pause = 0;
                                        }
                                        // std::cout << "\n\n";


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
                                        auto thisFrame = const_cast<__m256i*>(
                                            &m_mouseDeltaBuffers[m_readSwapBufferIndex * m_BUFFER_SIZE + i]);
                                }
                                Sleep(160);
                                m_bufferSizes[m_readSwapBufferIndex] = 0;
                        }

                        // single producer functions:
                        void Write(LPARAM& RawInputLParam)
                        {
                                if (!TryPushWrite(RawInputLParam))
                                {
                                        while (m_bufferSizes[m_readSwapBufferIndex] != 0)
                                        {
                                                Sleep(0);
                                        }
                                        // TODO // Run a job while we wait?

                                        //
                                        Swap();
                                        TryPushWrite(RawInputLParam);
                                        return;
                                };
                                if (m_bufferSizes[m_readSwapBufferIndex] == 0)
                                        Swap();

                                Sleep(0);
                        }

                        void Initialize()
                        {
                                m_mouseDeltaBuffers = (volatile __m256i*)_aligned_malloc(
                                    m_BUFFER_SIZE * sizeof(__m256i) * m_NUM_BUFFERS, alignof(__m256i));
                                m_mouseDeltaBuffersUnsignedView =
                                    reinterpret_cast<decltype(m_mouseDeltaBuffersUnsignedView)>(m_mouseDeltaBuffers);

                                ResetBuffer(const_cast<__m256i*>(m_mouseDeltaBuffers), m_BUFFER_SIZE);
                        }

                        void Shutdown()
                        {
                                _aligned_free(const_cast<__m256i*>(m_mouseDeltaBuffers));
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
        inline namespace Debug
        {
                void DbgIncrementInputFrame()
                {
                        if constexpr (DBGRawMouseInput)
                                g_dbg_inputFrameCounter++;
                }
                void DbgStoreInputFrameCounter(InputFrame* thisFrame)
                {
                        if constexpr (DBGRawMouseInput)
                                ;
                        /*thisFrame->frameNumber = g_dbg_inputFrameCounter;*/
                }
                void DbgDumpFrameData()
                {
                        if constexpr (DBGRawMouseInput)
                                ;
                        // g_dbg_prevInputFrameCounter = thisFrame->frameNumber;
                }
        } // namespace Debug
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
                                g_inputBuffer.ProcessReads();
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


struct Experimental
{
        // enum TransitionState : uint16_t
        //{
        //        UP,
        //        DOWN
        //};
        // enum ButtonId : uint16_t
        //{
        //        Mouse0,
        //        Mouse1,
        //        Mouse2,
        //        Mouse3,
        //        Mouse4,
        //        Mouse5
        //};
        // struct InputFrame
        //{
        //        InputFrame()
        //        {}
        //        InputFrame(long arg0, long arg1, TransitionState arg2, ButtonId arg3)
        //        {
        //                m_mouseDeltas     = {arg0, arg1};
        //                m_transitionState = arg2;
        //                m_buttonId        = arg3;
        //        }
        //        std::tuple<long, long> m_mouseDeltas;
        //        TransitionState        m_transitionState; // up or down
        //        ButtonId               m_buttonId;        // buttonId
        //};
        // static constexpr auto aewfwwaef = sizeof(Experimental::InputFrame);
        // static constexpr auto m_SZ      = 32;
        // static constexpr auto m_SZ256f  = m_SZ / (static_cast<float>(sizeof(__m256i)) / sizeof(Experimental::InputFrame));
        // static constexpr auto m_SZ256   = static_cast<unsigned>(m_SZ256f);
        // static_assert(!(m_SZ * sizeof(Experimental::InputFrame) % m_SZ256 * sizeof(__m256i)));

        // alignas(sizeof(__m256i)) InputFrame m_inputFrames[m_SZ];
        // long m_inputFramesSz = 0;
        // void reset()
        //{
        //        __m256i(&m_inputFrames256)[m_SZ256] = reinterpret_cast<__m256i(&)[m_SZ256]>(m_inputFrames);

        //        for (auto& itr : m_inputFrames256)
        //                itr = _mm256_setzero_si256();
        //}
        // void invalidate()
        //{
        //        __m256i(&m_inputFrames256)[m_SZ256] = reinterpret_cast<__m256i(&)[m_SZ256]>(m_inputFrames);

        //        for (auto& itr : m_inputFrames256)
        //                itr = _mm256_set1_epi32(-1);
        //}
        // void push_back(Experimental::InputFrame InputFrame)
        //{
        //        // mouse move		:	push_back or concat_back
        //        if (!InputFrame.m_buttonPresses)
        //        {
        //                // array is empty						:		always push_back
        //                if (!m_inputFramesSz)
        //                {
        //                        m_inputFrames[m_inputFramesSz] = InputFrame;
        //                        m_inputFramesSz++;
        //                }
        //                // last push was a button press			:		push_back
        //                else if (m_inputFrames[m_inputFramesSz - 1].m_buttonPresses)
        //                {
        //                        m_inputFrames[m_inputFramesSz] = InputFrame;
        //                        m_inputFramesSz++;
        //                }
        //                // last push was a mouse move			:		concat_back
        //                else
        //                {
        //                        m_inputFrames[m_inputFramesSz - 1].m_mouseDeltas += InputFrame.m_mouseDeltas;
        //                }
        //        }
        //        // button press		:	always push_back
        //        else
        //        {
        //                m_inputFrames[m_inputFramesSz] = InputFrame;
        //                m_inputFramesSz++;
        //        }
        //}
};


inline namespace WindowsProcs
{
        LRESULT CALLBACK g_mainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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


template <auto _ConfigFlag>
constexpr auto EnumToType();

struct EmptyModule
{

};

template <typename... Modules>
struct Admin final : public Modules...
{};

template <auto& CompileSettings, unsigned... Is>
constexpr auto CreateAdminImpl(std::index_sequence<Is...>)
{
        return Admin<decltype(EnumToType<std::get<Is>(CompileSettings)>())...>();
}
template <auto& CompileSettings>
constexpr auto CreateAdmin()
{
        constexpr auto tuple_size = std::tuple_size<std::remove_reference<decltype(CompileSettings)>::type>::value;
        return CreateAdminImpl<CompileSettings>(std::make_index_sequence<tuple_size>{});
}

enum class ConfigFlags
{
        DebugPie,
        ReleasePie,
        DebugWork,
        ReleaseWork,
        InvalidFlag
};


struct DebugBakePie
{
        void BakePie()
        {
                std::cout << "DebugPie\n";
        }
};
struct ReleaseBakePie
{
        void BakePie()
        {
                std::cout << "ReleasePie\n";
        }
};

struct DebugWork
{
        void DoWork()
        {
                std::cout << "DebugDoWork\n";
        }
};
struct ReleaseWork
{
        void DoWork()
        {
                std::cout << "ReleaseDoWork\n";
        }
};

template <>
constexpr auto EnumToType<ConfigFlags::InvalidFlag>()
{
        return EmptyModule{};
}
template <>
constexpr auto EnumToType<ConfigFlags::DebugPie>()
{
        return DebugBakePie{};
}
template <>
constexpr auto EnumToType<ConfigFlags::ReleasePie>()
{
        return ReleaseBakePie{};
}
template <>
constexpr auto EnumToType<ConfigFlags::DebugWork>()
{
        return DebugWork{};
}
template <>
constexpr auto EnumToType<ConfigFlags::ReleaseWork>()
{
        return ReleaseWork{};
}

#define COMPILE_FLAG(X) ConfigFlags::##X,
constexpr std::tuple CompileSettingsV5 = {
#include "CompileSettings.cmp"
    ConfigFlags::InvalidFlag};
#undef COMPILE_FLAG


constexpr std::tuple CompileSettingsV1 = {ConfigFlags::DebugPie};
constexpr std::tuple CompileSettingsV2 = std::tuple_cat(CompileSettingsV1, std::tuple(ConfigFlags::DebugWork));
inline auto          g_admin           = CreateAdmin<CompileSettingsV5>();

int WINAPI WinMain(_In_ HINSTANCE _hInstance, _In_opt_ HINSTANCE, _In_ LPSTR _pCmdLine, _In_ int _nCmdShow)
{
        Console::Initialize();
        Console::DisableQuickEdit();
        // auto test = CompileSettingsV5;
        g_admin.BakePie();


        // g_hInstance = _hInstance;
        // g_pCmdLine  = _pCmdLine;
        // g_nCmdShow  = _nCmdShow;

        // RawInput::Initialize();
        // Engine::Initialize();

        // auto MyWindow  = CreateWindow().Title("DarkSeer").Size(percent(50, 50)).Position(percent(25, 25)).Finalize();
        // auto MyWindow2 = CreateWindow()
        //                     .Title("TestWindow")
        //                     .Size(percent(25, 25))
        //                     .Position(percent(25, 25))
        //                     .WindProc(g_titleWindowProc)
        //                     .BackgroundColor(1, 0, 0)
        //                     .Finalize();

        // MyWindow.Show();
        // MyWindow2.Show();

        // WindowsMessages::LaunchMessageLoop();

        // Engine::Shutdown();
        // RawInput::ShutDown();

        return 0;
}
