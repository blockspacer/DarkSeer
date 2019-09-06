#include <InputSystem.h>
#include <ConsoleUtility.h>

#include <SingletonConsole.h>
#include <SingletonInput.h>
void InputSystem::PreUpdate(EntityAdmin* entityAdmin)
{
        auto singlInput   = entityAdmin->GetSingletonInput();
        auto singlConsole = entityAdmin->GetSingletonConsole();

        singlInput->m_inputBuffer.Signal();
        for (auto& inputFrame : singlInput->m_inputBuffer)
        {
                std::string echo;
                if (inputFrame.IsKeyPress(KeyCode::C))
                {
                        ConsoleUtil::Begin(singlConsole);
                        std::cout << "enter a string\n";
                        std::cin >> echo;
                        for (auto& itr : echo)
                                itr = std::toupper(itr);
                        std::cout << echo << std::endl;
                        std::cout << "enter anything to continue\n";
                        std::cin >> echo;
                        ConsoleUtil::End(singlConsole);
                }
        }
}
