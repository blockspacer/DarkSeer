#pragma once
struct InputSystem
{
    private:
        void RegisterRawInputDevices();
    public:
        void Initialize();
        void Shutdown();
};