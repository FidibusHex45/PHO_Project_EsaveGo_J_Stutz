#include "keystate.hpp"

KeyState::KeyState(int virtualKey)
{
    keyState = true;
    vkey = virtualKey;
}

void KeyState::SetKey(int virtualKey)
{
    keyState = true;
    vkey = virtualKey;
}

bool KeyState::KeyPressed()
{
    if (GetAsyncKeyState(vkey) == -0x8000 && keyState)
    {
        keyState = false;
        return true;
    }
    return false;
}

bool KeyState::KeyReleased()
{
    if (GetAsyncKeyState(vkey) == 0 && !keyState)
    {
        keyState = true;
        return true;
    }
    return false;
}

bool KeyState::keyPressedToggle()
{
    if (GetAsyncKeyState(vkey) == 0 && !keyState)
    {
        keyState = true;
    }

    if (GetAsyncKeyState(vkey) == -0x8000 && keyState)
    {
        keyState = false;
        return true;
    }
    return false;
}
