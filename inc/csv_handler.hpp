#ifndef CSV_HANDLER_HPP
#define CSV_HANDLER_HPP

#include <fstream>
#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <string>

typedef struct {
    cv::Point point;
    double velocity;
} splineData;

typedef struct {
    cv::Point point;
    int velocity;
} splineData_proc;

class CSVHandler {
   public:
    CSVHandler();
    void save2csv(std::string save_path, std::vector<cv::Point> dataPoints);
    std::vector<splineData_proc> loadSplineData(std::string load_path, int v_lower, int v_upper);
    void mapVelocity();

   private:
    std::vector<splineData> spline_data;
    std::vector<splineData_proc> spline_data_proc;
    int v_0;
    int v_E;
};

#endif