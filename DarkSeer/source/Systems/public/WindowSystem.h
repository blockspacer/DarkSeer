#pragma once
struct WindowSystem
{
        void Initialize(EntityAdmin* entityAdmin);
        void Update(EntityAdmin* entityAdmin);
        void ShutDown(EntityAdmin* entityAdmin);
};