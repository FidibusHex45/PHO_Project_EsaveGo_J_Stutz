#include "mem_handler.hpp"

MemHandler::MemHandler(HIDS *hCam, int bufferSize)
    : hCam(hCam), bufferSize(bufferSize) {
    checkBufferSize(bufferSize);
    nBits = is_SetColorMode(*hCam, IS_GET_BITS_PER_PIXEL);
    colorMode = is_SetColorMode(*hCam, IS_GET_COLOR_MODE);
    m_Ret = is_AOI(*hCam, IS_AOI_IMAGE_GET_AOI, (void *)&format, sizeof(format));

    imgWidth = format.s32Width;
    imgHeight = format.s32Height;

    setCVMatformat();
}

MemHandler::MemHandler(HIDS *hCam) : hCam(hCam) {
    m_Ret = is_SetFrameRate(*hCam, IS_GET_FRAMERATE, &fps);
    nBits = is_SetColorMode(*hCam, IS_GET_BITS_PER_PIXEL);
    colorMode = is_SetColorMode(*hCam, IS_GET_COLOR_MODE);
    m_Ret = is_AOI(*hCam, IS_AOI_IMAGE_GET_AOI, (void *)&format, sizeof(format));

    bufferSize = (int)(fps + 0.5);
    checkBufferSize(bufferSize);
    imgWidth = format.s32Width;
    imgHeight = format.s32Height;

    setCVMatformat();
}

MemHandler::~MemHandler() {
    freeImgMem();
}

void MemHandler::allocRingBuffer() {
    enableRingImg = true;
    enableSingleImg = false;
    freeImgMem();
    for (auto i = 0; i < bufferSize; i++) {
        // allocate buffer memory
        m_Ret = is_AllocImageMem(*hCam,
                                 imgWidth,
                                 imgHeight,
                                 nBits,
                                 &m_pcSeqImgMem[i],
                                 &m_lSeqMemId[i]);
        if (m_Ret != IS_SUCCESS) {
            throw std::runtime_error("Allocate ring buffer failed!");
        }

        // put memory into seq buffer
        m_Ret = is_AddToSequence(*hCam, m_pcSeqImgMem[i], m_lSeqMemId[i]);
        m_nSeqNumId[i] = i + 1;  // store sequence buffer number Id
        if (m_Ret != IS_SUCCESS) {
            // free latest buffer
            is_FreeImageMem(*hCam, m_pcSeqImgMem[i], m_lSeqMemId[i]);
            break;  // it makes no sense to continue
        }
    }
}

void MemHandler::allocSingleBuffer() {
    enableRingImg = false;
    enableSingleImg = true;
    freeImgMem();
    // allocate buffer memory
    m_Ret = is_AllocImageMem(*hCam,
                             imgWidth,
                             imgHeight,
                             nBits,
                             &imgMem,
                             &memId);
    if (m_Ret != IS_SUCCESS) {
        throw std::runtime_error("Allocate single Buffer failed!");
    }
    // Set cam memory
    is_SetImageMem(*hCam, imgMem, memId);
}

cv::Mat MemHandler::getOpenCVMatRingImg() {
    if (enableRingImg) {
        auto imgPointer = getLatestImgRingPointer();
        cv::Mat opCV = cv::Mat(imgHeight,
                               imgWidth,
                               openCVFormat, imgPointer);
        unlockImgRingMem();
        return opCV;
    } else {
        throw std::runtime_error("Wrong capturing Mode activated!");
    }
}

cv::Mat MemHandler::getOpenCVMatSingleImg() {
    if (enableSingleImg) {
        void *pMemVoid;
        is_GetImageMem(*hCam, &pMemVoid);
        cv::Mat opCV = cv::Mat(imgHeight,
                               imgWidth,
                               openCVFormat, pMemVoid);
        return opCV;
    } else {
        throw std::runtime_error("Wrong capturing Mode activated!");
    }
}

void MemHandler::drawFpsOnImg(cv::Mat dest) {
    std::ostringstream fpsStr;
    fpsStr.precision(3);
    fpsStr << std::fixed << realFps;
    cv::putText(dest, "real FPS: " + std::move(fpsStr).str(), cv::Point(dest.cols - 560, dest.rows - 40), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(255, 0, 255), 5);
}
void MemHandler::drawDroppedFramesOnImg(cv::Mat dest) {
    std::string droppedFramesSumStr = std::to_string(dropedFramesSum);
    cv::putText(dest, "dropped Frames: " + droppedFramesSumStr, cv::Point(20, dest.rows - 40), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(255, 0, 255), 5);
}
void MemHandler::drawTimeStamp(cv::Mat dest) {
    cv::putText(dest, timeStamp.str(), cv::Point(20, 70), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(255, 0, 255), 5);
}

void MemHandler::saveSingleImg(std::string path2file, const std::optional<cv::Mat> &mat) {
    if (mat) {
        cv::imwrite(path2file, *mat);
    } else {
        if (enableSingleImg) {
            cv::Mat singleImg = getOpenCVMatSingleImg();
            cv::imwrite(path2file, singleImg);
        } else {
            throw std::runtime_error("Wrong capturing Mode activated!");
        }
    }
}

char *MemHandler::getLatestImgRingPointer() {
    INT nNum;
    char *pcMem, *pcMemLast;
    is_GetActSeqBuf(*hCam, &nNum, &pcMem, &pcMemLast);
    // std::cout << "Active seq buffer: " << nNum << std::endl;
    int lastPointerPos = activeBufferIdx;
    for (activeBufferIdx = 0; activeBufferIdx < bufferSize; activeBufferIdx++) {
        if (pcMemLast == m_pcSeqImgMem[activeBufferIdx])
            break;
    }
    // std::cout << activeBufferIdx << std::endl;
    hanldeDroppedFrames(lastPointerPos);
    handleTimeStamp();

    // lock buffer for processing
    m_Ret = is_LockSeqBuf(*hCam, m_nSeqNumId[activeBufferIdx], m_pcSeqImgMem[activeBufferIdx]);

    return m_pcSeqImgMem[activeBufferIdx];
}

