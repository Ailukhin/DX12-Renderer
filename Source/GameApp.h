#include "Window.h"
#include "DXContext.h"
#include "DXDebugLayer.h"

// Vertex data and layout
struct Vertex
{
    float x;
    float y;
};

class GameApp
{
public :
	GameApp();
	~GameApp();

	void Initialize();
	int Run();

private:
	void Resize();
	void Update(const GameTimer& timer);
	void Draw(const GameTimer& timer);

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
