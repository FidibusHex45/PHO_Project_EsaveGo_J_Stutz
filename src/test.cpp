/* Testing only */

#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <stdexcept>
#include <string>

#include "camera.hpp"
#include "detectTrack.hpp"
#include "keystate.hpp"
#include "mem_handler.hpp"
#include "serialSTM32.hpp"

HIDS hCam;

int main(int argc, char *argv[]) {
    std::string configPathDetectTrack = "../../prop/cameraSettingsTrackDetection.json";
    std::vector<splineData> spline_data;
    cv::Mat img;
    std::string windowName = "mask";

    try {
        /* auto cam = Camera();
        cam.ConfigureCam(configPathDetectTrack);

        auto mem = MemHandler(&hCam);
        mem.allocSingleBuffer();
        cam.SetCaptureMode(SINGLE_FREERUN);
        img = mem.getOpenCVMatSingleImg();

        executePyScript();
        spline_data = getSplineData();
        drawSpline(img, spline_data);

        cv::namedWindow(windowName, cv::WINDOW_NORMAL);
        cv::resizeWindow(windowName, 300, 666);
        cv::imshow(windowName, img);
        cv::waitKey(0); */

        SerialSTM32 serial;
        serial.init("COM5", CBR_115200);

        auto cam = Camera();

        cv::Mat mask = detectTrack(hCam, cam, serial, configPathDetectTrack);

    } catch (std::exception &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
    }
    return 0;
}