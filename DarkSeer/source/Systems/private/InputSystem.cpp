#include <ConsoleUtility.h>
#include <InputSystem.h>

#include <SingletonConsole.h>
#include <SingletonInput.h>
void InputSystem::PreUpdate(EntityAdmin* entityAdmin)
{
        auto singlInput       = entityAdmin->GetSingletonInput();
        auto singlConsole     = entityAdmin->GetSingletonConsole();
        auto singlInputBuffer = singlInput->GetInputBuffer();
        singlInputBuffer->Signal();

        for (auto itr = singlInputBuffer->begin(); itr != singlInputBuffer->end(); itr++)
        {
                if (itr.IsKeyPressFrame(KeyCode::ControlRight))
                        std::cout << "C\n";
                if (itr->IsKeyCodeSetHeld(KeyCodeSet({KeyCode::A, KeyCode::ShiftLeft, KeyCode::S})))
                        std::cout << "HelloSIMD\n";
                if (itr.IsKeyHeld(KeyCode::A))
                        std::cout << "A[][]";
        }
}
