#include "detectTrack.hpp"

TrackHandler::TrackHandler(MemHandler *hmem, Camera *camera, SerialSTM32 *serial)
    : mem(hmem), cam(camera), ser(serial) {
}

trackData TrackHandler::getTrackData(std::string load_path, std::string save_path) {
    while (true) {
        std::cout << "\n\n";
        std::cout << "Do you want to LOAD or SCAN a configuration? [l/s]: ";
        std::string input;
        std::cin >> input;
        if (input == "l") {
            loadTrack(load_path);
            break;
        } else if (input == "s") {
            scanTrack(load_path, save_path);
            break;
        } else {
            std::cout << "Invalid input, try again!" << std::endl;
        }
    }
    return track_Data;
}

void TrackHandler::scanTrack(std::string load_path, std::string save_path) {
    recordSamples(300);
    evaluateSampleMean();
    evaluateSamples(30);
    hcsv.save2csv(save_path, trackPoints);
    executePyScript("C:/Users/joshu/anaconda3/python.exe", "../../py/app.py");
    spline_data_proc = hcsv.loadSplineData(load_path, 20, 90);
    track_Data.data = spline_data_proc;
    track_Data.mean_img = sampleMean.clone();
    track_Data.mask = evaluateMask();
    visualizeSpline(track_Data.mean_img.clone());
    saveConfiguration();
}

void TrackHandler::loadTrack(std::string load_path) {
    loadConfiguration();
    calibrateLoadedConfig();
    visualizeSpline(track_Data.mean_img.clone());
}

void TrackHandler::calibrateLoadedConfig() {
    std::cout << "Calibration upcoming, make sure no objects are on the track | press any Key to calibrate.";

    std::fflush(stdout);
    int in = getch();
    std::cout << in << "\n";

    cv::Mat unCallibMean;
    cv::Mat callibImg;
    cv::Mat callibMean_32F;

    unCallibMean = track_Data.mean_img.clone();

    cv::Mat unCallibMean_32F;
    unCallibMean.convertTo(unCallibMean_32F, CV_32F, 1. / 255);

    cam->startAqusition(CONTINOUS_FREERUN);
    callibImg = mem->getOpenCVMatRingImg();
    cam->stopAqusition();

    cv::Mat callibImg_32F;
    callibImg.convertTo(callibImg_32F, CV_32F, 1. / 255);

    cv::Mat gain = cv::Mat::zeros(callibImg_32F.size(), CV_32FC1);

    cv::divide(callibImg_32F, unCallibMean_32F, gain);
    cv::multiply(unCallibMean_32F, gain, callibMean_32F);

    cv::Mat callibMean;
    callibMean_32F.convertTo(callibMean, CV_8UC1, 255);
    // callibMean.convertTo(callibMean, CV_8UC1);

    cv::Mat diffCallib;
    cv::absdiff(callibImg, callibMean, diffCallib);
    cv::Mat diffUnCallib;
    cv::absdiff(callibImg, unCallibMean, diffUnCallib);
    track_Data.mean_img = callibMean.clone();

    std::string windowName = "Uncallibrated mean.";
    cv::namedWindow(windowName, cv::WINDOW_NORMAL);
    cv::resizeWindow(windowName, 300, 666);
    cv::imshow(windowName, track_Data.mean_img);

    windowName = "Callibrated mean.";
    cv::namedWindow(windowName, cv::WINDOW_NORMAL);
    cv::resizeWindow(windowName, 300, 666);
    cv::imshow(windowName, callibMean);

    windowName = "Uncallibrated diff";
    cv::namedWindow(windowName, cv::WINDOW_NORMAL);
    cv::resizeWindow(windowName, 300, 666);
    cv::imshow(windowName, diffUnCallib);

    windowName = "Verification img (Should be black).";
    cv::namedWindow(windowName, cv::WINDOW_NORMAL);
    cv::resizeWindow(windowName, 300, 666);
    cv::imshow(windowName, diffCallib);

    cv::waitKey(0);
    cv::destroyAllWindows();
}

void TrackHandler::recordSamples(int nSamples) {
    cv::Mat frame;
    int lastIdx = 0;
    int currentIdx = 0;

    cam->startAqusition(CONTINOUS_FREERUN);
    ser->WriteSerialPort(30);
    while (trackSamples.size() < nSamples) {
        currentIdx = mem->getActiveBufferID();

        if (lastIdx != currentIdx) {
            lastIdx = currentIdx;
            frame = mem->getOpenCVMatRingImg();
            trackSamples.emplace_back(frame.clone());
            // std::cout << trackSamples.size() << std::endl;
        }
    }
    ser->WriteSerialPort(0);
    cam->stopAqusition();
}

