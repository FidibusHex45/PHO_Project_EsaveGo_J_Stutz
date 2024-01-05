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
    CarController(MemHandler *hmem, Camera *camera, SerialSTM32 *serial, trackData data);
    void run();

   private:
    position findStaticCar();
    position findMovingCar();
    cv::Mat applyMask(cv::Mat img);
    cv::Rect carPosition(cv::Mat img1, cv::Mat img2);
    bool hasValidSize(cv::Rect boundingRect);
    void generateKDTree();
    void calcVelocity();

    double startTime = 0;
    std::pair<WORD, WORD> currentTime;
    std::pair<WORD, WORD> lastTime = {0, 0};
    cv::Point2i currentPos;
    cv::Point2i lastPos;
    double vel;

    position pos;

    cv::Point initialCarPos;
    cv::Mat meanMasked;
    cv::Mat diff;
    cv::Mat binaryDiff;
    cv::Mat lastImg;
    cv::Mat current;
    trackData track_data;
    KDTree tree;

    cv::Mat displyMovingCar;

    Camera *cam;
    SerialSTM32 *ser;
    MemHandler *mem;
};

#endif