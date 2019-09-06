#pragma once
struct SingletonWindow;
struct SingletonSystemManager;
struct SingletonTimer;
#include <SingletonSystemManager.h>

namespace SystemUtil
{
        void Shutdown(SingletonWindow* singlWindow, SingletonSystemManager* singlSystemManager);
} // namespace SystemUtil
