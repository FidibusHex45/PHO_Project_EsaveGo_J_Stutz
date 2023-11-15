#include "camera.hpp"

using json = nlohmann::json;
extern HIDS hCam;

Camera::Camera() {
    if (is_InitCamera(&hCam, NULL) != IS_SUCCESS) {
        throw std::runtime_error("Connecting to camera failed!");
    }
    nRet = is_GetCameraInfo(hCam, &camInfo);
    nRet = is_GetSensorInfo(hCam, &sensorInfo);
    std::cout << "Connecting to camera successfull!" << std::endl;
    PrintCamInfo();
    PrintSensorInfo();
}

Camera::~Camera() {
    is_StopLiveVideo(hCam, IS_WAIT);
    free(ptr);
}

void Camera::ConfigureCam(std::string path2Settings) {
    if (checkConfigFile(path2Settings, ".json")) {
        std::cout << "Configuring with json config file." << std::endl;
        ReadJsonConfig(path2Settings);
        PrintJsonConfig();
        ConfigureManualParams();
        ConfigureAutoParams();
    } else if (checkConfigFile(path2Settings, ".ini")) {
        throw std::runtime_error("Configuration with ini file not available in demo version!");
        std::cout << std::endl
                  << "Configuring with ini config file." << std::endl;
        std::wstring w_path2Settings = std::wstring(path2Settings.begin(), path2Settings.end());
        const wchar_t *wConst_path2Settings = w_path2Settings.c_str();
        nRet = is_ParameterSet(hCam, IS_PARAMETERSET_CMD_LOAD_FILE, (void *)wConst_path2Settings, NULL);
        printError("Status configuration with ini config file: ", "\t");
    } else {
        throw std::runtime_error("Specified file not found, pleas check filepath!");
    }

    fetchCameraConfig();
    printCameraConfig();
}

void Camera::SetCaptureMode(int capMode) {
    if (capMode == CONTINOUS_FREERUN) {
        nRet = is_CaptureVideo(hCam, IS_WAIT);
        std::cout << "Continous freerun capture mode set: " << nRet << std::endl;
    } else if (capMode == SINGLE_FREERUN) {
        nRet = is_FreezeVideo(hCam, IS_WAIT);
        std::cout << "Single frame capture mode set: " << nRet << std::endl;
    } else {
        throw std::runtime_error("Unknown capture mode!");
    }
}

double Camera::getFPS() {
    return effFPS;
}

int Camera::getImageFormat() {
    if (enableRoi) {
        std::cout << "Roi is enabled!" << std::endl;
    }
    return imageFormat;
}

int Camera::getColorMode() {
    return effColorMode;
}

void Camera::ReadJsonConfig(std::string path2Settings) {
    // Load json config file
    std::ifstream f(path2Settings);
    config = json::parse(f);
    f.close();

    // Manually
    imageFormat = config["manual"]["imageFormat"];
    colorMode = config["manual"]["colorMode"];
    exposureTime = config["manual"]["exposureTime"];
    shutterMode = config["manual"]["shutterMode"];
    pixelCock = config["manual"]["pixelCock"];
    fps = config["manual"]["fps"];
    offset = config["manual"]["offset"];
    gain = config["manual"]["gain"];
    gamma = config["manual"]["gamma"];
    binning = config["manual"]["binning"];
    enableRoi = config["manual"]["enableRoi"];
    rectAOI.s32X = config["manual"]["roi"]["x"];
    rectAOI.s32Y = config["manual"]["roi"]["y"];
    rectAOI.s32Width = config["manual"]["roi"]["width"];
    rectAOI.s32Height = config["manual"]["roi"]["height"];

    // Auto Settings
    autoGain = config["auto"]["gain"];
    autoShutter = config["auto"]["shutter"];
    autoWhiteBalance = config["auto"]["whiteBalance"];
    autoFramerate = config["auto"]["framerate"];
}

bool Camera::checkConfigFile(std::string path2Settings, const std::string filetype) {
    std::size_t pos = path2Settings.find_last_of(".");
    if (path2Settings.substr(pos) == filetype) {
        return true;
    }
    return false;
}

