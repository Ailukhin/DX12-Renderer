#include "DXContext.h"
#include <stdio.h>

bool DXContext::Init()
{
	if (SUCCEEDED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_Device))))
	{
		//m_Device->CheckFeatureSupport(---);

		//PrintDeviceSupportLevel();

		D3D12_COMMAND_QUEUE_DESC cmdQueueDesc{};
		cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // Direct is general purpose probably not best perf everywhere, use more specific types later on
		cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH; // Normal is for normal apps, High is for games, Global real-time not sure what this is
		cmdQueueDesc.NodeMask = 0; // 0 is default setting, node mask is basically what gpu is being used
		cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE; // None and gpu timeout, gpu timeout cancels work after some amount of time where no tasks are completed?
		
		if (SUCCEEDED(m_Device->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&m_CmdQueue))))
		{
			// A fence does not need a descriptor when created
			if (SUCCEEDED(m_Device->CreateFence(m_FenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence))))
			{
				return true;
			}
		}
	}

	return false;
}

void DXContext::Shutdown()
{
	m_Fence.Release();
	m_CmdQueue.Release();
	m_Device.Release();
}

void DXContext::PrintDeviceSupportLevel()
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

	//HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	//if (FAILED(hr)) {
	//    std::cerr << "Failed to initialize COM library. Error: " << hr << std::endl;
	//    return -1;
	//}

	//Microsoft::WRL::ComPtr<IDXGIFactory4> factory;
	//hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
	//if (FAILED(hr)) {
	//    std::cerr << "Failed to create DXGI Factory. Error: " << hr << std::endl;
	//    return -1;
	//}

	//// Get the first hardware adapter
	//Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
	//for (UINT i = 0; DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(i, &adapter); ++i) {
	//    DXGI_ADAPTER_DESC1 desc;
	//    adapter->GetDesc1(&desc);

	//    // Skip software adapters
	//    if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
	//        continue;
	//    }

	//    // Print adapter info
	//    std::wcout << L"Using Adapter: " << desc.Description << std::endl;
	//    break;
	//}

	//if (!adapter) {
	//    std::cerr << "No suitable adapter found." << std::endl;
	//    return -1;
	//}

	//// Create a D3D12 device
	//Microsoft::WRL::ComPtr<ID3D12Device> device;
	//hr = D3D12CreateDevice(
	//    adapter.Get(),
	//    D3D_FEATURE_LEVEL_11_0,
	//    IID_PPV_ARGS(&device)
	//);

	//if (FAILED(hr)) {
	//    std::cerr << "Failed to create D3D12 device. Error: " << hr << std::endl;
	//    return -1;
	//}

	//Microsoft::WRL::ComPtr<ID3D12Device10> dev10;
	//if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&dev10))))
	//{
	//    printf("ID3D12 Device 10 is supported.\n");
	//}
	//else
	//{
	//    printf("ID3D12 Device 10 is NOT supported.\n");
	//}
}




