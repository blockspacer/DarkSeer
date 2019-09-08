#include <ConsoleUtility.h>
#include <InputSystem.h>

#include <SingletonConsole.h>
#include <SingletonInput.h>

const ActivatedKeyCodeSet _test_keycode_set = ActivatedKeyCodeSet({KeyCode::A, KeyCode::ShiftLeft, KeyCode::S}, KeyCode::A);

void InputSystem::PreUpdate(EntityAdmin* entityAdmin)
{
        auto singlInput       = entityAdmin->GetSingletonInput();
        auto singlConsole     = entityAdmin->GetSingletonConsole();
        auto singlInputBuffer = singlInput->GetInputBuffer();
        singlInputBuffer->Signal();

        for (auto& itr : *singlInputBuffer)
        {
                if (itr.IsEmptyFrame())
                        continue;

                if (itr.IsMouseMoveFrame())
                {
                        auto [Rx, Ry] = itr->m_mouseDeltas;
                        std::cout << "MouseMove\t";
                        std::cout << "x:" << Rx << ",y:" << Ry << "\n";
                }
                if (itr.IsKeyCodeFrame())
                {
                        std::cout << KeyCodeToString(itr->m_keyCode) << "\t";
                        if (!itr.IsScrollFrame())
                        {
                                std::cout << TransitionStateToString(itr->m_transitionState) << "\n";
                        }
                        else
                        {
                                std::cout << itr->m_scrollDelta << "\n";
                        }
                }
                if (itr.IsKeySetBeginPress(_test_keycode_set))
                        std::cout << "HelloSIMD\n";
        }
}