void Camera::ConfigureManualParams() {
    std::cout << std::endl;
    std::cout << "Error states configure manual params:" << std::endl;
    std::cout << "-----------------------------------------------------------" << std::endl;

    // Shutter Mode
    nRet = is_DeviceFeature(hCam, IS_DEVICE_FEATURE_CMD_SET_SHUTTER_MODE,
                            (void *)&shutterMode, sizeof(shutterMode));
    printError("Set shutter mode: ", "\t\t");

    // Image format
    nRet = is_ImageFormat(hCam, IMGFRMT_CMD_SET_FORMAT, (void *)&imageFormat, sizeof(imageFormat));
    printError("Set image format: ", "\t\t");

    // Color mode
    nRet = is_SetColorMode(hCam, colorMode);
    printError("Set color mode: ", "\t\t");

    // Exposure Time
    SetExposureTime(exposureTime);

    // Pixel Clock
    nRet = is_PixelClock(hCam, IS_PIXELCLOCK_CMD_SET, (void *)&pixelCock, sizeof(pixelCock));
    printError("Set pixel clock: ", "\t\t");

    // FPS
    nRet = is_SetFrameRate(hCam, fps, &effFPS);
    printError("Set framerate: ", "\t\t\t");

    // Offset
    nRet = is_Blacklevel(hCam, IS_BLACKLEVEL_CMD_SET_OFFSET, (void *)&offset, sizeof(offset));
    printError("Set Backlevel/Offset: ", "\t\t");

    // Gain
    nRet = is_DeviceFeature(hCam, IS_DEVICE_FEATURE_CMD_SET_SENSOR_SOURCE_GAIN, (void *)&gain, sizeof(gain));
    printError("Set gain: ", "\t\t\t");

    // Gamma
    nRet = is_Gamma(hCam, IS_GAMMA_CMD_SET, (void *)&gamma, sizeof(gamma));
    printError("Set gamma: ", "\t\t\t");

    // Binning
    if (binning) {
        setBinning();
    }

    // Roi
    if (enableRoi) {
        setRoi();
    }

    std::cout << "-----------------------------------------------------------" << std::endl;
}

void Camera::ConfigureAutoParams() {
    std::cout << std::endl;
    std::cout << "Error states configure auto params:" << std::endl;
    std::cout << "-----------------------------------------------------------" << std::endl;

    nRet = is_SetAutoParameter(hCam, IS_SET_ENABLE_AUTO_GAIN, &autoGain, 0);
    printError("Set auto gain: ", "\t\t\t");

    nRet = is_SetAutoParameter(hCam, IS_SET_ENABLE_AUTO_SHUTTER, &autoShutter, 0);
    printError("Set auto shutter: ", "\t\t");

    nRet = is_SetAutoParameter(hCam, IS_SET_ENABLE_AUTO_WHITEBALANCE, &autoWhiteBalance, 0);
    printError("Set auto white balance: ", "\t");

    nRet = is_SetAutoParameter(hCam, IS_SET_ENABLE_AUTO_FRAMERATE, &autoFramerate, 0);
    printError("Set auto auto framerate: ", "\t");

    std::cout << "-----------------------------------------------------------" << std::endl;
}

void Camera::SetExposureTime(double expTime) {
    int enable = 1;
    int disable = 0;
    exposureTime = expTime;
    if (exposureTime > 80) {
        nRet = is_Exposure(hCam, IS_EXPOSURE_CMD_SET_LONG_EXPOSURE_ENABLE, (void *)&enable, sizeof(enable));
        printError("Long exposure Error: ", "\t");
    } else {
        nRet = is_Exposure(hCam, IS_EXPOSURE_CMD_SET_LONG_EXPOSURE_ENABLE, (void *)&disable, sizeof(disable));
        printError("Long exposure disable Error: ", "\t");
    }

    is_Exposure(hCam, IS_EXPOSURE_CMD_SET_EXPOSURE,
                (void *)&exposureTime,
                sizeof(exposureTime));
}

void Camera::setBinning() {
    nRet = is_SetBinning(hCam, IS_BINNING_2X_VERTICAL);
    printError("Set binning 2x: ", "\t\t");
    int supportedBin = is_SetBinning(hCam, IS_GET_SUPPORTED_BINNING);
    std::cout << "Supported binnings: \t\t" << supportedBin << std::endl;
}

void Camera::setRoi() {
    nRet = is_AOI(hCam, IS_AOI_IMAGE_SET_AOI, (void *)&rectAOI, sizeof(rectAOI));
    printError("Set roi:", "\t\t\t");
}

