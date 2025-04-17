#include "D3D_UTIL.h"
#include "ComPointer.h"
#include <cassert>
#include "GameTimer.h"
#include <string>
using namespace std;

class DXWindow
{
	// Singleton
protected:
	DXWindow();
	DXWindow(const DXWindow&) = delete;
	DXWindow& operator= (const DXWindow&) = delete;
	virtual ~DXWindow();

	// 
public:
	static DXWindow* GetApp();

	int Run();

	virtual bool Init();

	LRESULT CALLBACK OnWindowMessage(HWND wnd, UINT msg, WPARAM wParam, LPARAM aParam);

	bool Get4xMsaaState()const;
	void Set4xMsaaState(bool value);

	inline bool GameExit() const
	{
		return m_GameExit;
	}

	inline UINT GetBufferCount() const
	{
		return m_BufferCount;
	}

	inline bool ShouldResize() const
	{
		return m_Resize;
	}

	inline bool IsFullScreen() const
	{
		return m_isFullscreen;
	}

	inline UINT GetWindowWidth() const
	{
		return m_Width;
	}

	inline UINT GetWindowHeight() const
	{
		return m_Height;
	}

protected:
	virtual void Update(const GameTimer& timer) = 0;
	virtual void Draw(const GameTimer& timer) = 0;
	virtual void ResizeBuffers();

	virtual bool CreateRtvAndDsvDescriptorHeaps();
	bool CreateCommandObjects();
	bool CreateSwapChain();

	virtual void Shutdown();

	void SetFullScreen(bool enable);

	void Present();

	void CalculateFrameStats();

	void BeginFrame(ID3D12GraphicsCommandList6* cmdList);
	void EndFrame(ID3D12GraphicsCommandList6* cmdList);

protected:
	bool InitWindow();
	bool InitD3D();

	ID3D12GraphicsCommandList6* InitCommandList();
	void ExecuteCommandList();

	void SignalAndWait();

	inline void FlushCommandQueue(UINT count)
	{
		for (UINT i = 0; i < count; i++)
		{
			SignalAndWait();
		}
	}

	void LogAdapters(); // Prints some monitor display information
	void LogAdapterOutputs(IDXGIAdapter* adapter);
	void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);

	void PrintDeviceSupportLevel();
	void PrintCommandListSupportLevel();

private:
	bool GetBuffers();
	void ReleaseBuffers();

protected:
	//
	// Window / App 
	//
	static DXWindow* m_App;

	ATOM m_WndClass = 0;
	HWND m_Window = nullptr;
	bool m_GameExit = false;
	bool m_Resize = false;
	bool m_isResizing = false;
	bool m_isFullscreen = false;
	bool m_AppPaused = false;
	bool m_Minimized = false;
	bool m_Maximized = false;

	// Set true to use 4X MSAA. Default is false.
	bool m4xMsaaState = false; 
	UINT m4xMsaaQuality = 0;

	// Derived class can customize these starting values
	wstring m_WindowCaption = L"DX12 Renderer";
	UINT m_Width = 2560;
	UINT m_Height = 1440;
	D3D_DRIVER_TYPE md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
	DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// Keep track of delta time and game time
	GameTimer m_Timer;

	//
	// D3D Internals
	//
	static constexpr UINT m_BufferCount = 2;
	UINT m_CurrentBufferIndex = 0;

	// Render target view descriptor heap
	ComPointer<ID3D12DescriptorHeap> m_rtvDescHeap;

	D3D12_CPU_DESCRIPTOR_HANDLE m_rtvHandles[m_BufferCount] = { 0 };

	ComPointer<ID3D12Device8> m_Device;
	ComPointer<IDXGIFactory7> m_DxgiFactory;
	ComPointer<IDXGISwapChain3> m_SwapChain;
	ComPointer<ID3D12Resource2> m_Buffers[m_BufferCount];

	ComPointer<ID3D12CommandQueue> m_CmdQueue;
	ComPointer<ID3D12CommandAllocator> m_CmdAllocator;
	ComPointer<ID3D12GraphicsCommandList6> m_CmdList;

	// A fence is basically kinda like a condition variable/wait/future/promise from concurrency/multithreading, it can wait for a signal (value) to proceed otherwise block
	// There's a shared fence flag which sounds like shared_future
	ComPointer<ID3D12Fence1> m_Fence;
	UINT64 m_FenceValue = 0;

	HANDLE m_FenceEvent = nullptr;
};
