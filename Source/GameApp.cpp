#include "GameApp.h"
#include "ShaderObject.h"

void MixColor(float* color)
{
    static int colorState = 0;
    color[colorState] += 0.01f;
    if (color[colorState] > 1.0f)
    {
        colorState++;
        if (colorState == 3)
        {
            color[0] = 0.0f;
            color[1] = 0.0f;
            color[2] = 0.0f;
            colorState = 0;
        }
    }
}

GameApp::GameApp()
{
    
}

GameApp::~GameApp()
{
    
}

void GameApp::Initialize()
{
    DXDebugLayer::GetDXDebug().Init();
    DXContext::GetDXContext().Init();
    DXWindow::GetDXWindow().Init();

    // Set fullscreen
    //DXWindow::GetDXWindow().SetFullScreen(true);

    D3D12_HEAP_PROPERTIES hpUpload{};
    hpUpload.Type = D3D12_HEAP_TYPE_UPLOAD;
    hpUpload.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    hpUpload.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    hpUpload.CreationNodeMask = 0;
    hpUpload.VisibleNodeMask = 0;

    D3D12_HEAP_PROPERTIES hpDefault{};
    hpDefault.Type = D3D12_HEAP_TYPE_DEFAULT;
    hpDefault.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    hpDefault.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    hpDefault.CreationNodeMask = 0;
    hpDefault.VisibleNodeMask = 0;

    D3D12_INPUT_ELEMENT_DESC vertexLayout[] =
    {
        { "Position", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    // Upload and Vertex Buffers
    D3D12_RESOURCE_DESC rd{};
    rd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    rd.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    rd.Width = 1024;
    rd.Height = 1;
    rd.DepthOrArraySize = 1;
    rd.MipLevels = 1;
    rd.Format = DXGI_FORMAT_UNKNOWN;
    rd.SampleDesc.Count = 1;
    rd.SampleDesc.Quality = 0;
    rd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    rd.Flags = D3D12_RESOURCE_FLAG_NONE;

    DXContext::GetDXContext().GetDevice()->CreateCommittedResource(&hpUpload, D3D12_HEAP_FLAG_NONE, &rd, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&upBuffer));
    DXContext::GetDXContext().GetDevice()->CreateCommittedResource(&hpDefault, D3D12_HEAP_FLAG_NONE, &rd, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&vertBuffer));

    // Copy void pointer to CPU Resource
    void* upBufferAddress;
    D3D12_RANGE uploadRange;
    uploadRange.Begin = 0;
    uploadRange.End = 1023;
    upBuffer->Map(0, &uploadRange, &upBufferAddress);
    memcpy(upBufferAddress, verts, sizeof(verts));
    upBuffer->Unmap(0, &uploadRange);

    // Copy CPU Resource to GPU Resource
    cmdList = DXContext::GetDXContext().InitCommandList();
    cmdList->CopyBufferRegion(vertBuffer, 0, upBuffer, 0, 1024);
    DXContext::GetDXContext().ExecuteCommandList();

    // Shaders
    ShaderObject rootSignatureShader("RootSignature.cso");
    ShaderObject vertexShader("VertexShader.cso");
    ShaderObject pixelShader("PixelShader.cso");

    // Root signature object
    DXContext::GetDXContext().GetDevice()->CreateRootSignature(0, rootSignatureShader.GetBuffer(), rootSignatureShader.GetSize(), IID_PPV_ARGS(&rootSignature));

    // Pipeline state
    D3D12_GRAPHICS_PIPELINE_STATE_DESC gfxPsoDesc;
    gfxPsoDesc.pRootSignature = rootSignature;
    gfxPsoDesc.InputLayout.NumElements = _countof(vertexLayout);
    gfxPsoDesc.InputLayout.pInputElementDescs = vertexLayout;
    gfxPsoDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
    gfxPsoDesc.VS.pShaderBytecode = vertexShader.GetBuffer();
    gfxPsoDesc.VS.BytecodeLength = vertexShader.GetSize();
    gfxPsoDesc.PS.pShaderBytecode = pixelShader.GetBuffer();
    gfxPsoDesc.PS.BytecodeLength = pixelShader.GetSize();
    gfxPsoDesc.GS.BytecodeLength = 0; // Geo shader
    gfxPsoDesc.GS.pShaderBytecode = nullptr;
    gfxPsoDesc.DS.BytecodeLength = 0; // Domain shader
    gfxPsoDesc.DS.pShaderBytecode = nullptr;
    gfxPsoDesc.HS.BytecodeLength = 0; // Hull shader
    gfxPsoDesc.HS.pShaderBytecode = nullptr;
    gfxPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    gfxPsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    gfxPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    gfxPsoDesc.RasterizerState.FrontCounterClockwise = false;
    gfxPsoDesc.RasterizerState.DepthBias = 0;
    gfxPsoDesc.RasterizerState.DepthBiasClamp = 0.0f;
    gfxPsoDesc.RasterizerState.SlopeScaledDepthBias = 0.0f;
    gfxPsoDesc.RasterizerState.DepthClipEnable = false;
    gfxPsoDesc.RasterizerState.MultisampleEnable = false;
    gfxPsoDesc.RasterizerState.AntialiasedLineEnable = false;
    gfxPsoDesc.RasterizerState.ForcedSampleCount = 0;
    gfxPsoDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
    gfxPsoDesc.StreamOutput.NumEntries = 0;
    gfxPsoDesc.StreamOutput.NumStrides = 0;
    gfxPsoDesc.StreamOutput.pBufferStrides = nullptr;
    gfxPsoDesc.StreamOutput.pSODeclaration = nullptr;
    gfxPsoDesc.StreamOutput.RasterizedStream = 0;
    gfxPsoDesc.NumRenderTargets = 1;
    gfxPsoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    gfxPsoDesc.RTVFormats[1] = DXGI_FORMAT_UNKNOWN;
    gfxPsoDesc.RTVFormats[2] = DXGI_FORMAT_UNKNOWN;
    gfxPsoDesc.RTVFormats[3] = DXGI_FORMAT_UNKNOWN;
    gfxPsoDesc.RTVFormats[4] = DXGI_FORMAT_UNKNOWN;
    gfxPsoDesc.RTVFormats[5] = DXGI_FORMAT_UNKNOWN;
    gfxPsoDesc.RTVFormats[6] = DXGI_FORMAT_UNKNOWN;
    gfxPsoDesc.RTVFormats[7] = DXGI_FORMAT_UNKNOWN;
    gfxPsoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
    gfxPsoDesc.BlendState.AlphaToCoverageEnable = false;
    gfxPsoDesc.BlendState.IndependentBlendEnable = false;
    gfxPsoDesc.BlendState.RenderTarget[0].BlendEnable = true;
    gfxPsoDesc.BlendState.RenderTarget[0].LogicOpEnable = false;
    gfxPsoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
    gfxPsoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
    gfxPsoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    gfxPsoDesc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
    gfxPsoDesc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
    gfxPsoDesc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    gfxPsoDesc.BlendState.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
    gfxPsoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    gfxPsoDesc.DepthStencilState.DepthEnable = false;
    gfxPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    gfxPsoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    gfxPsoDesc.DepthStencilState.StencilEnable = false;
    gfxPsoDesc.DepthStencilState.StencilReadMask = 0;
    gfxPsoDesc.DepthStencilState.StencilWriteMask = 0;
    gfxPsoDesc.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    gfxPsoDesc.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    gfxPsoDesc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    gfxPsoDesc.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    gfxPsoDesc.DepthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    gfxPsoDesc.DepthStencilState.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    gfxPsoDesc.DepthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    gfxPsoDesc.DepthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    gfxPsoDesc.SampleMask = 0xFFFFFFFF;
    gfxPsoDesc.SampleDesc.Count = 1;
    gfxPsoDesc.SampleDesc.Quality = 0;
    gfxPsoDesc.NodeMask = 0;
    gfxPsoDesc.CachedPSO.CachedBlobSizeInBytes = 0;
    gfxPsoDesc.CachedPSO.pCachedBlob = nullptr;
    gfxPsoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    DXContext::GetDXContext().GetDevice()->CreateGraphicsPipelineState(&gfxPsoDesc, IID_PPV_ARGS(&pso));

    // Vertex buffer view
    vbv.BufferLocation = vertBuffer->GetGPUVirtualAddress();
    vbv.SizeInBytes = sizeof(Vertex) * _countof(verts); // or just sizeof(verts)
    vbv.StrideInBytes = sizeof(Vertex);
}