void Camera::getAvailableFormats() {
    // Get number of available formats and size of list
    UINT count;
    UINT bytesNeeded = sizeof(IMAGE_FORMAT_LIST);
    nRet =
        is_ImageFormat(hCam, IMGFRMT_CMD_GET_NUM_ENTRIES, &count, sizeof(count));
    bytesNeeded += (count - 1) * sizeof(IMAGE_FORMAT_INFO);
    ptr = malloc(bytesNeeded);

    // Create and fill list
    pformatList = (IMAGE_FORMAT_LIST *)ptr;
    pformatList->nSizeOfListEntry = sizeof(IMAGE_FORMAT_INFO);
    pformatList->nNumListElements = count;
    nRet = is_ImageFormat(hCam, IMGFRMT_CMD_GET_LIST, pformatList, bytesNeeded);
    if (nRet != IS_SUCCESS) {
        throw std::runtime_error("Fetch available formats failed");
    }
    // Print available formats
    else {
        std::cout << std::endl;
        std::cout << "Available format list:" << std::endl;
        std::cout << "-----------------------------------------------------------"
                  << std::endl;
        IMAGE_FORMAT_INFO formatInfo;
        for (UINT i = 0; i < count; i++) {
            formatInfo = pformatList->FormatInfo[i];
            std::cout << std::endl;
            std::cout << "Format nr. " << i << ":" << std::endl;
            std::cout << "********************************************" << std::endl;
            std::cout << "Format ID: "
                      << "\t\t" << formatInfo.nFormatID << std::endl;
            std::cout << "Width: "
                      << "\t\t\t" << formatInfo.nWidth << std::endl;
            std::cout << "Height: "
                      << "\t\t" << formatInfo.nHeight << std::endl;
            std::cout << "X0: "
                      << "\t\t\t" << formatInfo.nX0 << std::endl;
            std::cout << "Y0: "
                      << "\t\t\t" << formatInfo.nY0 << std::endl;
            std::cout << "Capture modes: "
                      << "\t\t" << formatInfo.nSupportedCaptureModes << std::endl;
            std::cout << "Binning mode: "
                      << "\t\t" << formatInfo.nBinningMode << std::endl;
            std::cout << "Subsampling mode: "
                      << "\t" << formatInfo.nSubsamplingMode << std::endl;
            std::cout << "Format: "
                      << "\t\t" << formatInfo.strFormatName << std::endl;
            std::cout << "Scaling factor: "
                      << "\t" << formatInfo.dSensorScalerFactor << std::endl;
            std::cout << "********************************************" << std::endl;
        }
        std::cout << "-----------------------------------------------------------"
                  << std::endl;
    }
}

void Camera::fetchCameraConfig() {
    std::cout << std::endl;
    std::cout << "Error states fetch camera config:" << std::endl;
    std::cout << "-----------------------------------------------------------" << std::endl;

    // Shutter mode
    nRet = is_DeviceFeature(hCam, IS_DEVICE_FEATURE_CMD_GET_SHUTTER_MODE,
                            (void *)&effShutterMode, sizeof(effShutterMode));
    printError("Fetch shutter mode:", "\t");

    // Color mode
    effColorMode = is_SetColorMode(hCam, IS_GET_COLOR_MODE);

    // Exposure Time
    nRet = is_Exposure(hCam, IS_EXPOSURE_CMD_GET_EXPOSURE, (void *)&effExposureTime, sizeof(effExposureTime));
    printError("Fetch exposure time", "\t\t\t");

    // Pixel Clock
    nRet = is_PixelClock(hCam, IS_PIXELCLOCK_CMD_GET, (void *)&effPixelCock, sizeof(effPixelCock));
    printError("Fetch pixel clock", "\t\t\t");

    // Offset
    nRet = is_Blacklevel(hCam, IS_BLACKLEVEL_CMD_GET_OFFSET, (void *)&effOffset, sizeof(effOffset));
    printError("Fetch frames captured per second", "\t");

    // Gain
    nRet = is_DeviceFeature(hCam, IS_DEVICE_FEATURE_CMD_GET_SENSOR_SOURCE_GAIN, (void *)&effGain, sizeof(effGain));
    printError("Fetch gain", "\t\t\t\t");

    // Gamma
    nRet = is_Gamma(hCam, IS_GAMMA_CMD_GET, (void *)&effGamma, sizeof(effGamma));
    printError("Fetch gamma", "\t\t\t\t");

    // Roi / img format
    nRet = is_AOI(hCam, IS_AOI_IMAGE_GET_AOI, (void *)&effRectAOI,
                  sizeof(effRectAOI));
    printError("Fetch AOI:", "\t\t");

    std::cout << "-----------------------------------------------------------" << std::endl;
}

void Camera::PrintCamInfo() {
    std::cout << std::endl;
    std::cout << "Camera info:" << std::endl;
    std::cout << "-----------------------------------------------------------" << std::endl;
    std::cout << "Cam serial number: \t" << camInfo.SerNo << std::endl;
    std::cout << "Cam ID: \t\t" << camInfo.ID << std::endl;
    std::cout << "Cam version: " << camInfo.Version << std::endl;
    std::cout << "Cam date: \t\t" << camInfo.Date << std::endl;
    std::cout << "Cam select (Cam ID): \t" << (int)camInfo.Select << std::endl;
    std::cout << "Cam type: \t\t" << camInfo.Type << std::endl;
    std::cout << "-----------------------------------------------------------" << std::endl;
}

