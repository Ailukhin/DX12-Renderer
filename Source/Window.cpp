#include "Window.h"

#ifdef _DEBUG
#include <DXDebugLayer.h>
#endif

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Forward hwnd on because we can get messages (e.g., WM_CREATE)
    // before CreateWindow returns, and thus before mhMainWnd is valid.
    return DXWindow::GetApp()->OnWindowMessage(hwnd, msg, wParam, lParam);
}

DXWindow::DXWindow()
{
    assert(m_App == nullptr);
    m_App = this;
}

DXWindow::~DXWindow()
{

}

DXWindow* DXWindow::m_App = nullptr;
DXWindow* DXWindow::GetApp()
{
    return m_App;
}

int DXWindow::Run()
{
    MSG msg = { 0 };

    m_Timer.Reset();

    while (!GameExit())
    {
        // Poll for a window message in the event queue
        // pass in message pointer, the window instance, some filter min/max figure out what this is later, 
        // PM_REMOVE removes the message from the event queue after being processed
        if (PeekMessageW(&msg, m_Window, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);

            // Call the WNDPROC to update the window
            DispatchMessageW(&msg);
        }
        else
        {
            m_Timer.Tick();

            if (!m_AppPaused)
            {
                CalculateFrameStats();
                Update(m_Timer);
                Draw(m_Timer);
            }
            else
            {
                Sleep(100);
            }
        }
    }

    // Cleanup

    // Flush command queue
    FlushCommandQueue(GetBufferCount());

    Shutdown();

#ifdef _DEBUG
    DXDebugLayer::GetDXDebug().Shutdown();
#endif

    return (int)msg.wParam;
}

bool DXWindow::Init()
{
#ifdef _DEBUG
    if (!DXDebugLayer::GetDXDebug().Init())
    {
        return false;
    }
#endif

    if (!InitWindow())
    {
        return false;
    }

    if (!InitD3D())
    {
        return false;
    }

    // Do initial resize
    ResizeBuffers();

    return true;
}

void DXWindow::Shutdown()
{
    //
    // Shut down window
    //
    ReleaseBuffers();

    m_RtvDescHeap.Release();
    m_DsvDescHeap.Release();

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

    //
    // Now shut down D3D
    //
    m_CmdList.Release();
    m_CmdAllocator.Release();

    if (m_FenceEvent)
    {
        CloseHandle(m_FenceEvent);
    }

    m_Fence.Release();
    m_CmdQueue.Release();
    m_Device.Release();
    m_DxgiFactory.Release();
}

