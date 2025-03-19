#include "DXContext.h"
#include <stdio.h>
#include <cassert>
#include <cstdlib>
#include "Window.h"
#include <dxgi1_3.h>

bool DXContext::Init()
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

void DXContext::Shutdown()
{
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

void DXContext::SignalAndWait()
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

ID3D12GraphicsCommandList6* DXContext::InitCommandList()
{
	// Reset allocator and command list
	m_CmdAllocator->Reset();
	m_CmdList->Reset(m_CmdAllocator, nullptr);

	return m_CmdList;
}

void DXContext::ExecuteCommandList()
{
	// Need to close the list before execution
	if (SUCCEEDED(m_CmdList->Close()))
	{
		ID3D12CommandList* lists[] = { m_CmdList };
		m_CmdQueue->ExecuteCommandLists(1, lists);

		SignalAndWait();
	}
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
}

void DXContext::PrintCommandListSupportLevel()
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




