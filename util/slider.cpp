#include <iostream>
#include <string>
#include <Windows.h>

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

int dutyCycle;

static void onSlider(int, void *)
{
}

int main()
{
    std::string filepath = "../../data/carreraGo.jpg";
    std::string windowName = "esave Go";
    std::string trackerBarName = "Speed";
    cv::namedWindow(windowName);

    cv::Mat src = cv::imread(filepath, cv::IMREAD_COLOR);

    int dutyMax = 2000;
    cv::createTrackbar(trackerBarName, windowName, &dutyCycle, dutyMax, onSlider);
    cv::imshow(windowName, src);

    bool mouseLeftState = true;
    while (true)
    {

        if (GetAsyncKeyState(VK_LBUTTON) == -0x8000 && mouseLeftState)
        {
            mouseLeftState = false;
        }
        if (GetAsyncKeyState(VK_LBUTTON) == 0 && !mouseLeftState)
        {
            cv::setTrackbarPos(trackerBarName, windowName, 0);
            mouseLeftState = true;
        }
        // std::cout << GetAsyncKeyState(VK_XBUTTON2) << std::endl;

        char c = cv::waitKey(10);
        if (c == 27)
        {
            break;
        }
    }
}
