#include <iostream>
#include <string>
#include <windows.h>

class KeyState
{
public:
    KeyState(int virtualKey);
    void SetKey(int virtualKey);
    bool KeyPressed();
    bool KeyReleased();
    bool keyPressedToggle();

private:
    int vkey;
    bool keyState;
};