LRESULT CALLBACK DXWindow::OnWindowMessage(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Handle windows messages
    switch (msg)
    {
        // WM_ACTIVATE is sent when the window is activated or deactivated.  
        // We pause the game when the window is deactivated and unpause it 
        // when it becomes active.  
    case WM_ACTIVATE:
        if (LOWORD(wParam) == WA_INACTIVE)
        {
            m_AppPaused = true;
            m_Timer.Stop();
        }
        else
        {
            m_AppPaused = false;
            m_Timer.Start();
        }
        break;

    case WM_KEYDOWN:
        if (wParam == VK_F11)
        {
            SetFullScreen(!DXWindow::GetApp()->IsFullScreen());
        }
        else if (wParam == VK_ESCAPE)
        {
            m_GameExit = true;
        }
        break;

    case WM_SIZE:
        // Save the new client area dimensions.
        m_Width = LOWORD(lParam);
        m_Height = HIWORD(lParam);
        if (m_Device)
        {
            if (wParam == SIZE_MINIMIZED)
            {
                m_AppPaused = true;
                m_Minimized = true;
                m_Maximized = false;
            }
            else if (wParam == SIZE_MAXIMIZED)
            {
                m_AppPaused = false;
                m_Minimized = false;
                m_Maximized = true;
                m_Resize = true;
            }
            else if (wParam == SIZE_RESTORED)
            {

                // Restoring from minimized state?
                if (m_Minimized)
                {
                    m_AppPaused = false;
                    m_Minimized = false;
                    m_Resize = true;
                }

                // Restoring from maximized state?
                else if (m_Maximized)
                {
                    m_AppPaused = false;
                    m_Maximized = false;
                    m_Resize = true;
                }
                else if (m_isResizing)
                {
                    // If user is dragging the resize bars, we do not resize 
                    // the buffers here because as the user continuously 
                    // drags the resize bars, a stream of WM_SIZE messages are
                    // sent to the window, and it would be pointless (and slow)
                    // to resize for each WM_SIZE message received from dragging
                    // the resize bars.  So instead, we reset after the user is 
                    // done resizing the window and releases the resize bars, which 
                    // sends a WM_EXITSIZEMOVE message.
                }
                else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
                {
                    m_Resize = true;
                }
            }
        }
        break;

        // WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
    case WM_ENTERSIZEMOVE:
        m_AppPaused = true;
        m_isResizing = true;
        m_Timer.Stop();
        break;

        // WM_EXITSIZEMOVE is sent when the user releases the resize bars.
        // Here we reset everything based on the new window dimensions.
    case WM_EXITSIZEMOVE:
        m_AppPaused = false;
        m_isResizing = false;
        m_Timer.Start();
        m_Resize = true;
        break;

        // Catch this message so to prevent the window from becoming too small.
    case WM_GETMINMAXINFO:
        ((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
        ((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
        break;

    case WM_CLOSE: // Handle manual window close
        m_GameExit = true;
        break;
    }

    return DefWindowProcW(wnd, msg, wParam, lParam);
}

bool DXWindow::Get4xMsaaState() const
{
    return m4xMsaaState;
}

void DXWindow::Set4xMsaaState(bool value)
{
    if (m4xMsaaState != value)
    {
        m4xMsaaState = value;

        // Recreate the swapchain and buffers with new multisample settings.
        CreateSwapChain();
        ResizeBuffers();
    }
}

void DXWindow::Present()
{
    // 1st param, 0 = present immediately no sync (can tear), 1-4  sync presentation after nth vertical blank (frame?)
    // 2nd param - https://learn.microsoft.com/en-us/windows/win32/direct3ddxgi/dxgi-present
    m_SwapChain->Present(1, 0);
}

void DXWindow::ResizeBuffers()
{
    // Buffer references must be released before resizing
    ReleaseBuffers();

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

    // Now get new buffer references after resize
    CreateRtvAndDsvBuffers();

    // Viewport for client area
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    vp.Width = (float)m_Width;
    vp.Height = (float)m_Height;
    vp.MinDepth = 1.0f;
    vp.MaxDepth = 0.0f;

    scRect.left = 0;
    scRect.top = 0;
    scRect.right = m_Width;
    scRect.bottom = m_Height;
}

bool DXWindow::CreateRtvAndDsvDescriptorHeaps()
{
    // Create Render Target View Heap
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.NumDescriptors = m_BufferCount;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;

    if (FAILED(m_Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_RtvDescHeap))))
    {
        return false;
    }

    // Handles for render target views
    for (size_t i = 0; i < m_BufferCount; i++)
    {
        m_RtvHandles[i] = m_RtvDescHeap->GetCPUDescriptorHandleForHeapStart();
        m_RtvHandles[i].ptr += mRtvDescriptorSize * i;
    }

    // Depth stencil view heap
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvHeapDesc.NodeMask = 0;

    if (FAILED(m_Device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_DsvDescHeap))))
    {
        return false;
    }

    // Handle for depth stencil view
    m_DsvHandle = m_DsvDescHeap->GetCPUDescriptorHandleForHeapStart();

    if (!CreateRtvAndDsvBuffers())
    {
        return false;
    }

    return true;
}

bool DXWindow::CreateCommandObjects()
{
    // Create command queue descriptor and use it to create a command queue
    D3D12_COMMAND_QUEUE_DESC cmdQueueDesc{};
    cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // Direct is general purpose probably not best perf everywhere, use more specific types later on
    cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH; // Normal is for normal apps, High is for games, Global real-time not sure what this is
    cmdQueueDesc.NodeMask = 0; // 0 is default setting, node mask is basically what gpu is being used
    cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE; // None and gpu timeout, gpu timeout cancels work after some amount of time where no tasks are completed?

    if (FAILED(m_Device->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&m_CmdQueue))))
    {
        printf("-- FAILED TO CREATE COMMAND QUEUE --\n");
        return false;
    }

    // Command allocator
    if (FAILED(m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CmdAllocator))))
    {
        return false;
    }

    // Command list
    if (FAILED(m_Device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&m_CmdList))))
    {
        return false;
    }
    else
    {
        //PrintCommandListSupportLevel();
    }

    return true;
}

