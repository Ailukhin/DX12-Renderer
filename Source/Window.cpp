#include "Window.h"
#include "DXContext.h"

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

    
    if (m_Window == nullptr)
    {
        return false;
    }
    

    // Swap chain descriptor
    DXGI_SWAP_CHAIN_DESC1 swap_desc{};
    swap_desc.Width = 1920;
    swap_desc.Height = 1080;
    swap_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // Normal format for 8 bits in each color channel
    swap_desc.Stereo = false;
    swap_desc.SampleDesc.Count = 1; // 1 pixel per pixel
    swap_desc.SampleDesc.Quality = 0; // Multisampling (anti-aliasing), 1 yes, 0 no
    swap_desc.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT; // Back buffer to write to as the render target
    swap_desc.BufferCount = 2; // 2 buffers for swapping, front is displayed while back is being written to - "swap chain"
                               // 3 used for vsync to help prevent tearing 
                               // with vsync, must wait for monitor to finish using a buffer, completed buffer will be waiting for sawp while 3rd is being written to
                               // Vsync is basically doing work ahead of time and waiting to make sure the buffer swap is smooth
    // Set internal buffer/frame count for flush later
    m_BufferCount = swap_desc.BufferCount;
    swap_desc.Scaling = DXGI_SCALING_STRETCH; // How the swap chain behaves if the window is resized
    swap_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // How swapping the buffers is handled
    swap_desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE; // Mode for alpha blending
    swap_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING; // Allow swap chain to be modifiable, allow tearing aka no vsync

    // Full screen swap chain descriptor
    DXGI_SWAP_CHAIN_FULLSCREEN_DESC swap_fullscreen_desc{};
    //swap_fullscreen_desc.RefreshRate; // These 3 values not used since widowed = true
    //swap_fullscreen_desc.ScanlineOrdering;
    //swap_fullscreen_desc.Scaling;
    swap_fullscreen_desc.Windowed = true;

    ComPointer<IDXGISwapChain1> swapChain1;
    
    // Set up swap chain
    auto& factory = DXContext::GetDXContext().GetDXGIFactory();
    factory->CreateSwapChainForHwnd(DXContext::GetDXContext().GetCommandQueue(), m_Window, &swap_desc, &swap_fullscreen_desc, nullptr, &swapChain1);

    if (!swapChain1.QueryInterface(m_SwapChain))
    {
        return false;
    }
    
    return true;
}

void DXWindow::Shutdown()
{
    m_SwapChain.Release();

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

void DXWindow::Present()
{
    // 1st param, 0 = present immediately no sync (can tear), 1-4  sync presentation after nth vertical blank (frame?)
    // 2nd param - https://learn.microsoft.com/en-us/windows/win32/direct3ddxgi/dxgi-present
    m_SwapChain->Present(1, 0);
}

void DXWindow::Resize()
{
    RECT clientRect;
    if (GetClientRect(m_Window, &clientRect))
    {
        m_Width = clientRect.right - clientRect.left;
        m_Height = clientRect.bottom - clientRect.top;

        // DXGI_FORMAT_UNKNOWN - keep same format as before
        // DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING - same as when creating the swap chain with descriptor in Init
        m_SwapChain->ResizeBuffers(m_BufferCount, m_Width, m_Height, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING);
        m_Resize = false;
    }
}

void DXWindow::SetFullScreen(bool enable)
{
    // Update window styling
    DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
    DWORD exStyle = WS_EX_OVERLAPPEDWINDOW | WS_EX_APPWINDOW;

    if (enable)
    {
        style = WS_POPUP | WS_VISIBLE;
        exStyle = WS_EX_APPWINDOW;
    }

    // Set internal style of window
    SetWindowLongW(m_Window, GWL_STYLE, style);
    SetWindowLongW(m_Window, GWL_EXSTYLE, exStyle);

    // Adjust window size
    if (enable)
    {
        HMONITOR monitorHandle = MonitorFromWindow(m_Window, MONITOR_DEFAULTTONEAREST); // Get the monitor the window is currently living on
        MONITORINFO monitorInfo{};
        monitorInfo.cbSize = sizeof(monitorInfo);
        
        if (GetMonitorInfoW(monitorHandle, &monitorInfo))
        {
            // Change window position and size to the size of the monitor
            SetWindowPos(m_Window,
                nullptr,
                monitorInfo.rcMonitor.left,
                monitorInfo.rcMonitor.top,
                monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
                monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
                SWP_NOZORDER
                );
        }
    }
    else // Maximize when deactiving fullscreen
    {
        ShowWindow(m_Window, SW_MAXIMIZE);
    }

    m_isFullscreen = enable;
}

LRESULT CALLBACK DXWindow::OnWindowMessage(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Handle windows messages
    switch (msg)
    {
    case WM_KEYDOWN: // When a key is pressed
        if (wParam == VK_F11) // F11 key for full screen
        {
            GetDXWindow().SetFullScreen(!GetDXWindow().IsFullScreen());
        }
        break;
    case WM_SIZE: // Window resize
        // Checks to make sure resize isn't triggering for minimizing and maximizing the window
        if (lParam && HIWORD(lParam) != GetDXWindow().m_Height && LOWORD(lParam) != GetDXWindow().m_Width)
        {
            GetDXWindow().m_Resize = true;
        }
        break;
    case WM_CLOSE: // Handle manual window close
        GetDXWindow().m_GameExit = true;
        return 0;
    }

    return DefWindowProcW(wnd, msg, wParam, lParam);
}
