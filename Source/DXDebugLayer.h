#include "D3D_UTIL.h"
#include "ComPointer.h"

class DXDebugLayer
{
// Singleton
public:
	DXDebugLayer(const DXDebugLayer&) = delete;
	DXDebugLayer& operator= (const DXDebugLayer&) = delete;

	inline static DXDebugLayer& GetDXDebug()
	{
		static DXDebugLayer instance;
		return instance;
	}

private:
	DXDebugLayer() = default;

// 
public:
	bool Init();
	void Shutdown();

private:
#ifdef _DEBUG
	ComPointer<ID3D12Debug3> m_d3d12Debug;
	ComPointer<IDXGIDebug1> m_dxgiDebug;
#endif

};
