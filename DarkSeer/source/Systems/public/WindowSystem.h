#pragma once
struct WindowSystem
{
    private:
        EntityAdmin* m_Admin = &g_userEntityAdmin;
    public:
        void Initialize();
        void Update();
        void ShutDown();
};