void Camera::PrintSensorInfo() {
    std::cout << std::endl;
    std::cout << "Sensor info:" << std::endl;
    std::cout << "-----------------------------------------------------------" << std::endl;
    std::cout << "Sensor ID: \t\t\t\t" << sensorInfo.SensorID << std::endl;
    std::cout << "Sensor Name: \t\t\t\t" << sensorInfo.strSensorName << std::endl;
    std::cout << "Sensor color mode: \t\t\t" << (int)sensorInfo.nColorMode << std::endl;
    std::cout << "Sensor max. width: \t\t\t" << sensorInfo.nMaxWidth << std::endl;
    std::cout << "Sensor max. height: \t\t\t" << sensorInfo.nMaxHeight << std::endl;
    std::cout << "Sensor master gain available: \t\t" << sensorInfo.bMasterGain << std::endl;
    std::cout << "Sensor red channel gain available: \t" << sensorInfo.bRGain << std::endl;
    std::cout << "Sensor green channel gain available: \t" << sensorInfo.bGGain << std::endl;
    std::cout << "Sensor blue channel gain available: \t" << sensorInfo.bBGain << std::endl;
    std::cout << "Sensor gobal shutter available: \t" << sensorInfo.bGlobShutter << std::endl;
    std::cout << "Sensor pixel size (um): \t\t" << sensorInfo.wPixelSize << std::endl;
    std::cout << "Sensor top left bayer pixel value: \t" << sensorInfo.nUpperLeftBayerPixel << std::endl;
    std::cout << "-----------------------------------------------------------" << std::endl;
}

void Camera::printCameraConfig() {
    std::cout << std::endl;
    std::cout << "Current camera params:" << std::endl;
    std::cout << "-----------------------------------------------------------"
              << std::endl;
    std::cout << "Active shutter mode: \t\t" << effShutterMode << std::endl;
    std::cout << "Active color mode: \t\t" << effColorMode << std::endl;
    std::cout << "Active exposure time [ms]: \t" << effExposureTime << std::endl;
    std::cout << "Active pixel clock [MHz]: \t" << effPixelCock << std::endl;
    std::cout << "Actual FPS: \t\t\t" << effFPS << std::endl;
    std::cout << "Actual Backlevel/Offset: \t" << effOffset << std::endl;
    std::cout << "Actual gain: \t\t\t" << effGain << std::endl;
    std::cout << "Actual gamma: \t\t\t" << effGamma << std::endl;
    std::cout << "roi: \t\t\t\t"
              << "(X: " << effRectAOI.s32X << ", "
              << "Y: " << effRectAOI.s32Y << ", "
              << "Width: " << effRectAOI.s32Width << ", "
              << "Height: " << effRectAOI.s32Height << ")" << std::endl;
    std::cout << "-----------------------------------------------------------"
              << std::endl;
}

void Camera::PrintJsonConfig() {
    std::cout << std::endl;
    std::cout << "Json configuration:" << std::endl;
    std::cout << "-----------------------------------------------------------" << std::endl;
    std::cout << "imageFormat: \t" << imageFormat << " ("
              << formatDefinition[imageFormat].width << "x"
              << formatDefinition[imageFormat].height << ")" << std::endl;
    std::cout << "colorMode: \t" << colorMode << std::endl;
    std::cout << "exposureTime: \t" << exposureTime << std::endl;
    std::cout << "shutter mode: \t" << shutterMode << std::endl;
    std::cout << "pixelCock: \t" << pixelCock << std::endl;
    std::cout << "fps: \t\t" << fps << std::endl;
    std::cout << "offset: \t" << offset << std::endl;
    std::cout << "gain: \t\t" << gain << std::endl;
    std::cout << "gamma: \t\t" << gamma << std::endl;
    std::cout << "roi: \t\t"
              << "(X: " << rectAOI.s32X << ", "
              << "Y: " << rectAOI.s32Y << ", "
              << "Width: " << rectAOI.s32Width << ", "
              << "Height: " << rectAOI.s32Height << ")" << std::endl
              << std::endl;

    std::cout << "auto gain: \t\t" << autoGain << std::endl;
    std::cout << "auto shutter: \t\t" << autoShutter << std::endl;
    std::cout << "auto white balance: \t" << autoWhiteBalance << std::endl;
    std::cout << "auto framerate: \t" << autoFramerate << std::endl;
    std::cout << "-----------------------------------------------------------" << std::endl;
}

void Camera::printError(std::string errDescription, std::string tabs = "") {
    // Error
    if (nRet != 0) {
        int lastError = 0;
        char *errStr = "";

        is_GetError(hCam, &lastError, &errStr);
        std::cerr << errDescription << tabs << errStr << " "
                  << "(" << nRet << ")" << std::endl;
    } else {
        std::cerr << errDescription << tabs << "No error "
                  << "(" << nRet << ")" << std::endl;
    }
}
