#pragma once
struct SingletonInput;

namespace InputUtil
{
        void InitializeInputBuffer(SingletonInput* singlInput, HWND hwnd);
        void ReleaseInputBufferMemory(SingletonInput* singlInput);
}