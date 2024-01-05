#include "car_controller.hpp"

CarController::CarController(MemHandler* hmem, Camera* camera, SerialSTM32* serial, trackData data)
    : mem(hmem), cam(camera), ser(serial), track_data(data) {
    meanMasked = applyMask(track_data.mean_img);
    lastImg = meanMasked.clone();
    generateKDTree();
}

void CarController::run() {
    cam->startAqusition(CONTINOUS_FREERUN);
    position initPos = findStaticCar();
    size_t nearest;
    if (!initPos.found) {
        initPos.coord = cv::Point2i(645, 1062);
        std::cout << "Assumeing car is under bridge." << std::endl;
    }
    lastPos = initPos.coord;
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

    point_t pt = {double(initPos.coord.x), double(initPos.coord.y)};
    nearest = tree.nearest_index(pt);
    // std::cout << nearest << std::endl;

    ser->WriteSerialPort(track_data.data.at(nearest).velocity);
    position runPos;

    windowName = "Live Position | Press 'q' to quit.";
    cv::namedWindow(windowName, cv::WINDOW_NORMAL);
    cv::resizeWindow(windowName, 300, 666);
    while (true) {
        // startTime = clock();
        runPos = findMovingCar();
        if (runPos.found) {
            pt = {double(runPos.coord.x), double(runPos.coord.y)};
            nearest = tree.nearest_index(pt);
            ser->WriteSerialPort(track_data.data.at(nearest).velocity);
            cv::circle(displyMovingCar, track_data.data.at(nearest).point, 70, cv::Scalar(255), 3);
            // std::cout << "Nearest: " << nearest << std::endl;
            cv::putText(displyMovingCar, std::to_string(vel), cv::Point(50, 50), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(255), 3);
            std::cout << "Velocity: " << track_data.data.at(nearest).velocity << std::endl;
        }
        // std::cout << mem->getRealFps() << std::endl;
        cv::imshow(windowName, displyMovingCar);
        int c = cv::pollKey();
        if (c > 0) {
            break;
        }
        // std::cout << "Elapsed time: " << clock() - startTime << std::endl;
    }
    std::cout << "Terminated." << std::endl;
    ser->WriteSerialPort(0);
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
    displyMovingCar = mem->getOpenCVMatRingImg().clone();
    current = applyMask(displyMovingCar).clone();
    // std::cout << "Is empty: " << current.empty() << std::endl;
    cv::Rect b = carPosition(meanMasked, current);
    if (hasValidSize(b)) {
        pos.coord = (b.br() + b.tl()) * 0.5;
        pos.found = true;
        calcVelocity();
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
    cv::absdiff(img1, img2, diff);
    cv::threshold(diff, binaryDiff, 20, 255, cv::THRESH_BINARY);
    return cv::boundingRect(binaryDiff);
}

bool CarController::hasValidSize(cv::Rect boundingRect) {
    // std::cout << boundingRect.area() << std::endl;
    return boundingRect.area() > 1000 && boundingRect.area() < 60000;
}

void CarController::generateKDTree() {
    pointVec kdPoints;
    point_t pt;
    for (auto data : track_data.data) {
        pt = {double(data.point.x), double(data.point.y)};
        kdPoints.push_back(pt);
        /* for (auto p : pt) {
            std::cout << p << ", ";
        }
        std::cout << std::endl; */
    }
    tree = KDTree(kdPoints);
}

void CarController::calcVelocity() {
    currentTime = mem->getTimeStamp();
    currentPos = pos.coord;

    double distPxl = sqrt(pow(double(currentPos.x - lastPos.x), 2) + pow(double(currentPos.y - lastPos.y), 2));
    if (distPxl < 1.0) {
        return;
    }
    double dTime;
    if (currentTime.second < lastTime.second) {
        dTime = double(currentTime.second + (1000 - lastTime.second));
    } else {
        dTime = currentTime.second - lastTime.second;
    }
    if (dTime < 1.0) {
        return;
    } else {
        vel = ((distPxl / 17.25) / dTime) * 10;
    }
    // std::cout << "Dist pxl: " << distPxl << ", dTime: " << dTime << ", vel: " << vel << "\n";
    lastTime = currentTime;
    lastPos = currentPos;
}
