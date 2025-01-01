#include "Window.h"

bool DXWindow::Init()
{
    WNDCLASSEXW wnd{};
    wnd.cbSize = sizeof(wnd);
    wnd.style = CS_OWNDC;
    wnd.lpfnWndProc = &DXWindow::OnWindowMessage; // Callback function for WNDPROC
    wnd.cbClsExtra = 0; // Something about additional memory
    wnd.cbWndExtra = 0; // Something about additional memory
    wnd.hInstance = GetModuleHandle(nullptr); // Executable handle
    wnd.hIcon = LoadIconW(nullptr, IDI_APPLICATION); // Art icon for the window - IDI_APPLICATION provided by windows
    wnd.hCursor = LoadCursorW(nullptr, IDC_ARROW); // Art for cursor - IDC_ARROW provided by windows
    wnd.hbrBackground = nullptr; // Redraw background color in window when resized, not needed when using dx12
    wnd.lpszMenuName = nullptr; // Not using a menu
    wnd.lpszClassName = L"D3D12ExWndCls";
    wnd.hIconSm = LoadIconW(nullptr, IDI_APPLICATION); // Another icon
    
    // Register the window class on os
    m_WndClass = RegisterClassExW(&wnd);

    if (m_WndClass == 0)
    {
        return false;
    }

    // Start window on the monitor where the cursor is
    /*POINT pos{ 0, 0 };
    GetCursorPos(&pos);

    HMONITOR monitorHandle = MonitorFromPoint(pos, MONITOR_DEFAULTTOPRIMARY);

    MONITORINFO monitorInfo{};
    monitorInfo.cbSize = sizeof(monitorInfo);

    GetMonitorInfoW(monitorHandle, &monitorInfo);*/

    // For x and y below if above is uncommented
    // x - monitorInfo.rcWork.left + 100
    // y - monitorInfo.rcWork.top + 100

    // Actually create the window
    // WS_EX_OVERLAPPEDWINDOW is default window
    m_Window = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW | WS_EX_APPWINDOW, // window style
                              (LPCWSTR)m_WndClass, // Window class name that was just registered
                              L"DX12 Renderer", // Name of window
                              WS_OVERLAPPEDWINDOW | WS_VISIBLE, // something else to do with window style
                              0, // x position of monitor, starts from top left
                              0, // y position of monitor, starts from top left
                              1920, // Width
                              1080, // Height
                              nullptr, // Window parent
                              nullptr, // Window menu
                              wnd.hInstance, // Window instance
                              nullptr);

    return m_Window != nullptr;
}

void DXWindow::Shutdown()
{
    // If window is manually closed, destroy the window
    if (m_Window)
    {
        DestroyWindow(m_Window);
    }

    // Unregister window class on os
    if (m_WndClass)
    {
        UnregisterClassW((LPCWSTR)m_WndClass, GetModuleHandle(nullptr));
    }
}

void DXWindow::Update()
{
    MSG msg;

    // Poll for a window message in the event queue
    // pass in message pointer, the window instance, some filter min/max figure out what this is later, 
    // PM_REMOVE removes the message from the event queue after being processed
    while (PeekMessageW(&msg, m_Window, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);

        // Call the WNDPROC to update the window
        DispatchMessageW(&msg);
    }
}

LRESULT CALLBACK DXWindow::OnWindowMessage(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Handle windows messages
    switch (msg)
    {
    case WM_CLOSE: // Handle manual window close
        GetDXWindow().m_GameExit = true;
        return 0;
    }

    return DefWindowProcW(wnd, msg, wParam, lParam);
}
