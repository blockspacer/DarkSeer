#pragma once
struct SingletonInput;
struct SingletonWindow;

struct Admin
{
    private:
        SingletonInput* m_singletonInput = 0;
    public:
        void            Initialize();
        void            Shutdown();
        SingletonInput* GetSingletonInput();
};
inline Admin admin;