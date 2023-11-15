#include <iostream>
#include <conio.h>
#include <windows.h>

int main()
{
    bool arrowUpState = true;
    bool arrowDownState = true;

    unsigned int pulseWidth = 0;

    while (true)
    {

        if (GetAsyncKeyState(VK_UP) == -0x8000 && arrowUpState)
        {
            arrowUpState = false;
            pulseWidth++;
            std::cout << "Pulse width: " << pulseWidth << std::endl;
            // std::cout << "Up arrow pressed." << std::endl;
        }
        if (GetAsyncKeyState(VK_UP) == 0 && !arrowUpState)
        {
            arrowUpState = true;
        }

        if (GetAsyncKeyState(VK_DOWN) == -0x8000 && arrowDownState)
        {
            arrowDownState = false;
            pulseWidth--;
            std::cout << "Pulse width: " << pulseWidth << std::endl;
            // std::cout << "Down arrow pressed." << std::endl;
        }
        if (GetAsyncKeyState(VK_DOWN) == 0 && !arrowDownState)
        {
            arrowDownState = true;
        }

        if (GetAsyncKeyState(VK_ESCAPE))
        {
            break;
        }
    }

    return 0;
}