bool DXWindow::CreateSwapChain()
{
    // Swap chain descriptor
    DXGI_SWAP_CHAIN_DESC1 swap_desc{};
    swap_desc.Width = 2560;
    swap_desc.Height = 1440;
    swap_desc.Format = mBackBufferFormat; // Normal format for 8 bits in each color channel
    swap_desc.Stereo = false;
    swap_desc.SampleDesc.Count = 1; // 1 pixel per pixel
    swap_desc.SampleDesc.Quality = 0; // Multisampling (anti-aliasing), 1 yes, 0 no
    swap_desc.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT; // Back buffer to write to as the render target
    swap_desc.BufferCount = 2; // 2 buffers for swapping, front is displayed while back is being written to - "swap chain"
    // 3 used for vsync to help prevent tearing 
    // with vsync, must wait for monitor to finish using a buffer, completed buffer will be waiting for sawp while 3rd is being written to
    // Vsync is basically doing work ahead of time and waiting to make sure the buffer swap is smooth
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
    m_DxgiFactory->CreateSwapChainForHwnd(m_CmdQueue, m_Window, &swap_desc, &swap_fullscreen_desc, nullptr, &swapChain1);

    if (!swapChain1.QueryInterface(m_SwapChain))
    {
        return false;
    }

    return true;
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
                SWP_NOZORDER);
        }
    }
    else // Maximize when deactiving fullscreen
    {
        ShowWindow(m_Window, SW_MAXIMIZE);
    }

    m_isFullscreen = enable;
}

void DXWindow::CalculateFrameStats()
{
    // Code computes the average frames per second, and also the 
    // average time it takes to render one frame.  These stats 
    // are appended to the window caption bar.
    static int frameCnt = 0;
    static float timeElapsed = 0.0f;

    frameCnt++;

    // Compute averages over one second period.
    if ((m_Timer.TotalTime() - timeElapsed) >= 1.0f)
    {
        float fps = (float)frameCnt; // fps = frameCnt / 1
        float mspf = 1000.0f / fps;

        wstring fpsStr = to_wstring(fps);
        wstring mspfStr = to_wstring(mspf);

        wstring windowText = m_WindowCaption +
            L"    fps: " + fpsStr +
            L"   mspf: " + mspfStr;

        SetWindowText(m_Window, windowText.c_str());

        // Reset for next average.
        frameCnt = 0;
        timeElapsed += 1.0f;
    }
}

void DXWindow::BeginFrame(ID3D12GraphicsCommandList6* cmdList)
{
    // Update the current buffer index to start drawing the frame to the correct buffer
    m_CurrentBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();

    // State transition on resource usage to render_target for the back buffer to now be rendered to
    D3D12_RESOURCE_BARRIER barrier;
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = m_Buffers[m_CurrentBufferIndex];
    barrier.Transition.Subresource = 0;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

    cmdList->ResourceBarrier(1, &barrier);
    
    // Clear back buffer and depth buffer
    float clearColor[] = { 0.6f, 0.8f, 0.9f, 1.0f };
    cmdList->ClearRenderTargetView(m_RtvHandles[m_CurrentBufferIndex], clearColor, 0, nullptr);
    cmdList->ClearDepthStencilView(m_DsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    // Specify the buffer that will now be rendered to
    cmdList->OMSetRenderTargets(1, &m_RtvHandles[m_CurrentBufferIndex], false, &m_DsvHandle);
}

void DXWindow::EndFrame(ID3D12GraphicsCommandList6* cmdList)
{
    // State transition on resource usage to present for the back buffer which is now ready to present
    D3D12_RESOURCE_BARRIER barrier;
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = m_Buffers[m_CurrentBufferIndex];
    barrier.Transition.Subresource = 0;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

    cmdList->ResourceBarrier(1, &barrier);
}

bool DXWindow::InitWindow()
{
    WNDCLASSEXW wnd{};
    wnd.cbSize = sizeof(wnd);
    wnd.style = CS_OWNDC;
    wnd.lpfnWndProc = MainWndProc; // Callback function for WNDPROC
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
        m_WindowCaption.c_str(), // Name of window
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, // something else to do with window style
        0, // x position of monitor, starts from top left
        0, // y position of monitor, starts from top left
        2560, // Width
        1440, // Height
        nullptr, // Window parent
        nullptr, // Window menu
        wnd.hInstance, // Window instance
        nullptr);


    if (m_Window == nullptr)
    {
        return false;
    }

    return true;
}

