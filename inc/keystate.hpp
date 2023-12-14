#pragma once

#include <windows.h>

#include <iostream>
#include <string>

class KeyState {
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