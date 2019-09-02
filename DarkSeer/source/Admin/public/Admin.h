#pragma once
struct SingletonInput;
struct SingletonWindow;

struct Admin
{
    private:
        SingletonInput* m_singletonInput = 0;

    public:
        SingletonInput* GetSingletonInput();

    public:
        void            Initialize();
        void            Shutdown();
};
inline Admin g_userAdmin;