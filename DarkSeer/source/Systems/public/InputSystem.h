#pragma once
struct InputSystem
{
        static void Initialize(EntityAdmin* entityAdmin);
        static void Update(EntityAdmin* entityAdmin);
        static void ShutDown(EntityAdmin* entityAdmin);
};