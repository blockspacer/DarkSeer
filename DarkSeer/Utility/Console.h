#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <iostream>
inline namespace Console
{
        inline void InitializeConsole()
        {
                AllocConsole();
                auto success = freopen("CONOUT$", "w", stdout);
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
} // namespace Console
