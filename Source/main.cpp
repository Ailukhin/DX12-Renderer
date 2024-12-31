#include <iostream>
#include "WinInclude.h"
#include "ComPointer.h"
#include "DXDebugLayer.h"
#include "DXContext.h"
#include "wrl.h"

int main()
{
    DXDebugLayer::GetDXDebug().Init();

    if (DXContext::GetDXContext().Init())
    {
        while (true)
        {
            auto* cmdList = DXContext::GetDXContext().InitCommandList();

            DXContext::GetDXContext().ExecuteCommandList();
        }

        DXContext::GetDXContext().Shutdown();
    }

    DXDebugLayer::GetDXDebug().Shutdown();
}

