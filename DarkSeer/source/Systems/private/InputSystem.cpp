#include <ConsoleUtility.h>
#include <InputSystem.h>

#include <SingletonConsole.h>
#include <SingletonInput.h>

const ActivatedKeyCodeSet _private_set = ActivatedKeyCodeSet({KeyCode::A, KeyCode::ShiftLeft, KeyCode::S}, KeyCode::A);

void InputSystem::PreUpdate(EntityAdmin* entityAdmin)
{
        auto singlInput       = entityAdmin->GetSingletonInput();
        auto singlConsole     = entityAdmin->GetSingletonConsole();
        auto singlInputBuffer = singlInput->GetInputBuffer();
        singlInputBuffer->Signal();

        for (auto& itr : *singlInputBuffer)
        {
                if (itr.IsKeySetBeginPress(_private_set))
                        std::cout << "HelloSIMD\n";
        }
}
