#include "Window.h"
#include "DXDebugLayer.h"

// Vertex data and layout
struct Vertex
{
    float x;
    float y;
};

class GameApp : public DXWindow
{
public :
	GameApp();
	~GameApp();

	bool Init() override;

private:
	virtual void Update(const GameTimer& timer) override;
	virtual void Draw(const GameTimer& timer) override;
	virtual void ResizeBuffers() override;
	virtual void Shutdown() override;

private:
	ComPointer<ID3D12PipelineState> pso;
	D3D12_VERTEX_BUFFER_VIEW vbv{};
	ComPointer<ID3D12Resource2> upBuffer, vertBuffer;
	ComPointer<ID3D12RootSignature> rootSignature;
	ID3D12GraphicsCommandList6* cmdList;

	Vertex verts[3] =
	{
		{ -1.0f, -1.0f },
		{  0.0f,  1.0f },
		{  1.0f, -1.0f}
	};
};
