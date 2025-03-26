#include <iostream>
#include "WinInclude.h"
#include "ComPointer.h"
#include "DXDebugLayer.h"
#include "DXContext.h"
#include "Window.h"
#include "ShaderObject.h"

int main()
{
    DXDebugLayer::GetDXDebug().Init();

    if (DXContext::GetDXContext().Init() && DXWindow::GetDXWindow().Init())
    {
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

        // Vertex data and layout
        struct Vertex
        {
            float x;
            float y;
        };

        Vertex verts[] = 
        {
            { -1.0f, -1.0f },
            {  0.0f,  1.0f },
            {  1.0f, -1.0f}
        };

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

        ComPointer<ID3D12Resource2> upBuffer, vertBuffer;
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
        auto* cmdList = DXContext::GetDXContext().InitCommandList();
        cmdList->CopyBufferRegion(vertBuffer, 0, upBuffer, 0, 1024);
        DXContext::GetDXContext().ExecuteCommandList();

        // Shaders
        ShaderObject rootSignatureShader("RootSignature.cso");
        ShaderObject vertexShader("VertexShader.cso");
        ShaderObject pixelShader("PixelShader.cso");

        // Root signature object
        ComPointer<ID3D12RootSignature> rootSignature;
        DXContext::GetDXContext().GetDevice()->CreateRootSignature(0, rootSignatureShader.GetBuffer(), rootSignatureShader.GetSize(), IID_PPV_ARGS(&rootSignature));

        // Pipeline state
        D3D12_GRAPHICS_PIPELINE_STATE_DESC gfxPsoDesc;
        gfxPsoDesc.pRootSignature = rootSignature;
        gfxPsoDesc.InputLayout.NumElements = _countof(vertexLayout);
        gfxPsoDesc.InputLayout.pInputElementDescs = vertexLayout;
        gfxPsoDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
        gfxPsoDesc.VS.pShaderBytecode = vertexShader.GetBuffer();
        gfxPsoDesc.VS.BytecodeLength = vertexShader.GetSize();
        gfxPsoDesc.PS.pShaderBytecode = vertexShader.GetBuffer();
        gfxPsoDesc.PS.BytecodeLength = vertexShader.GetSize();
        gfxPsoDesc.GS.BytecodeLength = 0; // Geo shader
        gfxPsoDesc.GS.pShaderBytecode = nullptr;
        gfxPsoDesc.DS.BytecodeLength = 0; // Domain shader
        gfxPsoDesc.DS.pShaderBytecode = nullptr;
        gfxPsoDesc.HS.BytecodeLength = 0; // Hull shader
        gfxPsoDesc.HS.pShaderBytecode = nullptr;
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
        gfxPsoDesc.StreamOutput.NumEntries = 0;
        gfxPsoDesc.StreamOutput.NumStrides = 0;
        gfxPsoDesc.StreamOutput.pBufferStrides = nullptr;
        gfxPsoDesc.StreamOutput.pSODeclaration = nullptr;
        gfxPsoDesc.StreamOutput.RasterizedStream = 0;
        gfxPsoDesc.NodeMask = 0;
        gfxPsoDesc.CachedPSO.CachedBlobSizeInBytes = 0;
        gfxPsoDesc.CachedPSO.pCachedBlob = nullptr;
        gfxPsoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

        // Vertex buffer view
        D3D12_VERTEX_BUFFER_VIEW vbv{};
        vbv.BufferLocation = vertBuffer->GetGPUVirtualAddress();
        vbv.SizeInBytes = sizeof(Vertex) * _countof(verts); // or just sizeof(verts)
        vbv.StrideInBytes = sizeof(Vertex);

        while (!DXWindow::GetDXWindow().GameExit())
        {
            // Poll for window update
            DXWindow::GetDXWindow().Update();

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

            // Input assembly
            cmdList->IASetVertexBuffers(0, 1, &vbv);
            cmdList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            // Draw
            cmdList->DrawInstanced(_countof(verts), 1, 0, 0);

            // End drawing frame
            DXWindow::GetDXWindow().EndFrame(cmdList);

            // Finish drawing and present
            DXContext::GetDXContext().ExecuteCommandList();

            DXWindow::GetDXWindow().Present();
        }

        // Cleanup

        // Flush command queue
        DXContext::GetDXContext().FlushCommandQueue(DXWindow::GetDXWindow().GetBufferCount());

        vertBuffer.Release();
        upBuffer.Release();

        DXWindow::GetDXWindow().Shutdown();
        DXContext::GetDXContext().Shutdown();
    }

    DXDebugLayer::GetDXDebug().Shutdown();
}

