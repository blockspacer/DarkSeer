#include <WindowUtility.h>

#include "../private/WindowFactories.h"

#include <SingletonWindow.h>

namespace WindowUtil
{
        LRESULT CALLBACK MainWindowProc(HWND m_hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
        {
                switch (uMsg)
                {
                        case WM_DESTROY:
                        {
                                PostQuitMessage(NO_ERROR);
                                g_userEntityAdmin.WindowsShutdown(g_userEntityAdmin.GetSingletonWindow());
                                break;
                        }
                }
                return DefWindowProcA(m_hwnd, uMsg, wParam, lParam);
        }

        void CreateAndShowMainWindow(SingletonWindow* singlWindow)
        {
                auto window = WindowProxy()
                                  .Title("DarkSeer")
                                  .Size(percent(50, 50))
                                  .Position(percent(25, 25))
                                  .WindProc(MainWindowProc)
                                  .Create();

                singlWindow->m_mainHwnd = window.m_hwnd;
                ShowWindow(singlWindow->m_mainHwnd, SW_SHOW);
        }
} // namespace WindowUtil