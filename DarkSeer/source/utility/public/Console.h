#pragma once
inline namespace Console
{
        inline namespace Globals
        {
                inline bool g_consoleActivated     = false;
                inline HWND g_prevForegroundWindow = 0;
        } // namespace Globals
        inline void InitializeConsole()
        {
                AllocConsole();
                bool success;
                success = freopen("CONOUT$", "w", stdout);
                success = freopen("CONIN$", "r", stdin);
                success = freopen("CONOUT$", "w", stderr);
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
        inline void ShutdownConsole()
        {
                fclose(stdout);
                FreeConsole();
        }
        inline void Begin()
        {
                g_consoleActivated      = true;
                g_prevForegroundWindow  = GetForegroundWindow();
                auto ConsoleInputHandle = GetStdHandle(STD_INPUT_HANDLE);
                FlushConsoleInputBuffer(ConsoleInputHandle);
                SetForegroundWindow(GetConsoleWindow());
        }
        inline void End()
        {
                g_consoleActivated = false;
                SetForegroundWindow(g_prevForegroundWindow);
        }
        inline float GetFloatNoFail(std::string promptMessage)
        {
                std::string cinStrBuffer;

                std::string::size_type read_count = 0;
                float                  cin_value  = 0;

                do
                {
                        std::cout << promptMessage;
                        std::getline(std::cin, cinStrBuffer);
                        try
                        {
                                cin_value = std::stof(cinStrBuffer, &read_count);
                        }
                        catch (std::invalid_argument)
                        {}
                } while (!read_count || read_count != cinStrBuffer.size());

                return cin_value;
        }
        inline bool IsActive()
        {
                return g_consoleActivated;
        }
} // namespace Console
