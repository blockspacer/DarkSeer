#pragma once
struct SingletonInput;
struct SingletonWindow;
struct SingletonConsole;
struct SingletonSystemManager;
struct SingletonTimer;

struct EntityAdmin
{
    private:
        void LaunchSystems(SingletonSystemManager* singlSystemManager);
        void LaunchWnproc(SingletonWindow* singlWindow);
        void PostShutDown(SingletonInput* singlInput);

        SingletonInput*         m_singletonInput         = 0;
        SingletonWindow*        m_singletonWindow        = 0;
        SingletonConsole*       m_singletonConsole       = 0;
        SingletonSystemManager* m_singletonSystemManager = 0;
        SingletonTimer*         m_singletonTimer         = 0;
    public:
        SingletonInput*         GetSingletonInput();
        SingletonWindow*        GetSingletonWindow();
        SingletonConsole*       GetSingletonConsole();
        SingletonSystemManager* GetSingletonSystemManager();
        SingletonTimer*         GetSingletonTimer();

        void Initialize();
        void Update();
        void ShutDown();
};
inline EntityAdmin g_userEntityAdmin;