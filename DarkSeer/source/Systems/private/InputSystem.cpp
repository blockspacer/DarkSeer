#include <InputSystem.h>
#include <ConsoleUtility.h>

#include <SingletonConsole.h>
#include <SingletonInput.h>
void InputSystem::PreUpdate(EntityAdmin* entityAdmin)
{
        auto singlInput   = entityAdmin->GetSingletonInput();
        auto singlConsole = entityAdmin->GetSingletonConsole();
        auto singlInputBuffer = singlInput->GetInputBuffer();
        singlInputBuffer->Signal();
        for (auto itr = singlInputBuffer->begin(); itr != singlInputBuffer->end(); itr++)
        {
                if (itr.IsKeyBeginPressFrame(KeyCode::C))
					std::cout << "C\n";
        }
}
