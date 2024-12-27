#include "DXDebugLayer.h"

bool DXDebugLayer::Init()
{
#ifdef _DEBUG
    // Create D3D12 Debug layer
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_d3d12Debug))))
    {
        m_d3d12Debug->EnableDebugLayer();

        // Init DXGI Debug
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&m_dxgiDebug))))
        {
            m_dxgiDebug->EnableLeakTrackingForThread();
            return true;
        }
    }
#endif

    return false;
}

void DXDebugLayer::Shutdown()
{
#ifdef _DEBUG
    // Check for leaks, coms that haven't been cleaned up
    if (m_dxgiDebug)
    {
        OutputDebugString(L"DXGI Report for living device objects:\n");

        // Detailed info but also ignore internal information // DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL)
        m_dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
    }

    m_dxgiDebug.Release();
    m_d3d12Debug.Release();
#endif
}
