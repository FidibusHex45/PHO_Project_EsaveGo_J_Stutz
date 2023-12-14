#include "car_controller.hpp"

CarController::CarController(MemHandler* hmem, Camera* camera, SerialSTM32* serial, trackData data, std::string configPath)
    : mem(hmem), cam(camera), ser(serial), track_data(data) {
    cam->ConfigureCam(configPath);
    meanMasked = applyMask(track_data.mean_img);
}

void CarController::run() {
    position initPos = findStaticCar();
    pointIndex nearest;
    if (!initPos.found) {
        initPos.coord = cv::Point2i(645, 1062);
        std::cout << "Assumeing car is under bridge." << std::endl;
    }
    cv::Mat initPosVis = track_data.mean_img.clone();
    cv::circle(initPosVis, initPos.coord, 10, cv::Scalar(255), cv::FILLED);
    std::string windowName = "Initial Position | Press 'q' if position is wrong.";
    cv::namedWindow(windowName, cv::WINDOW_NORMAL);
    cv::resizeWindow(windowName, 300, 666);
    cv::imshow(windowName, initPosVis);
    char c = cv::waitKey(0);
    if (c == 'q') {
        return;
    }
    cv::destroyWindow(windowName);

    windowName = "Live Position | Press 'q' to quit.";
    cv::namedWindow(windowName, cv::WINDOW_NORMAL);
    cv::resizeWindow(windowName, 300, 666);
    nearest = tree.nearest_pointIndex(point_t(initPos.coord.x, initPos.coord.y));
    ser->WriteSerialPort(track_data.data.at(nearest.second).velocity);
    position runPos;
    while (true) {
        startTime = clock();
        runPos = findMovingCar();
        if (runPos.found) {
            nearest = tree.nearest_pointIndex(point_t(runPos.coord.x, runPos.coord.y));
            ser->WriteSerialPort(track_data.data.at(nearest.second).velocity);
        }
        cv::imshow(windowName, initPosVis);
        char c = cv::waitKey(0);
        if (c == 'q') {
            return;
        }
        std::cout << "Elapsed time: " << clock() - startTime << std::endl;
    }
}

position CarController::findStaticCar() {
    cv::Mat carOnTrack = applyMask(mem->getOpenCVMatRingImg());
    cv::Rect b;
    position pos;
    for (int i = 0; i < 10; i++) {
        b = carPosition(meanMasked, carOnTrack);
        if (hasValidSize(b)) {
            pos.coord = (b.br() + b.tl()) * 0.5;
            pos.found = true;
            return pos;
        }
    }
    // std::cerr << "Car not found, make sure car is on track." << std::endl;
    pos.found = false;
    return pos;
}

position CarController::findMovingCar() {
    position pos;
    cv::Mat current = applyMask(mem->getOpenCVMatRingImg());
    cv::Rect b = carPosition(lastImg, current);
    if (hasValidSize(b)) {
        pos.coord = (b.br() + b.tl()) * 0.5;
        pos.found = true;
        return pos;
    }
    lastImg = current.clone();
    pos.found = false;
    return pos;
}

cv::Mat CarController::applyMask(cv::Mat img) {
    cv::Mat maskedImg;
    img.copyTo(maskedImg, track_data.mask);
    return maskedImg;
}

cv::Rect CarController::carPosition(cv::Mat img1, cv::Mat img2) {
    diff = img1 - img2;
    binaryDiff;
    cv::threshold(diff, binaryDiff, 30, 255, cv::THRESH_BINARY);
    return cv::boundingRect(binaryDiff);
}

bool CarController::hasValidSize(cv::Rect boundingRect) {
    return boundingRect.area() > 200 && boundingRect.area() < 50000;
}

void CarController::generateKDTree() {
    pointVec kdPoints;
    point_t pt;
    for (auto data : track_data.data) {
        pt = {double(data.point.x), double(data.point.y)};
        kdPoints.push_back(pt);
    }
    tree = KDTree(kdPoints);
}