int MemHandler::getActiveBufferID() {
    INT nNum;
    char *pcMem, *pcMemLast;
    is_GetActSeqBuf(*hCam, &nNum, &pcMem, &pcMemLast);
    // std::cout << "Active seq buffer: " << nNum << std::endl;
    int lastPointerPos = activeBufferIdx;
    for (activeBufferIdx = 0; activeBufferIdx < bufferSize; activeBufferIdx++) {
        if (pcMemLast == m_pcSeqImgMem[activeBufferIdx])
            break;
    }
    return activeBufferIdx;
}

void MemHandler::checkBufferSize(int bufSize) {
    if (bufSize > 999 || bufSize < 1) {
        throw std::runtime_error("Buffersize invalid!");
    }
}

void MemHandler::unlockImgRingMem() {
    is_UnlockSeqBuf(*hCam, m_nSeqNumId[activeBufferIdx], m_pcSeqImgMem[activeBufferIdx]);
}

void MemHandler::freeImgMem() {
    // For single buffer
    if (enableSingleImg) {
        is_FreeImageMem(*hCam, imgMem, memId);
    }

    // For ring buffer
    if (enableRingImg) {
        int nRet = is_ImageQueue(*hCam, IS_IMAGE_QUEUE_CMD_EXIT, NULL, 0);
        // remove the buffers from the sequence
        is_ClearSequence(*hCam);

        // free seq buffers
        for (auto i = (bufferSize - 1); i >= 0; i--) {
            // free buffers
            is_FreeImageMem(*hCam, m_pcSeqImgMem[i], m_lSeqMemId[i]);
        }
    }
}

int MemHandler::getDropedFramesSum() {
    return dropedFramesSum;
}

double MemHandler::getRealFps() {
    return realFps;
}

void MemHandler::hanldeDroppedFrames(int lastPointerPos) {
    if (lastPointerPos < activeBufferIdx) {
        dropedFramesSum += (activeBufferIdx - lastPointerPos - 1);
    } else if (lastPointerPos > activeBufferIdx) {
        dropedFramesSum += (bufferSize - lastPointerPos + activeBufferIdx - 1);
    }
    if (lastPointerPos != activeBufferIdx) {
        showedImg++;
    }

    auto timeSinceLastFpsUpdate = clock() - clockLastFpsUpdate;
    if (timeSinceLastFpsUpdate > 1000) {
        realFps = ((double)timeSinceLastFpsUpdate / 1000.0) * (double)showedImg;
        clockLastFpsUpdate = clock();
        showedImg = 0;
    }
}

void MemHandler::handleTimeStamp() {
    UEYEIMAGEINFO imageInfo;
    m_Ret = is_GetImageInfo(*hCam, m_nSeqNumId[activeBufferIdx], &imageInfo, sizeof(imageInfo));
    if (m_Ret == IS_SUCCESS) {
        unsigned long long u64TimestampDevice;
        u64TimestampDevice = imageInfo.u64TimestampDevice;
        timeStamp.str(std::string());
        timeStamp << imageInfo.TimestampSystem.wDay
                  << "." << imageInfo.TimestampSystem.wMonth
                  << "." << imageInfo.TimestampSystem.wYear
                  << ", " << imageInfo.TimestampSystem.wHour
                  << ":" << imageInfo.TimestampSystem.wMinute
                  << ":" << imageInfo.TimestampSystem.wSecond
                  << ":" << imageInfo.TimestampSystem.wMilliseconds;
    }
}

std::pair<WORD, WORD> MemHandler::getTimeStamp() {
    UEYEIMAGEINFO imageInfo;
    std::pair<WORD, WORD> time;
    m_Ret = is_GetImageInfo(*hCam, m_nSeqNumId[activeBufferIdx], &imageInfo, sizeof(imageInfo));
    if (m_Ret == IS_SUCCESS) {
        time.first = imageInfo.TimestampSystem.wSecond;
        time.second = imageInfo.TimestampSystem.wMilliseconds;
    }
    return time;
}

void MemHandler::setCVMatformat() {
    switch (colorMode) {
        case IS_CM_MONO16:
        case IS_CM_MONO12:
        case IS_CM_MONO10:
            openCVFormat = CV_16UC1;
            break;
        case IS_CM_MONO8:
            openCVFormat = CV_8UC1;
            break;
        case IS_CM_SENSOR_RAW16:
        case IS_CM_SENSOR_RAW12:
        case IS_CM_SENSOR_RAW10:
        case IS_CM_SENSOR_RAW8:
            throw std::runtime_error("Raw format not compatible with openCV!");
            break;
        case IS_CM_BGR12_UNPACKED:
        case IS_CM_BGR10_UNPACKED:
            openCVFormat = CV_16UC3;
            break;
        case IS_CM_BGR10_PACKED:
        case IS_CM_BGRA12_UNPACKED:
        case IS_CM_BGR565_PACKED:
        case IS_CM_BGR5_PACKED:
            throw std::runtime_error("Color format not compatible with openCV!");
            break;
        case IS_CM_BGR8_PACKED:
            openCVFormat = CV_8UC3;
            break;
        case IS_CM_BGRA8_PACKED:
        case IS_CM_BGRY8_PACKED:
        case IS_CM_UYVY_PACKED:
        case IS_CM_CBYCRY_PACKED:
            openCVFormat = CV_8UC4;
            break;
        default:
            throw std::runtime_error("Unknown format!");
            break;
    };
}
