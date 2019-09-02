#pragma once
struct SingletonInput;
struct SingletonWindow;

struct Admin
{
    private:
        void LaunchMessageLoop();
    private:
        SingletonInput*  m_singletonInput  = 0;
        SingletonWindow* m_singletonWindow = 0;

    public:
        SingletonInput*  GetSingletonInput();
        SingletonWindow* GetSingletonWindow();

    public:
        void Initialize();
        void ShutDown();
        void PostShutDown();
};
inline Admin g_userAdmin;