bool DXWindow::InitD3D()
{
    // Create dxgi factory
    if (FAILED(CreateDXGIFactory2(0, IID_PPV_ARGS(&m_DxgiFactory))))
    {
        return false;
    }

    // Create Device
    if (FAILED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_Device))))
    {
        //m_Device->CheckFeatureSupport(---);

        //PrintDeviceSupportLevel();

        printf("-- FAILED TO CREATE DX12 DEVICE --\n");

        return false;
    }

    // Create a fence
    // A fence does not need a descriptor when created
    if (FAILED(m_Device->CreateFence(m_FenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence))))
    {
        printf("-- FAILED TO CREATE FENCE --\n");
        return false;
    }

    // Fence event
    m_FenceEvent = CreateEvent(nullptr, false, false, nullptr);
    assert(m_FenceEvent != nullptr);

    // Descriptor handle sizes
    mRtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    mDsvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    mCbvSrvUavDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    if (!CreateCommandObjects())
    {
        return false;
    }

    if (!CreateSwapChain())
    {
        return false;
    }

    if (!CreateRtvAndDsvDescriptorHeaps())
    {
        return false;
    }
    
#ifdef _DEBUG
    LogAdapters();
#endif

    return true;
}

ID3D12GraphicsCommandList6* DXWindow::InitCommandList()
{
    // Reset allocator and command list
    m_CmdAllocator->Reset();
    m_CmdList->Reset(m_CmdAllocator, nullptr);

    return m_CmdList;
}

void DXWindow::ExecuteCommandList()
{
    // Need to close the list before execution
    if (SUCCEEDED(m_CmdList->Close()))
    {
        // Add command lists to queue for execution
        ID3D12CommandList* lists[] = { m_CmdList };
        m_CmdQueue->ExecuteCommandLists(_countof(lists), lists);

        SignalAndWait();
    }
}

void DXWindow::SignalAndWait()
{
    m_CmdQueue->Signal(m_Fence, ++m_FenceValue);

    // Fence notifies the event when it gets a completion signal
    if (SUCCEEDED(m_Fence->SetEventOnCompletion(m_FenceValue, m_FenceEvent)))
    {
        // If event isn't notified within 20s, close
        if (WaitForSingleObject(m_FenceEvent, 20000) != WAIT_OBJECT_0)
        {
            std::exit(-1);
        }
    }
    else
    {
        // End the program
        std::exit(-1);
    }
}

void DXWindow::LogAdapters()
{
    UINT i = 0;
    IDXGIAdapter* adapter = nullptr;
    std::vector<IDXGIAdapter*> adapterList;
    while (m_DxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_ADAPTER_DESC desc;
        adapter->GetDesc(&desc);

        std::wstring text = L"***Adapter: ";
        text += desc.Description;
        text += L"\n";

        OutputDebugString(text.c_str());

        adapterList.push_back(adapter);

        ++i;
    }

    for (size_t i = 0; i < adapterList.size(); ++i)
    {
        LogAdapterOutputs(adapterList[i]);
        ReleaseCom(adapterList[i]);
    }
}

void DXWindow::LogAdapterOutputs(IDXGIAdapter* adapter)
{
    UINT i = 0;
    IDXGIOutput* output = nullptr;
    while (adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_OUTPUT_DESC desc;
        output->GetDesc(&desc);

        std::wstring text = L"***Output: ";
        text += desc.DeviceName;
        text += L"\n";
        OutputDebugString(text.c_str());

        // Hard set format for now, but it will eventually be a protected member to be specified by user
        LogOutputDisplayModes(output, DXGI_FORMAT_R8G8B8A8_UNORM);

        ReleaseCom(output);

        ++i;
    }
}

