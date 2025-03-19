#include "WinInclude.h"
#include "ComPointer.h"
#include <cassert>

class DXWindow
{
	// Singleton
public:
	DXWindow(const DXWindow&) = delete;
	DXWindow& operator= (const DXWindow&) = delete;

	inline static DXWindow& GetDXWindow()
	{
		static DXWindow instance;
		return instance;
	}

private:
	DXWindow() = default;

	// 
public:
	bool Init();
	void Shutdown();
	void Update();
	void Present();
	void Resize();
	void SetFullScreen(bool enable);

	void BeginFrame(ID3D12GraphicsCommandList6* cmdList);
	void EndFrame(ID3D12GraphicsCommandList6* cmdList);

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

private:
	static LRESULT CALLBACK OnWindowMessage(HWND wnd, UINT msg, WPARAM wParam, LPARAM aParam);

	bool GetBuffers();
	void ReleaseBuffers();

private:
	ATOM m_WndClass = 0;
	HWND m_Window = nullptr;
	bool m_GameExit = false;

	static constexpr UINT m_BufferCount = 2;

	UINT m_Width = 2560;
	UINT m_Height = 1440;
	bool m_Resize = false;
	bool m_isFullscreen = false;

	ComPointer<IDXGISwapChain3> m_SwapChain;
	ComPointer<ID3D12Resource2> m_Buffers[m_BufferCount];

	UINT m_CurrentBufferIndex = 0;

	// Render target view descriptor heap
	ComPointer<ID3D12DescriptorHeap> m_rtvDescHeap;

	D3D12_CPU_DESCRIPTOR_HANDLE m_rtvHandles[m_BufferCount];
};
