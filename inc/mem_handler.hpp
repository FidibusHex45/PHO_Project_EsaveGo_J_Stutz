#ifndef MEMHANDLER_HPP
#define MEMHANDLER_HPP

#include <time.h>
#include <uEye.h>
#include <uEye_tools.h>

#include <fstream>
#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>

class MemHandler {
   public:
    MemHandler(HIDS *hCam, int bufferSize);
    MemHandler(HIDS *hCam);
    ~MemHandler();
    void allocRingBuffer();
    void allocSingleBuffer();
    cv::Mat getOpenCVMatRingImg();
    cv::Mat getOpenCVMatSingleImg();
    void drawFpsOnImg(cv::Mat dest);
    void drawDroppedFramesOnImg(cv::Mat dest);
    void drawTimeStamp(cv::Mat dest);
    void saveSingleImg(std::string path2file, const std::optional<cv::Mat> &mat = std::nullopt);

    int getActiveBufferID();
    int activeBufferIdx = 0;

   private:
    char *getLatestImgRingPointer();
    void checkBufferSize(int bufSize);
    void unlockImgRingMem();
    void freeImgMem();
    void hanldeDroppedFrames(int hanldeDroppedFrames);
    void handleTimeStamp(int activeBufferIdx);
    int getDropedFramesSum();
    double getRealFps();

    void setCVMatformat();

    // Memory ring buffer
    INT m_lSeqMemId[1000] = {};      // camera memory - buffer ID
    char *m_pcSeqImgMem[1000] = {};  // camera memory - pointer to buffer
    int m_nSeqNumId[1000] = {};      // varibale to hold the number of the sequence buffer Id
    bool enableRingImg = false;

    // Memory single buffer
    char *imgMem;  // Zeiger auf reservierten Speicherbereich für das Bild
    int memId;     // ID für den Bildspeicher (mehrere Bildspeicher sind möglich)
    bool enableSingleImg = false;

    int m_Ret;

    HIDS *hCam;
    double fps = 0;
    int colorMode = 0;
    IS_RECT format;
    int imgWidth = 0;
    int imgHeight = 0;

    int imgFormat = 0;

    int bufferSize = 20;
    int nBits = 0;
    int dropedFramesSum = 0;
    double clockLastFpsUpdate = 0;
    int showedImg = 0;
    double realFps = 0.0;
    std::stringstream timeStamp;

    int openCVFormat;
};

#endif