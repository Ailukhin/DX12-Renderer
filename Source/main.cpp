#include <iostream>
#include "WinInclude.h"
#include "ComPointer.h"

int main()
{
    std::cout << "Hello World!\n";

    POINT pt;
    GetCursorPos(&pt);

    std::cout << "The cursor is x: " << pt.x << " y: " << pt.y;
}

