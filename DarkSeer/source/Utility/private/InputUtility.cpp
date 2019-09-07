#include <InputUtility.h>

#include <SingletonInput.h>
#include <SingletonWindow.h>

namespace InputUtil
{
        LRESULT CALLBACK InputWndProc(_In_ HWND hwnd, _In_ UINT message, _In_ WPARAM wParam, _In_ LPARAM lParam);

		void InitializeInputWndProc(SingletonInput* singlInput, const SingletonWindow* singlWindow)
        {
                // add input capture window proc to the window handle's previous window proc
                singlInput->m_parentWndProc =
                    (WNDPROC)SetWindowLongPtrA(singlWindow->m_mainHwnd, GWLP_WNDPROC, (LONG_PTR)InputUtil::InputWndProc);
                SetWindowPos(singlWindow->m_mainHwnd, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
        }

        void RegisterDefaultRawInputDevices()
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
                        std::exception();
                }
        }

        LRESULT InputWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
        {
                InputFrame tempInputFrame;
                memset(&tempInputFrame, 0, sizeof(InputFrame));

				auto singlInputBuffer = g_userEntityAdmin.GetSingletonInput()->GetInputBuffer();
                tempInputFrame.m_keyStateLow = singlInputBuffer->back().m_keyStateLow;
                tempInputFrame.m_keyStateHigh = singlInputBuffer->back().m_keyStateHigh;

                switch (message)
                {
                        case WM_INPUT:
                        {
                                RAWINPUT rawInputFrame{};
                                UINT     dwSize{};
                                GetRawInputData((HRAWINPUT)lParam, RID_INPUT, 0, &dwSize, sizeof(RAWINPUTHEADER));
                                GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &rawInputFrame, &dwSize, sizeof(RAWINPUTHEADER));

                                if (rawInputFrame.header.dwType == RIM_TYPEKEYBOARD)
                                {
                                        uint16_t      button_signature_flag = rawInputFrame.data.keyboard.Flags >> 1;
                                        KeyTransition transition_state =
                                            static_cast<KeyTransition>(rawInputFrame.data.keyboard.Flags & 1);
                                        tempInputFrame.m_buttonSignature =
                                            static_cast<KeyCode>(rawInputFrame.data.keyboard.MakeCode +
                                                                 button_signature_flag * INPUT_NUM_KEYBOARD_SCANCODES);
                                        tempInputFrame.m_transitionState = transition_state;

                                        singlInputBuffer->push_back(tempInputFrame);
                                }
                                else if (rawInputFrame.header.dwType == RIM_TYPEMOUSE)
                                {
                                        // relative mouse movement
                                        if (!(rawInputFrame.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE))
                                        {
                                                tempInputFrame.m_mouseDeltas = std::tuple{rawInputFrame.data.mouse.lLastX,
                                                                                          rawInputFrame.data.mouse.lLastY};
                                                singlInputBuffer->push_back(tempInputFrame);
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

                                                tempInputFrame.m_mouseDeltas = std::tuple{x, y};
                                                singlInputBuffer->push_back(tempInputFrame);
                                        }
                                }
                                break;
                        }
                        case WM_LBUTTONDOWN:
                        {
                                tempInputFrame.m_buttonSignature = KeyCode::mouseLeft;
                                singlInputBuffer->push_back(tempInputFrame);
                                break;
                        }
                        case WM_LBUTTONUP:
                        {
                                tempInputFrame.m_buttonSignature = KeyCode::mouseLeft;
                                tempInputFrame.m_transitionState = KeyTransition::Up;
                                singlInputBuffer->push_back(tempInputFrame);
                                break;
                        }
                        case WM_RBUTTONDOWN:
                        {
                                tempInputFrame.m_buttonSignature = KeyCode::mouseRight;
                                singlInputBuffer->push_back(tempInputFrame);
                                break;
                        }
                        case WM_RBUTTONUP:
                        {
                                tempInputFrame.m_buttonSignature = KeyCode::mouseRight;
                                tempInputFrame.m_transitionState = KeyTransition::Up;
                                singlInputBuffer->push_back(tempInputFrame);
                                break;
                        }
                        case WM_MBUTTONDOWN:
                        {
                                tempInputFrame.m_buttonSignature = KeyCode::mouseMiddle;
                                singlInputBuffer->push_back(tempInputFrame);
                                break;
                        }
                        case WM_MBUTTONUP:
                        {
                                tempInputFrame.m_buttonSignature = KeyCode::mouseMiddle;
                                tempInputFrame.m_transitionState = KeyTransition::Up;
                                singlInputBuffer->push_back(tempInputFrame);
                                break;
                        }
                        case WM_MOUSEWHEEL:
                        {
                                tempInputFrame.m_buttonSignature = KeyCode::mouseScrollVertical;
                                tempInputFrame.m_scrollDelta     = GET_WHEEL_DELTA_WPARAM(wParam);
                                singlInputBuffer->push_back(tempInputFrame);
                                break;
                        }
                        case WM_MOUSEHWHEEL:
                        {
                                tempInputFrame.m_buttonSignature = KeyCode::mouseScrollHorizontal;
                                tempInputFrame.m_scrollDelta     = GET_WHEEL_DELTA_WPARAM(wParam);
                                singlInputBuffer->push_back(tempInputFrame);
                                break;
                        }
                        case WM_XBUTTONDOWN:
                        {
                                switch (GET_XBUTTON_WPARAM(wParam))
                                {
                                        case XBUTTON1:
                                        {
                                                tempInputFrame.m_buttonSignature = KeyCode::mouseForward;
                                                singlInputBuffer->push_back(tempInputFrame);
                                                break;
                                        }
                                        case XBUTTON2:
                                        {
                                                tempInputFrame.m_buttonSignature = KeyCode::mouseBack;
                                                singlInputBuffer->push_back(tempInputFrame);
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
                                                tempInputFrame.m_buttonSignature = KeyCode::mouseForward;
                                                tempInputFrame.m_transitionState = KeyTransition::Up;
                                                singlInputBuffer->push_back(tempInputFrame);
                                                break;
                                        }
                                        case XBUTTON2:
                                        {
                                                tempInputFrame.m_buttonSignature = KeyCode::mouseBack;
                                                tempInputFrame.m_transitionState = KeyTransition::Up;
                                                singlInputBuffer->push_back(tempInputFrame);
                                                break;
                                        }
                                }
                                break;
                        }
                        case WM_KILLFOCUS:
                        {
                                // set all press states to released
                                memset(&tempInputFrame.m_keyStateLow, 0, sizeof(tempInputFrame.m_keyStateLow));
                                singlInputBuffer->push_back(tempInputFrame);
                                break;
                        }
                }

                return CallWindowProcA(g_userEntityAdmin.GetSingletonInput()->m_parentWndProc,
                                       g_userEntityAdmin.GetSingletonWindow()->m_mainHwnd,
                                       message,
                                       wParam,
                                       lParam);
        }
} // namespace InputUtil

