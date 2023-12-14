#include "detectTrack.hpp"

Trackdetector::Trackdetector(MemHandler *hmem, Camera *camera, SerialSTM32 *serial, std::string configurePath)
    : mem(hmem), cam(camera), ser(serial), configPath(configurePath) {
    cam->ConfigureCam(configPath);
}

trackData Trackdetector::detectTrack(std::string load_path, std::string save_path) {
    recordSamples(300);
    evaluateSampleMean();
    evaluateSamples(30);
    hcsv.save2csv(save_path, trackPoints);
    executePyScript("C:/Users/joshu/anaconda3/envs/KI/python.exe", "../../py/clustering.py");
    spline_data_proc = hcsv.loadSplineData(load_path, 90, 255);
    track_Data.data = spline_data_proc;
    track_Data.mean_img = sampleMean.clone();
    track_Data.mask = evaluateMask();
    return track_Data;
}

void Trackdetector::recordSamples(int nSamples) {
    cv::Mat frame;
    int lastIdx = 0;
    int currentIdx = 0;

    ser->WriteSerialPort(90);
    while (trackSamples.size() < nSamples) {
        currentIdx = mem->getActiveBufferID();

        if (lastIdx != currentIdx) {
            lastIdx = currentIdx;
            frame = mem->getOpenCVMatRingImg();
            trackSamples.emplace_back(frame.clone());
        }
    }
    ser->WriteSerialPort(0);
}

void Trackdetector::evaluateSamples(int thres) {
    // cv::Mat demoVid = images.at(0).clone();
    cv::Mat displayImg = trackSamples.at(0).clone();
    cv::Mat lastFrame = trackSamples.at(0).clone();
    cv::Mat diffFrame;
    cv::Mat binaryDiffImg;
    cv::Rect boundingBox;
    // Video writer
    /*
    int codec = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');  // select desired codec (must be available at runtime)
    double fps = 20.0;                                        // framerate of the created video stream
    std::string filename = "../../out/detecTrackbBox.avi";    // name of the output video file
    cv::Size size(992, 2000);
    cv::VideoWriter writer(filename, codec, fps, size, true);
    */
    for (auto i = 0; i < trackSamples.size(); i++) {
        diffFrame = trackSamples.at(i) - lastFrame;
        lastFrame = trackSamples.at(i).clone();
        // demoVid = images.at(i).clone();
        // threshold default = 30
        cv::threshold(diffFrame, binaryDiffImg, thres, 255, cv::THRESH_BINARY);

        boundingBox = cv::boundingRect(binaryDiffImg);
        std::vector<cv::Point> nonZeros;
        cv::findNonZero(binaryDiffImg, nonZeros);

        if (nonZeros.size() != 0) {
            if (boundingBox.area() > 500 && boundingBox.area() < 50000) {
                cv::rectangle(displayImg, boundingBox, cv::Scalar(255), 3);
                // cv::Point center = (boundingBox.br() + boundingBox.tl()) * 0.5;
                cv::Point center = centerPixelCloud(binaryDiffImg, boundingBox);
                trackPoints.push_back(center);
                // Demo Video
                // ***********************************************************
                // cv::rectangle(demoVid, boundingBox, cv::Scalar(255), 3);
                // ***********************************************************
                std::cout << "Distance: " << cv::norm(trackPoints.at(0) - center) << std::endl;
            }
        }
        // Demo Video
        // ***********************************************************
        // cv::Mat color;
        // cv::cvtColor(demoVid, color, cv::COLOR_GRAY2BGR);
        // writer.write(color);
        // ***********************************************************
    }
    std::string windowName = "Boxes";
    cv::namedWindow(windowName, cv::WINDOW_NORMAL);
    cv::resizeWindow(windowName, 300, 666);
    cv::imshow(windowName, displayImg);
    cv::waitKey(0);
}

void Trackdetector::evaluateSampleMean() {
    if (trackSamples.empty()) {
        std::cerr << "No samples found." << std::endl;
    }

    sampleMean = cv::Mat(trackSamples[0].rows, trackSamples[0].cols, CV_64FC1);
    sampleMean.setTo(cv::Scalar(0));

    cv::Mat temp;
    for (int i = 0; i < trackSamples.size(); ++i) {
        trackSamples[i].convertTo(temp, CV_64FC1);
        sampleMean += temp;
    }

    sampleMean.convertTo(sampleMean, CV_8U, 1. / trackSamples.size());
}

cv::Mat Trackdetector::evaluateMask() {
    cv::Mat mask = cv::Mat::zeros(trackSamples.at(0).size(), CV_8UC1);
    for (int i = 1; i < spline_data_proc.size(); i++) {
        cv::line(mask, spline_data_proc.at(i - 1).point, spline_data_proc.at(i).point, cv::Scalar(255), 2);
    }
    return mask;
}

cv::Point2i Trackdetector::centerPixelCloud(cv::Mat img, cv::Rect rect) {
    cv::Mat crop = img(rect);
    std::vector<cv::Point2i> locations;
    cv::findNonZero(crop, locations);
    int x_center = 0;
    for (auto point : locations) {
        x_center += point.x;
    }
    int y_center = 0;
    for (auto point : locations) {
        y_center += point.y;
    }
    x_center = double(x_center) / double(locations.size());
    y_center = double(y_center) / double(locations.size());
    x_center += rect.x;
    y_center += rect.y;

    return cv::Point2i(x_center, y_center);
}

void Trackdetector::executePyScript(std::string interpreter_path, std::string script_path) {
    std::cout << "executing python script" << std::endl;
    // Interpreter: C:/Users/joshu/anaconda3/envs/KI/python.exe
    // Python script: ../../py/clustering.py
    int s_ret = system((interpreter_path + " " + script_path).c_str());
    if (s_ret == 0) {
        std::cout << "Execution successful." << std::endl;
    }
}

void Trackdetector::visualizeSpline(cv::Mat &dst) {
    for (int i = 1; i < spline_data_proc.size(); i++) {
        cv::line(dst, spline_data_proc.at(i - 1).point, spline_data_proc.at(i).point, cv::Scalar(255), 5);
    }
    std::string windowName = "Spline visualisation";
    cv::namedWindow(windowName, cv::WINDOW_NORMAL);
    cv::resizeWindow(windowName, 300, 666);
    cv::imshow(windowName, dst);
    cv::waitKey(0);
}
