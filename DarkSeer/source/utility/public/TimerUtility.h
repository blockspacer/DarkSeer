#pragma once
struct SingletonTimer;

namespace TimerUtil
{
        void ResetTimers(SingletonTimer* singlTimer);
        void Signal(SingletonTimer* singlTimer);
} // namespace TimerUtil