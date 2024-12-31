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

	void PrintDeviceSupportLevel();

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

	// A fence is basically kinda like a condition variable/wait/future/promise from concurrency/multithreading, it can wait for a signal (value) to proceed otherwise it blocks
	// There's a shared fence flag which sounds like shared_future
	ComPointer<ID3D12Fence1> m_Fence;
	UINT64 m_FenceValue = 0;

};
