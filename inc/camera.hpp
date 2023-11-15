#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <uEye.h>
#include <uEye_tools.h>

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
using json = nlohmann::json;

class Camera {
   public:
    Camera();
    ~Camera();
    void ConfigureCam(std::string path2Settings);
    void SetCaptureMode(int capMode);
    double getFPS();
    int getImageFormat();
    int getColorMode();
    void getAvailableFormats();

   private:
    void ReadJsonConfig(std::string path2Settings);
    bool checkConfigFile(std::string path2Settings, const std::string filetype);
    void ConfigureManualParams();
    void ConfigureAutoParams();

    void SetExposureTime(double expTime);
    void setBinning();
    void setRoi();

    void fetchCameraConfig();

    void PrintCamInfo();
    void PrintSensorInfo();
    void printCameraConfig();
    void PrintJsonConfig();
    void printError(std::string errDescription, std::string tabs);

    double effExposureTime = 0;
    unsigned int effPixelCock = 0;
    double effActualFPS = 0;
    double effFramerate = 0;
    int effOffset = 0;
    int effGain = 0;
    int effGamma = 0;
    int effShutterMode = 0;
    int enableRoi = 0;
    IS_RECT effRectAOI;
    int imageFormat = 0;
    int effColorMode = 0;
    double effFPS = 0;

    IMAGE_FORMAT_LIST *pformatList;
    void *ptr;

    json config;

    CAMINFO camInfo;
    SENSORINFO sensorInfo;
    int nRet;

    // Manually
    int colorMode = 0;
    double exposureTime = 0;
    unsigned int pixelCock = 0;
    double fps = 0;
    int offset = 0;
    int gain = 0;
    int gamma = 0;
    int binning = 0;
    int shutterMode = 0;
    IS_RECT rectAOI;

    // Auto
    double autoGain = 0;
    double autoShutter = 0;
    double autoWhiteBalance = 0;
    double autoFramerate = 0;

    typedef struct
    {
        unsigned int width;
        unsigned int height;
    } cameraFormat;

    std::map<int, cameraFormat> formatDefinition{
        {5, {2048, 1536}},
        {6, {1920, 1080}},
        {8, {1280, 960}},
        {9, {1280, 720}},
        {13, {640, 480}},
        {18, {320, 240}},
        {20, {1600, 1200}},
        {25, {1280, 1024}},
        {26, {2448, 2048}},
        {27, {1024, 768}},
        {28, {1024, 1024}},
        {29, {800, 600}},
        {35, {1920, 1200}},
        {36, {2456, 2054}},
        {40, {2048, 2048}}};

    // Defines
#define CONTINOUS_FREERUN 0
#define SINGLE_FREERUN 1
};

#endif