void DXWindow::LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format)
{
    UINT count = 0;
    UINT flags = 0;

    // Call with nullptr to get list count.
    output->GetDisplayModeList(format, flags, &count, nullptr);

    std::vector<DXGI_MODE_DESC> modeList(count);
    output->GetDisplayModeList(format, flags, &count, &modeList[0]);

    for (auto& x : modeList)
    {
        UINT n = x.RefreshRate.Numerator;
        UINT d = x.RefreshRate.Denominator;
        std::wstring text =
            L"Width = " + std::to_wstring(x.Width) + L" " +
            L"Height = " + std::to_wstring(x.Height) + L" " +
            L"Refresh = " + std::to_wstring(n) + L"/" + std::to_wstring(d) +
            L"\n";

        ::OutputDebugString(text.c_str());
    }
}

void DXWindow::PrintDeviceSupportLevel()
{
    // Check Device support
    ComPointer<ID3D12Device1> dev1;
    HRESULT hr;
    if (m_Device.QueryInterface(dev1, &hr))
    {
        printf("ID3D12 Device 1 is supported.\n");
    }
    else
    {
        printf("ID3D12 Device 1 is NOT supported.\n");
    }

    ComPointer<ID3D12Device2> dev2;
    if (m_Device.QueryInterface(dev2, &hr))
    {
        printf("ID3D12 Device 2 is supported.\n");
    }
    else
    {
        printf("ID3D12 Device 2 is NOT supported.\n");
    }

    ComPointer<ID3D12Device3> dev3;
    if (m_Device.QueryInterface(dev3, &hr))
    {
        printf("ID3D12 Device 3 is supported.\n");
    }
    else
    {
        printf("ID3D12 Device 3 is NOT supported.\n");
    }

    ComPointer<ID3D12Device4> dev4;
    if (m_Device.QueryInterface(dev4, &hr))
    {
        printf("ID3D12 Device 4 is supported.\n");
    }
    else
    {
        printf("ID3D12 Device 4 is NOT supported.\n");
    }

    ComPointer<ID3D12Device5> dev5;
    if (m_Device.QueryInterface(dev5, &hr))
    {
        printf("ID3D12 Device 5 is supported.\n");
    }
    else
    {
        printf("ID3D12 Device 5 is NOT supported.\n");
    }

    ComPointer<ID3D12Device6> dev6;
    if (m_Device.QueryInterface(dev6, &hr))
    {
        printf("ID3D12 Device 6 is supported.\n");
    }
    else
    {
        printf("ID3D12 Device 6 is NOT supported.\n");
    }

    ComPointer<ID3D12Device7> dev7;
    if (m_Device.QueryInterface(dev7, &hr))
    {
        printf("ID3D12 Device 7 is supported.\n");
    }
    else
    {
        printf("ID3D12 Device 7 is NOT supported.\n");
    }

    ComPointer<ID3D12Device8> dev8;
    if (m_Device.QueryInterface(dev8, &hr))
    {
        printf("ID3D12 Device 8 is supported.\n");
    }
    else
    {
        printf("ID3D12 Device 8 is NOT supported.\n");
    }

    ComPointer<ID3D12Device9> dev9;
    if (m_Device.QueryInterface(dev9, &hr))
    {
        printf("ID3D12 Device 9 is supported.\n");
    }
    else
    {
        printf("ID3D12 Device 9 is NOT supported.\n");
    }

    ComPointer<ID3D12Device10> dev10;
    if (m_Device.QueryInterface(dev10, &hr))
    {
        printf("ID3D12 Device 10 is supported.\n");
    }
    else
    {
        printf("ID3D12 Device 10 is NOT supported.\n");
    }
}

