struct WindowCreationDescriptor
{
        char    WindowTitle[32];
        char    WindowClassName[32];
        HWND    m_hwnd;
        WNDPROC wndProc;
        int     x;
        int     y;
        int     width;
        int     height;
        HWND    parent;
        HMENU   menu;
        DWORD   style;
        DWORD   exstyle;
        HBRUSH  hbrBackground;
};

struct CreatedWindow
{
        HWND    m_hwnd;
        WNDPROC m_wndproc;
};

struct WindowProxy
{
    private:
        WindowCreationDescriptor m_windowProxyDescriptor;

    public:
        WindowProxy();
        WindowProxy&  Title(const char* title);
        WindowProxy&  Position(int x, int y);
        WindowProxy&  Position(percent<float, float> pos);
        WindowProxy&  WindProc(WNDPROC wndProc);
        WindowProxy&  Size(int width, int height);
        WindowProxy&  Size(percent<float, float> size);
        WindowProxy&  BackgroundColor(unsigned r, unsigned g, unsigned b);
        CreatedWindow Create();
};
