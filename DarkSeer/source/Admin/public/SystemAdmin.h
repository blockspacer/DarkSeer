#pragma once
struct EntityAdmin;
struct SingletonTimer;
#include <SingletonSystemManager.h>

struct SystemManager
{

		void ShutDown(EntityAdmin* entityAdmin);
};
inline SystemManager g_userSystemManager;