void DXWindow::PrintCommandListSupportLevel()
{
    ComPointer<ID3D12GraphicsCommandList1> cmdList1;
    HRESULT hr;

    if (m_CmdList.QueryInterface(cmdList1, &hr))
    {
        printf("ID3D12 Command List 1 is supported.\n");
    }
    else
    {
        printf("ID3D12 Command List 1 is NOT supported.\n");
    }

    ComPointer<ID3D12GraphicsCommandList2> cmdList2;
    if (m_CmdList.QueryInterface(cmdList2, &hr))
    {
        printf("ID3D12 Command List 2 is supported.\n");
    }
    else
    {
        printf("ID3D12 Command List 2 is NOT supported.\n");
    }

    ComPointer<ID3D12GraphicsCommandList3> cmdList3;
    if (m_CmdList.QueryInterface(cmdList3, &hr))
    {
        printf("ID3D12 Command List 3 is supported.\n");
    }
    else
    {
        printf("ID3D12 Command List 3 is NOT supported.\n");
    }

    ComPointer<ID3D12GraphicsCommandList4> cmdList4;
    if (m_CmdList.QueryInterface(cmdList4, &hr))
    {
        printf("ID3D12 Command List 4 is supported.\n");
    }
    else
    {
        printf("ID3D12 Command List 4 is NOT supported.\n");
    }

    ComPointer<ID3D12GraphicsCommandList5> cmdList5;
    if (m_CmdList.QueryInterface(cmdList5, &hr))
    {
        printf("ID3D12 Command List 5 is supported.\n");
    }
    else
    {
        printf("ID3D12 Command List 5 is NOT supported.\n");
    }

    ComPointer<ID3D12GraphicsCommandList6> cmdList6;
    if (m_CmdList.QueryInterface(cmdList6, &hr))
    {
        printf("ID3D12 Command List 6 is supported.\n");
    }
    else
    {
        printf("ID3D12 Command List 6 is NOT supported.\n");
    }

    ComPointer<ID3D12GraphicsCommandList7> cmdList7;
    if (m_CmdList.QueryInterface(cmdList7, &hr))
    {
        printf("ID3D12 Command List 7 is supported.\n");
    }
    else
    {
        printf("ID3D12 Command List 7 is NOT supported.\n");
    }
}

bool DXWindow::CreateRtvAndDsvBuffers()
{
    ID3D12GraphicsCommandList6* cmdList = InitCommandList();

    // Rtv
    for (UINT i = 0; i < m_BufferCount; i++)
    {
        if (FAILED(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_Buffers[i]))))
        {
            return false;
        }

        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
        rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Texture2D.MipSlice = 0;
        rtvDesc.Texture2D.PlaneSlice = 0;

        m_Device->CreateRenderTargetView(m_Buffers[i], &rtvDesc, m_RtvHandles[i]);
    }

    // Dsv
    D3D12_RESOURCE_DESC depthStencilDesc;
    depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthStencilDesc.Alignment = 0;
    depthStencilDesc.Width = m_Width;
    depthStencilDesc.Height = m_Height;
    depthStencilDesc.DepthOrArraySize = 1;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    depthStencilDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
    depthStencilDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
    depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE optClear;
    optClear.Format = mDepthStencilFormat;
    optClear.DepthStencil.Depth = 1.0f;
    optClear.DepthStencil.Stencil = 0;

    D3D12_HEAP_PROPERTIES hpDefault{};
    hpDefault.Type = D3D12_HEAP_TYPE_DEFAULT;
    hpDefault.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    hpDefault.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    hpDefault.CreationNodeMask = 0;
    hpDefault.VisibleNodeMask = 0;
    
    m_Device->CreateCommittedResource(&hpDefault, D3D12_HEAP_FLAG_NONE, &depthStencilDesc, D3D12_RESOURCE_STATE_COMMON, &optClear, IID_PPV_ARGS(&m_DepthStencilBuffer));

    // Create descriptor to mip level 0 of entire resource using the format of the resource.
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Format = mDepthStencilFormat;
    dsvDesc.Texture2D.MipSlice = 0;
    m_Device->CreateDepthStencilView(m_DepthStencilBuffer, &dsvDesc, m_DsvHandle);

    // Transition the resource from its initial state to be used as a depth buffer.
    D3D12_RESOURCE_BARRIER barrier;
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = m_DepthStencilBuffer;
    barrier.Transition.Subresource = 0;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;

    cmdList->ResourceBarrier(1, &barrier);

    // Execute resize commands
    ExecuteCommandList();

    // Wait until resize is done
    FlushCommandQueue(GetBufferCount());

    return true;
}

void DXWindow::ReleaseBuffers()
{
    for (UINT i = 0; i < m_BufferCount; i++)
    {
        m_Buffers[i].Release();
    }

    m_DepthStencilBuffer.Release();
}
