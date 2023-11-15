#include "detectTrack.hpp"

int rotRectArea(cv::RotatedRect rRect) {
    cv::Point2f vertices[4];
    rRect.points(vertices);

    double l = cv::norm(vertices[0] - vertices[1]);
    double b = cv::norm(vertices[1] - vertices[2]);

    return int(l * b + 0.5);
}

void drawRotRect(cv::Mat &dst, cv::RotatedRect rRect) {
    cv::Point2f vertices[4];
    rRect.points(vertices);
    for (int i = 0; i < 4; i++) {
        cv::line(dst, vertices[i], vertices[(i + 1) % 4], cv::Scalar(255), 2);
    }
}

void executePyScript() {
    std::cout << "executing python script" << std::endl;
    int s_ret = system("C:/Users/joshu/anaconda3/envs/KI/python.exe ../../py/trackInterpolation.py");

    if (s_ret == 0) {
        std::cout << "Execution successful." << std::endl;
    }
}

std::vector<splineData> getSplineData() {
    std::vector<splineData> spline_data;
    splineData data_point;
    std::string line, value;
    int colNum = 0;

    std::fstream data("../../prop/splineData.csv", std::ios::in);
    if (data.is_open()) {
        while (getline(data, line)) {
            std::stringstream str(line);
            colNum = 0;
            while (getline(str, value, ',')) {
                switch (colNum) {
                    case 0:
                        data_point.point.x = std::stoi(value);
                        break;
                    case 1:
                        data_point.point.y = std::stoi(value);
                        break;
                    case 2:
                        data_point.slope = std::stoi(value);
                        break;
                    default:
                        std::cout << "Sacre bleu!" << std::endl;
                }
                colNum++;
            }
            spline_data.push_back(data_point);
        }
    } else {
        std::cout << "Could not open the file" << std::endl;
    }
    std::cout << "first data point("
              << "x: " << spline_data.at(0).point.x
              << ", y: " << spline_data.at(0).point.y
              << ", slope: " << spline_data.at(0).slope << ")"
              << std::endl;

    return spline_data;
}

void drawSpline(cv::Mat &img, std::vector<splineData> spline_data) {
    for (int i = 1; i < spline_data.size(); i++) {
        cv::line(img, spline_data.at(i - 1).point, spline_data.at(i).point, cv::Scalar(255), 5);
    }
}

void save2csv(std::vector<cv::Point> dataPoints) {
    remove("../../prop/dataPoints.csv");

    std::ofstream file;
    file.open("../../prop/dataPoints.csv");

    for (auto point : dataPoints) {
        file << point.x << "," << point.y << std::endl;
    }
    file.close();
}

std::vector<cv::Point> getTrackPoints(std::vector<cv::Mat> images) {
    cv::Mat demoVid = images.at(0);

    cv::Mat diffFrame;
    cv::Mat lastFrame = images.at(0);
    cv::Mat binaryDiffImg;
    cv::Mat displayImg = images.at(0).clone();
    cv::Rect boundingBox;
    cv::RotatedRect rotBox;
    std::vector<cv::Point> trackPoints;

    std::string windowName = "Images";
    cv::namedWindow(windowName, cv::WINDOW_NORMAL);
    cv::resizeWindow(windowName, 300, 666);

    std::cout << "Size of images: " << images.size() << std::endl;

    // Video writer
    int codec = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');  // select desired codec (must be available at runtime)
    double fps = 20.0;                                        // framerate of the created video stream
    std::string filename = "../../out/detecTrackbBox.avi";    // name of the output video file
    cv::Size size(992, 2000);
    cv::VideoWriter writer(filename, codec, fps, size, true);

    for (auto i = 1; i < images.size(); i++) {
        diffFrame = images.at(i) - lastFrame;
        lastFrame = images.at(i).clone();
        demoVid = images.at(i).clone();

        cv::threshold(diffFrame, binaryDiffImg, 30, 255, cv::THRESH_BINARY);

        boundingBox = cv::boundingRect(binaryDiffImg);
        std::vector<cv::Point> nonZeros;
        cv::findNonZero(binaryDiffImg, nonZeros);

        if (nonZeros.size() != 0) {
            rotBox = cv::minAreaRect(nonZeros);

            if (rotRectArea(rotBox) > 500 && rotRectArea(rotBox) < 50000) {
                drawRotRect(displayImg, rotBox);
                cv::Point center = rotBox.center;
                trackPoints.push_back(center);

                // Demo Video
                // drawRotRect(demoVid, rotBox);
                cv::rectangle(demoVid, boundingBox, cv::Scalar(255), 3);

                // std::cout << "Distance: " << cv::norm(trackPoints.at(0) - center) << std::endl;
            }
        }

        cv::Mat color;
        cv::cvtColor(demoVid, color, cv::COLOR_GRAY2BGR);
        writer.write(color);

        cv::imshow(windowName, images.at(i));

        char c = cv::waitKey(1000 / 30);
        if (c == 27) {
            break;
        }
    }
    windowName = "Boxes";
    cv::namedWindow(windowName, cv::WINDOW_NORMAL);
    cv::resizeWindow(windowName, 300, 666);
    cv::imshow(windowName, displayImg);
    cv::waitKey(0);

    return trackPoints;
}

cv::Mat detectTrack(HIDS &hcam, Camera &cam, SerialSTM32 &ser, std::string configPath) {
    const int nSamples = 200;

    std::string saveImgPath = "../../out/Mask.png";
    std::string windowName;

    cv::Mat frame;
    cv::Mat trackMat = cv::Mat::zeros(cv::Size(cv::Point(992, 2000)), CV_8UC1);
    std::vector<cv::Mat> trackImages;
    std::vector<cv::Point> trackPoints;

    try {
        cam.ConfigureCam(configPath);

        auto mem = MemHandler(&hcam);

        mem.allocRingBuffer();
        cam.SetCaptureMode(CONTINOUS_FREERUN);

        std::cout << "Size of track images :" << trackImages.size() << std::endl;

        ser.WriteSerialPort(10);

        int lastIdx = 0;
        int currentIdx = 0;

        while (trackImages.size() < nSamples) {
            currentIdx = mem.getActiveBufferID();

            if (lastIdx != currentIdx) {
                lastIdx = currentIdx;
                frame = mem.getOpenCVMatRingImg();
                trackImages.emplace_back(frame.clone());
            }
        }

        ser.WriteSerialPort(0);
        std::cout << "Size of track images :" << trackImages.size() << std::endl;

        trackPoints = getTrackPoints(trackImages);
        save2csv(trackPoints);

        executePyScript();
        auto spline_data = getSplineData();

        cv::Mat displayImg = trackImages.at(0).clone();

        drawSpline(displayImg, spline_data);

        windowName = "Mask";
        cv::namedWindow(windowName, cv::WINDOW_NORMAL);
        cv::resizeWindow(windowName, 300, 666);
        cv::imshow(windowName, displayImg);
        cv::waitKey(0);

        cv::destroyAllWindows();

    } catch (std::exception &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
    }
    return trackMat;
}