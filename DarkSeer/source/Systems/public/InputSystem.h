#pragma once
struct InputSystem
{
    private:
        Admin* m_Admin = &g_userAdmin;
    public:
        void Initialize();
        void Shutdown();
};