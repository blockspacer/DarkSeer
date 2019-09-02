#include <DSWindows.h>

inline namespace DSWindows
{
        inline namespace Globals
        {
                LRESULT CALLBACK g_mainWindowProc(HWND m_hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
                {
                        switch (uMsg)
                        {
                                case WM_DESTROY:
                                {
                                        PostQuitMessage(0);
                                        RequestMessageLoopExit();
                                        break;
                                }
                        }

                        return DefWindowProcA(m_hwnd, uMsg, wParam, lParam);
                }
        } // namespace Globals
} // namespace Windows