void TrackHandler::evaluateSamples(int thres) {
    // cv::Mat demoVid = images.at(0).clone();
    cv::Mat displayImg = sampleMean.clone();
    cv::Mat lastImg = sampleMean.clone();
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
    std::string windowName = "diff Image";
    cv::namedWindow(windowName, cv::WINDOW_NORMAL);
    cv::resizeWindow(windowName, 300, 666);

    for (auto i = 0; i < trackSamples.size(); i++) {
        cv::absdiff(trackSamples.at(i), lastImg, diffFrame);
        lastImg = trackSamples.at(i).clone();
        // demoVid = images.at(i).clone();
        // threshold default = 30
        cv::threshold(diffFrame, binaryDiffImg, thres, 255, cv::THRESH_BINARY);

        boundingBox = cv::boundingRect(binaryDiffImg);
        std::vector<cv::Point> nonZeros;
        cv::findNonZero(binaryDiffImg, nonZeros);

        if (nonZeros.size() != 0) {
            if (boundingBox.area() > 500 && boundingBox.area() < 50000) {
                // cv::rectangle(displayImg, boundingBox, cv::Scalar(255), 3);
                cv::Point center = centerPixelCloud(binaryDiffImg, boundingBox);
                trackPoints.push_back(center);
                cv::circle(displayImg, center, 10, cv::Scalar(255), -1);
                // Demo Video
                // ***********************************************************
                // cv::rectangle(demoVid, boundingBox, cv::Scalar(255), 3);
                // ***********************************************************
                // std::cout << "Distance: " << cv::norm(trackPoints.at(0) - center) << std::endl;
            }
        }
        // Demo Video
        // ***********************************************************
        // cv::Mat color;
        // cv::cvtColor(demoVid, color, cv::COLOR_GRAY2BGR);
        // writer.write(color);
        // ***********************************************************
        cv::imshow(windowName, binaryDiffImg);
        cv::waitKey(1000 / 60);
    }
    windowName = "Points";
    cv::namedWindow(windowName, cv::WINDOW_NORMAL);
    cv::resizeWindow(windowName, 300, 666);
    cv::imshow(windowName, displayImg);
    cv::waitKey(0);
    cv::destroyAllWindows();
}

void TrackHandler::evaluateSampleMean() {
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

cv::Mat TrackHandler::evaluateMask() {
    cv::Mat mask = cv::Mat::zeros(sampleMean.size(), CV_8UC1);
    for (int i = 1; i < spline_data_proc.size(); i++) {
        cv::line(mask, spline_data_proc.at(i - 1).point, spline_data_proc.at(i).point, cv::Scalar(255), 10);
    }
    return mask;
}

cv::Point2i TrackHandler::centerPixelCloud(cv::Mat img, cv::Rect rect) {
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

void TrackHandler::executePyScript(std::string interpreter_path, std::string script_path) {
    std::cout << "executing python script" << std::endl;
    // Interpreter: C:/Users/joshu/anaconda3/envs/KI/python.exe
    // Python script: ../../py/clustering.py
    int s_ret = system((interpreter_path + " " + script_path).c_str());
    if (s_ret == 0) {
        std::cout << "Execution successful." << std::endl;
    }
}

void TrackHandler::visualizeSpline(cv::Mat &dst) {
    for (int i = 1; i < track_Data.data.size(); i++) {
        cv::line(dst, track_Data.data.at(i - 1).point, track_Data.data.at(i).point, cv::Scalar(255), 10);
    }
    std::string windowName = "Spline visualisation";
    cv::namedWindow(windowName, cv::WINDOW_NORMAL);
    cv::resizeWindow(windowName, 300, 666);
    cv::imshow(windowName, dst);
    cv::waitKey(0);
    cv::destroyWindow(windowName);
}

void TrackHandler::saveConfiguration() {
    std::string dirName;
    while (true) {
        std::cout << "Do you want to save this configuration? [y/n]";
        std::string input;
        std::cin >> input;
        std::cout << "\n";
        if (input == "y") {
            while (true) {
                std::cout << "Enter name of current config (folder name).";
                std::string name;
                std::cin >> name;
                std::cout << "\n";
                dirName = "../../conf/" + name;
                std::error_code err;
                if (!std::filesystem::create_directories(dirName, err)) {
                    if (std::filesystem::exists(dirName)) {
                        // The folder already exists:
                        std::cout << "Configuration already exists!" << std::endl;
                        err.clear();
                    } else {
                        std::cout << "Creating directory FAILED, err: " << err.message() << std::endl;
                    }
                } else {
                    std::cout << "Creating directory SUCCESS." << std::endl;
                    break;
                }
            }
            cv::imwrite(dirName + "/mask.png", track_Data.mask);
            cv::imwrite(dirName + "/mean.png", track_Data.mean_img);
            hcsv.saveSplineData(dirName + "/vel_points.csv", spline_data_proc);
            break;
        } else if (input == "n") {
            break;
        } else {
            std::cout << "Invalid input, try again!" << std::endl;
        }
    }
}

void TrackHandler::loadConfiguration() {
    std::string dirName;
    while (true) {
        std::cout << "Enter name of configuration (folder name): ";
        std::string name;
        std::cin >> name;
        dirName = "../../conf/" + name;
        std::error_code err;
        if (std::filesystem::exists(dirName)) {
            break;
        } else {
            std::cout << "No such configuration found." << std::endl;
        }
    }
    track_Data.mask = cv::imread(dirName + "/mask.png", cv::IMREAD_GRAYSCALE);
    track_Data.mean_img = cv::imread(dirName + "/mean.png", cv::IMREAD_GRAYSCALE);
    track_Data.data = hcsv.loadSplineDataConfig(dirName + "/vel_points.csv");
}
