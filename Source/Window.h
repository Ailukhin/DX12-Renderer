#include "WinInclude.h"
#include "ComPointer.h"

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

	inline bool GameExit() const
	{
		return m_GameExit;
	}

private:
	static LRESULT CALLBACK OnWindowMessage(HWND wnd, UINT msg, WPARAM wParam, LPARAM aParam);

private:
	ATOM m_WndClass = 0;
	HWND m_Window = nullptr;
	bool m_GameExit = false;
};
