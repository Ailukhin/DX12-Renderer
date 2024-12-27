#include <iostream>
#include "WinInclude.h"
#include "ComPointer.h"
#include "DXDebugLayer.h"

int main()
{
    DXDebugLayer::GetDXDebug().Init();



    DXDebugLayer::GetDXDebug().Shutdown();
}

