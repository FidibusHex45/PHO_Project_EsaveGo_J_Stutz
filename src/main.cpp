/* main */

#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <stdexcept>
#include <string>

#include "camera.hpp"
#include "car_controller.hpp"
#include "detectTrack.hpp"
#include "keystate.hpp"
#include "mem_handler.hpp"
#include "serialSTM32.hpp"

HIDS hCam;

int main(int argc, char *argv[]) {
    std::string configPathDetectTrack = "../../prop/cameraSettingsTrackDetection.json";
    std::string configPathCarController = "../../prop/cameraSettings.json";
    std::string save_path = "../../data/datapoints.csv";
    std::string load_path = "../../data/splineData.csv";
    trackData track_data;

    try {
        auto ser = SerialSTM32("COM5", CBR_115200);
        auto cam = Camera();
        cam.ConfigureCam(configPathDetectTrack);
        auto mem = MemHandler(&hCam, 60);
        auto tdet = Trackdetector(&mem, &cam, &ser, configPathDetectTrack);
        track_data = tdet.detectTrack(load_path, save_path);
        auto contr = CarController(&mem, &cam, &ser, track_data, configPathCarController);
        contr.run();
    } catch (std::exception &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
    }
    return 0;
}