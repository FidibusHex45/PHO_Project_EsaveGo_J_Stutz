#ifndef DETECTTRACK_HPP
#define DETECTTRACK_HPP

#include <Python.h>
#include <windows.h>

#include <cmath>
#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <stdexcept>
#include <string>

#include "camera.hpp"
#include "csv_handler.hpp"
#include "mem_handler.hpp"
#include "serialSTM32.hpp"

typedef struct {
    std::vector<splineData_proc> data;
    cv::Mat mask;
    cv::Mat mean_img;
} trackData;

class Trackdetector : private CSVHandler {
   public:
    Trackdetector(MemHandler *hmem, Camera *camera, SerialSTM32 *serial, std::string configurePath);
    trackData detectTrack(std::string load_path, std::string save_path);

   private:
    void recordSamples(int nSamples);
    void evaluateSamples(int thres);
    void evaluateSampleMean();
    cv::Mat evaluateMask();
    cv::Point2i centerPixelCloud(cv::Mat img, cv::Rect rect);
    void executePyScript(std::string interpreter_path, std::string script_path);
    void visualizeSpline(cv::Mat &dst);

    std::vector<cv::Mat> trackSamples;
    std::vector<cv::Point> trackPoints;
    std::vector<splineData_proc> spline_data_proc;
    trackData track_Data;
    cv::Mat sampleMean;
    std::string configPath;

    Camera *cam;
    SerialSTM32 *ser;
    CSVHandler hcsv;
    MemHandler *mem;
};

#endif