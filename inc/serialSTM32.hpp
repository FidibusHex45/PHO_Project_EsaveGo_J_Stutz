#ifndef SERIALSTM32_HPP
#define SERIALSTM32_HPP

#include <iostream>
#include <string>
#include <windows.h>

class SerialSTM32
{
public:
    void init(char *com_port, DWORD baud);
    void WriteSerialPort(int msg);
    bool CloseSerialPort();
    ~SerialSTM32();
    bool connected_;

private:
    HANDLE h_Serial;
    DCB dcbSerialParam;
};

#endif