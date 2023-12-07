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
#include "mem_handler.hpp"
#include "serialSTM32.hpp"

#define ESAVE_GO_ROI_FORMAT 100

typedef struct {
    cv::Point point;
    int slope;
} splineData;

typedef struct {
    cv::Mat mask;
    cv::Mat meanTrack;
    std::vector<splineData> splineData_vec;
}

int
rotRectArea(cv::RotatedRect rRect);
void drawRotRect(cv::Mat &dst, cv::RotatedRect rRect);
void executePyScript();
std::vector<splineData> getSplineData();
void drawSpline(cv::Mat &img, std::vector<splineData> spline_data);
void save2csv(std::vector<cv::Point> dataPoints);
cv::Mat detectTrack(HIDS &hcam, Camera &cam, SerialSTM32 &ser, std::string configPath);

#endif