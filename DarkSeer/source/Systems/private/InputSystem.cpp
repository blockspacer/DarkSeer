#include <ConsoleUtility.h>
#include <InputSystem.h>

#include <SingletonConsole.h>
#include <SingletonInput.h>

const ActivatedKeyCodeSet _private_set = ActivatedKeyCodeSet({KeyCode::A, KeyCode::ShiftLeft, KeyCode::S}, KeyCode::A);

void InputSystem::PreUpdate(EntityAdmin* entityAdmin)
{
        std::this_thread::sleep_for(std::chrono::seconds(2));
        auto singlInput       = entityAdmin->GetSingletonInput();
        auto singlConsole     = entityAdmin->GetSingletonConsole();
        auto singlInputBuffer = singlInput->GetInputBuffer();
        singlInputBuffer->Signal();
    
        for (auto& itr : *singlInputBuffer)
        {
                auto [Ax, Ay] = itr->m_absoluteMousePos;
                auto [Rx, Ry] = itr->m_mouseDeltas;
                std::cout << "[A]\tx:["<<Ax<<"],y:["<<Ay<<"]\n";
                std::cout << "[R]\t[" << Rx << "],y:[" << Ry << "]\n";
                if (itr.IsKeySetBeginPress(_private_set))
                        std::cout << "HelloSIMD\n";
        }
}
