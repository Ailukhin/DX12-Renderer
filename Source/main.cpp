#include <iostream>
#include "WinInclude.h"
#include "ComPointer.h"
#include "DXDebugLayer.h"
#include "DXContext.h"
#include "Window.h"

int main()
{
    DXDebugLayer::GetDXDebug().Init();

    if (DXContext::GetDXContext().Init() && DXWindow::GetDXWindow().Init())
    {
        // Set fullscreen
        DXWindow::GetDXWindow().SetFullScreen(true);

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

            // Draw
            auto* cmdList = DXContext::GetDXContext().InitCommandList();

            //

            // Finish drawing and present
            DXContext::GetDXContext().ExecuteCommandList();

            DXWindow::GetDXWindow().Present();
        }

        // Flush command queue
        DXContext::GetDXContext().FlushCommandQueue(DXWindow::GetDXWindow().GetBufferCount());

        DXWindow::GetDXWindow().Shutdown();
        DXContext::GetDXContext().Shutdown();
    }

    DXDebugLayer::GetDXDebug().Shutdown();
}

