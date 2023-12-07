#include <Windows.h>

#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <string>

#include "serialSTM32.hpp"

int dutyCycle;

static void onSlider(int, void *) {
}

int main() {
    SerialSTM32 serial;
    serial.init("COM5", CBR_115200);

    std::string windowName = "esave Go";
    std::string trackerBarName = "Speed";
    cv::namedWindow(windowName);

    cv::Mat src = cv::Mat::zeros(cv::Size(500, 100), CV_8U);

    int dutyMax = 255;
    int dutyLast = 0;
    cv::createTrackbar(trackerBarName, windowName, &dutyCycle, dutyMax, onSlider);
    cv::imshow(windowName, src);

    bool mouseLeftState = true;
    while (true) {
        if (GetAsyncKeyState(VK_LBUTTON) == -0x8000 && mouseLeftState) {
            mouseLeftState = false;
        }
        if (GetAsyncKeyState(VK_LBUTTON) == 0 && !mouseLeftState) {
            cv::setTrackbarPos(trackerBarName, windowName, 0);
            mouseLeftState = true;
        }
        // std::cout << GetAsyncKeyState(VK_XBUTTON2) << std::endl;
        if (dutyCycle != dutyLast) {
            serial.WriteSerialPort(dutyCycle);
            dutyLast = dutyCycle;
        }

        char c = cv::waitKey(10);
        if (c == 27) {
            break;
        }
    }
}
