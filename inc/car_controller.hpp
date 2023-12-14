#ifndef CAR_CONTROLLER_HPP
#define CAR_CONTROLLER_HPP

#include "KDTree.hpp"
#include "detectTrack.hpp"
#include "keystate.hpp"

typedef struct {
    cv::Point2i coord;
    bool found;
} position;

class CarController {
   public:
    CarController(MemHandler *hmem, Camera *camera, SerialSTM32 *serial, trackData data, std::string configPath);
    void run();

   private:
    position findStaticCar();
    position findMovingCar();
    cv::Mat applyMask(cv::Mat img);
    cv::Rect carPosition(cv::Mat img1, cv::Mat img2);
    bool hasValidSize(cv::Rect boundingRect);
    void generateKDTree();

    double startTime = 0;

    cv::Point initialCarPos;
    cv::Mat meanMasked;
    cv::Mat diff;
    cv::Mat binaryDiff;
    cv::Mat lastImg;
    trackData track_data;
    KDTree tree;

    Camera *cam;
    SerialSTM32 *ser;
    MemHandler *mem;
};

#endif