int GameApp::Run()
{
    MSG msg = { 0 };

    DXWindow::GetDXWindow().mTimer.Reset();

    while (!DXWindow::GetDXWindow().GameExit())
    {
        // Poll for a window message in the event queue
        // pass in message pointer, the window instance, some filter min/max figure out what this is later, 
        // PM_REMOVE removes the message from the event queue after being processed
        if (PeekMessageW(&msg, DXWindow::GetDXWindow().m_Window, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);

            // Call the WNDPROC to update the window
            DispatchMessageW(&msg);
        }
        else
        {
            DXWindow::GetDXWindow().mTimer.Tick();

            DXWindow::GetDXWindow().CalculateFrameStats();
            Update(DXWindow::GetDXWindow().mTimer);
            Draw(DXWindow::GetDXWindow().mTimer);
        }
    }

    // Cleanup

    // Flush command queue
    DXContext::GetDXContext().FlushCommandQueue(DXWindow::GetDXWindow().GetBufferCount());

    vertBuffer.Release();
    upBuffer.Release();
    rootSignature.Release();
    pso.Release();
    cmdList = nullptr;

    DXWindow::GetDXWindow().Shutdown();
    DXContext::GetDXContext().Shutdown();

    DXDebugLayer::GetDXDebug().Shutdown();

    return (int)msg.wParam;
}

