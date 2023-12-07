#include "serialSTM32.hpp"

void SerialSTM32::init(char* com_port, DWORD baud) {
    connected_ = false;
    h_Serial = CreateFile(static_cast<LPCSTR>(com_port),
                          GENERIC_READ | GENERIC_WRITE,
                          0,
                          NULL,
                          OPEN_EXISTING,
                          FILE_ATTRIBUTE_NORMAL,
                          NULL);

    if (h_Serial == INVALID_HANDLE_VALUE) {
        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            std::cout << "Serial Port not found." << std::endl;
        }
        std::cout << "Some error occured." << std::endl;
    }

    dcbSerialParam = {0};
    dcbSerialParam.DCBlength = sizeof(dcbSerialParam);

    if (!GetCommState(h_Serial, &dcbSerialParam)) {
        throw std::runtime_error("Com port invalid!");
    }

    dcbSerialParam.BaudRate = baud;
    dcbSerialParam.ByteSize = 8;
    dcbSerialParam.StopBits = ONESTOPBIT;
    dcbSerialParam.Parity = NOPARITY;

    if (!SetCommState(h_Serial, &dcbSerialParam)) {
        throw std::runtime_error("Com port configuration incomplete!");
    } else {
        connected_ = true;
    }
}

void SerialSTM32::WriteSerialPort(int msg) {
    auto data = std::to_string(msg);
    while (data.length() < 3) {
        data.insert(0, "0");
    }
    char sBuff[4] = {};
    strcpy(sBuff, data.c_str());

    DWORD dwRead = 0;
    bool error = WriteFile(h_Serial, sBuff, sizeof(sBuff), &dwRead, NULL);

    if (error) {
        // Success du Tubel!!
    }

    else {
        std::cout << "Could not write Port." << std::endl;
    }
}

bool SerialSTM32::CloseSerialPort() {
    if (connected_) {
        connected_ = false;
        CloseHandle(h_Serial);
        return true;
    } else {
        return false;
    }
}

SerialSTM32::~SerialSTM32() {
    WriteSerialPort(0);
    if (connected_) {
        connected_ = false;
        CloseHandle(h_Serial);
    }
}
