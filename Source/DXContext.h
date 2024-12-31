#include "WinInclude.h"
#include "ComPointer.h"

class DXContext
{
	// Singleton
public:
	DXContext(const DXContext&) = delete;
	DXContext& operator= (const DXContext&) = delete;

	inline static DXContext& GetDXContext()
	{
		static DXContext instance;
		return instance;
	}

private:
	DXContext() = default;

	// 
public:
	bool Init();
	void Shutdown();

	void SignalAndWait();

	ID3D12GraphicsCommandList6* InitCommandList();
	void ExecuteCommandList();

	void PrintDeviceSupportLevel();
	void PrintCommandListSupportLevel();

	inline ComPointer<ID3D12Device8>& GetDevice()
	{
		return m_Device;
	}

	inline ComPointer<ID3D12CommandQueue>& GetCommandQueue()
	{
		return m_CmdQueue;
	}

private:
	ComPointer<ID3D12Device8> m_Device;
	ComPointer<ID3D12CommandQueue> m_CmdQueue;

	ComPointer<ID3D12CommandAllocator> m_CmdAllocator;
	ComPointer<ID3D12GraphicsCommandList6> m_CmdList;

	// A fence is basically kinda like a condition variable/wait/future/promise from concurrency/multithreading, it can wait for a signal (value) to proceed otherwise block
	// There's a shared fence flag which sounds like shared_future
	ComPointer<ID3D12Fence1> m_Fence;
	UINT64 m_FenceValue = 0;

	HANDLE m_FenceEvent = nullptr;

};
