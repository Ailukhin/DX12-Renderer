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

	inline bool GameExit() const
	{
		return m_GameExit;
	}

	inline UINT GetBufferCount() const
	{
		return bufferCount;
	}

private:
	static LRESULT CALLBACK OnWindowMessage(HWND wnd, UINT msg, WPARAM wParam, LPARAM aParam);

private:
	ATOM m_WndClass = 0;
	HWND m_Window = nullptr;
	bool m_GameExit = false;
	UINT bufferCount = 0;

	ComPointer<IDXGISwapChain3> m_SwapChain;
};
