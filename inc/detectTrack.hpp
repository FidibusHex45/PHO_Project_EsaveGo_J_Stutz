#ifndef DETECTTRACK_HPP
#define DETECTTRACK_HPP

#include <conio.h>
#include <windows.h>

#include <cmath>
#include <filesystem>
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

class TrackHandler {
   public:
    TrackHandler(MemHandler *hmem, Camera *camera, SerialSTM32 *serial);
    trackData getTrackData(std::string load_path, std::string save_path);

   private:
    void scanTrack(std::string load_path, std::string save_path);
    void loadTrack(std::string load_path);
    void calibrateLoadedConfig();
    void recordSamples(int nSamples);
    void evaluateSamples(int thres);
    void evaluateSampleMean();
    cv::Mat evaluateMask();
    cv::Point2i centerPixelCloud(cv::Mat img, cv::Rect rect);
    void executePyScript(std::string interpreter_path, std::string script_path);
    void visualizeSpline(cv::Mat &dst);
    void saveConfiguration();
    void loadConfiguration();

    std::vector<cv::Mat> trackSamples;
    std::vector<cv::Point> trackPoints;
    std::vector<splineData_proc> spline_data_proc;
    trackData track_Data;
    cv::Mat sampleMean;

    Camera *cam;
    SerialSTM32 *ser;
    CSVHandler hcsv;
    MemHandler *mem;
};

#endif