void GameApp::Resize()
{

}

void GameApp::Update(const GameTimer& timer)
{

}

void GameApp::Draw(const GameTimer& timer)
{
    // Check for resize after window update
    if (DXWindow::GetDXWindow().ShouldResize())
    {
        // Command queue must be flushed before resize
        DXContext::GetDXContext().FlushCommandQueue(DXWindow::GetDXWindow().GetBufferCount());

        DXWindow::GetDXWindow().Resize();
    }

    // Prepare the command list for drawing
    cmdList = DXContext::GetDXContext().InitCommandList();

    // Begin drawing frame
    DXWindow::GetDXWindow().BeginFrame(cmdList);

    // Pipeline state
    cmdList->SetPipelineState(pso);
    cmdList->SetGraphicsRootSignature(rootSignature);

    // Input assembly
    cmdList->IASetVertexBuffers(0, 1, &vbv);
    cmdList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Raster
    D3D12_VIEWPORT vp;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    vp.Width = DXWindow::GetDXWindow().GetWindowWidth();
    vp.Height = DXWindow::GetDXWindow().GetWindowHeight();
    vp.MinDepth = 1.0f;
    vp.MaxDepth = 0.0f;
    cmdList->RSSetViewports(1, &vp);

    RECT scRect;
    scRect.left = 0;
    scRect.top = 0;
    scRect.right = DXWindow::GetDXWindow().GetWindowWidth();
    scRect.bottom = DXWindow::GetDXWindow().GetWindowHeight();
    cmdList->RSSetScissorRects(1, &scRect);

    // Root arguments
    static float color[] = { 0.0f, 0.0f, 0.0f };
    MixColor(color);
    cmdList->SetGraphicsRoot32BitConstants(0, 3, color, 0);

    // Draw
    cmdList->DrawInstanced(_countof(verts), 1, 0, 0);

    // End drawing frame
    DXWindow::GetDXWindow().EndFrame(cmdList);

    // Finish drawing and present
    DXContext::GetDXContext().ExecuteCommandList();

    DXWindow::GetDXWindow().Present();
}
