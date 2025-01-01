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
        while (!DXWindow::GetDXWindow().GameExit())
        {
            // Poll for window update
            DXWindow::GetDXWindow().Update();

            auto* cmdList = DXContext::GetDXContext().InitCommandList();

            DXContext::GetDXContext().ExecuteCommandList();
        }

        DXWindow::GetDXWindow().Shutdown();
        DXContext::GetDXContext().Shutdown();
    }

    DXDebugLayer::GetDXDebug().Shutdown();
}

