/**
 * @Projectname:    Esave go!!!
 *
 * @Description:    main programm
 *
 * @IDE:            VSCode
 * @Compiler:       CL 14.35.32215
 * @openCV:         V4.5.4
 * @Platform        Windows 11 Pro
 *
 * @Creation date:  15.10.2023
 * @Creator:        Joshua Stutz
 */

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
#define ESAVE_GO_ROI_FORMAT 100

int dutyCycle = 0;
int dutyCycleTrackBar = 0;

static void onSlider(int, void *);

int main(int argc, char *argv[]) {
    std::string configPathDetectTrack = "../../prop/cameraSettingsTrackDetection.json";
    std::string configPath = "../../prop/cameraSettings.json";
    std::string saveImgPath = "../../out/singleImage.png";
    std::string maskPath = "../../res/trackMask.png";
    std::string trackerBarName = "Speed";
    std::string windowName;

    cv::Mat frame;
    cv::Mat greyFrame;
    cv::Mat binaryFrame;
    cv::Mat diffImg;
    cv::Mat binDiffImg;
    cv::Mat greyFrameMasked;
    cv::Mat lastFrame;

    cv::Rect boundingBox;
    cv::Point center;

    int dutyCycleLast = 0;

    try {
#if true
        SerialSTM32 serial;
        serial.init("COM5", CBR_115200);
        auto keyState1 = KeyState(VK_LBUTTON);
#endif

        auto cam = Camera();

        cv::Mat mask = detectTrack(hCam, cam, serial, configPathDetectTrack);

        auto mem = MemHandler(&hCam);
        mem.allocSingleBuffer();
        cam.SetCaptureMode(SINGLE_FREERUN);
        lastFrame = mem.getOpenCVMatSingleImg();

        cam.ConfigureCam(configPath);
        auto memHandler = MemHandler(&hCam);

#if true

        windowName = "IDS image";
        cv::namedWindow(windowName, cv::WINDOW_NORMAL);
        cv::resizeWindow(windowName, 300, 666);

        int maxDutyCycle = 250;
        cv::createTrackbar(trackerBarName, windowName, &dutyCycleTrackBar, maxDutyCycle, onSlider);

        memHandler.allocRingBuffer();
        cam.SetCaptureMode(CONTINOUS_FREERUN);

        while (true) {
            frame = memHandler.getOpenCVMatRingImg();

            // Do img processing stuff here:
            // -------------------------------------------------------------------

            /* Dont do this ->      cv::Mat frame1 = frame.clone();
                                    cv::medianBlur(frame, frame1, 9); */

            cv::cvtColor(frame, greyFrame, cv::COLOR_BGR2GRAY);
            greyFrame.copyTo(greyFrameMasked, mask);

            diffImg = greyFrameMasked - lastFrame;
            lastFrame = greyFrameMasked.clone();

            cv::threshold(diffImg, binDiffImg, 50, 255, cv::THRESH_BINARY);
            boundingBox = cv::boundingRect(binDiffImg);
            if (boundingBox.area() > 500 && boundingBox.area() < 50000) {
                // cv::rectangle(frame, boundingBox, cv::Scalar(0, 255, 0), 3);
                center = (boundingBox.br() + boundingBox.tl()) * 0.5;
                cv::circle(frame, center, 100, cv::Scalar(0, 255, 0), 3);
            }

            /* keyState1.KeyPressed();
            if (keyState1.KeyReleased())
            {
                cv::setTrackbarPos(trackerBarName, windowName, 0);
            } */

            if (keyState1.KeyPressed()) {
                dutyCycle = 15;
            }
            if (keyState1.KeyReleased()) {
                dutyCycle = 0;
            }

            if (dutyCycle != dutyCycleLast) {
                serial.WriteSerialPort(dutyCycle);
                dutyCycleLast = dutyCycle;
            }

            // -------------------------------------------------------------------

            /* memHandler.drawFpsOnImg(frame);
            memHandler.drawDroppedFramesOnImg(frame);
            memHandler.drawTimeStamp(frame); */

            cv::imshow(windowName, frame);
            // cv::imshow(windowName1, binaryFrame);

            char c = cv::pollKey();
            if (c == 27) {
                cv::destroyWindow(windowName);
                break;
            }
        }
#endif
    } catch (std::exception &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
    }
    return 0;
}

static void onSlider(int, void *) {
    dutyCycle = dutyCycleTrackBar;
}
