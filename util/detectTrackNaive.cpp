#include "detectTrack.hpp"

std::vector<std::vector<cv::Point>> detectTrack(cv::Mat src)
{
    std::string windowName = "Source image";
    cv::namedWindow(windowName, cv::WINDOW_NORMAL);
    cv::resizeWindow(windowName, 816, 682);
    cv::imshow(windowName, src);
    cv::waitKey();

    /*
    cv::Mat srcLaplace = src.clone(); // Destination image
    cv::Mat kernel;                   // Filter kernel

    int ddepth = -1;                      // Datentyp Ergebnis-Bild = Datentyp Eingangs-Bild
    cv::Point anchor = cv::Point(-1, -1); // relative Position des Filters zum berechneten Pixel.
    double delta = 0;                     // Grauwert-Offset (addiert zum Ergebnis-Bild)
    int borderType = cv::BORDER_CONSTANT; // Wie soll am Rand verfahren werden

    // Define second derivative filter in x and y direction with dimensions 3x3
    kernel = (cv::Mat_<float>(3, 3) << 0, 1, 0, 1, -4, 1, 0, 1, 0);

    cv::Mat img32 = src.clone();
    src.convertTo(img32, CV_32FC3, 1 / 255.0);
    cv::filter2D(img32, srcLaplace, ddepth, kernel, anchor, delta, cv::BORDER_DEFAULT);
    // Display filtered image
    windowName = "Laplace filtered";
    cv::namedWindow(windowName, cv::WINDOW_NORMAL);
    cv::resizeWindow(windowName, 800, 600);
    cv::imshow(windowName, srcLaplace);
    */

    cv::Mat blurredImg = src.clone();
    cv::bilateralFilter(src, blurredImg, 3, 200, 200);
    // cv::GaussianBlur(src, blurredImg, cv::Size(cv::Point2i(5, 5)), 3, 3);
    windowName = "Blurred image";
    cv::namedWindow(windowName, cv::WINDOW_NORMAL);
    cv::resizeWindow(windowName, 816, 682);
    cv::imshow(windowName, blurredImg);

    cv::Mat srcGrey = blurredImg.clone();
    cv::cvtColor(blurredImg, srcGrey, cv::COLOR_BGR2GRAY);
    windowName = "Grey image";
    cv::namedWindow(windowName, cv::WINDOW_NORMAL);
    cv::resizeWindow(windowName, 816, 682);
    cv::imshow(windowName, srcGrey);

    srcGrey = 255 - srcGrey;
    windowName = "Grey image negated";
    cv::namedWindow(windowName, cv::WINDOW_NORMAL);
    cv::resizeWindow(windowName, 816, 682);
    cv::imshow(windowName, srcGrey);

    cv::Mat canny = srcGrey.clone();
    cv::Canny(srcGrey, canny, 150, 255, 3, false);
    windowName = "Canny image";
    cv::namedWindow(windowName, cv::WINDOW_NORMAL);
    cv::resizeWindow(windowName, 816, 682);
    cv::imshow(windowName, canny);

    // §cv::blur(srcGrey, srcLaplace, cv::Size(3, 3));

    /*
    cv::Mat srcGrey8 = src.clone();
    canny.convertTo(srcGrey8, CV_8UC1, 255);
    windowName = "8Bit Grey";
    cv::namedWindow(windowName, cv::WINDOW_NORMAL);
    cv::resizeWindow(windowName, 816, 682);
    cv::imshow(windowName, srcGrey8);

    cv::Mat binaryImg = src.clone();
    cv::threshold(srcGrey8, binaryImg, 10, 255, cv::THRESH_BINARY);
    // cv::threshold(srcGrey8, binaryImg, 0, 255, cv::THRESH_BINARY + cv::THRESH_OTSU);


    windowName = "Binary image";
    cv::namedWindow(windowName, cv::WINDOW_NORMAL);
    cv::resizeWindow(windowName, 816, 682);
    cv::imshow(windowName, binaryImg);
    */

    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;

    findContours(canny, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    cv::Mat drawing = cv::Mat::zeros(canny.size(), CV_8UC3);

    std::vector<std::vector<cv::Point>> biggestContours2 = {{contours.at(1)}, {contours.at(2)}};
    std::cout << biggestContours2.at(0).size() << std::endl;

    for (std::vector<cv::Point> contour : contours)
    {
        if (biggestContours2.at(1).size() > biggestContours2.at(0).size())
        {
            biggestContours2 = {biggestContours2.at(1), biggestContours2.at(0)};
        }
        // drawContours(drawing, std::vector<std::vector<cv::Point>>(1, contour), -1, cv::Scalar(0, 0, 255), 1, 8);
        if (contour.size() > biggestContours2.at(1).size())
        {
            biggestContours2.at(1) = contour;
        }
    }

    // drawContours(drawing, biggestContours2, -1, cv::Scalar(0, 0, 255), 2, cv::LINE_8);
    cv::RNG rng(12345);
    for (size_t i = 0; i < biggestContours2.size(); i++)
    {
        cv::Scalar color = cv::Scalar(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256));
        drawContours(drawing, biggestContours2, (int)i, color, 2, cv::LINE_8);
        drawContours(src, biggestContours2, (int)i, color, 2, cv::LINE_8);
    }

    windowName = "Contours";
    cv::namedWindow(windowName, cv::WINDOW_NORMAL);
    cv::resizeWindow(windowName, 816, 682);
    cv::imshow(windowName, drawing);

    windowName = "Contours on rgb img";
    cv::namedWindow(windowName, cv::WINDOW_NORMAL);
    cv::resizeWindow(windowName, 816, 682);
    cv::imshow(windowName, src);
    cv::waitKey(0);

    return biggestContours2;
}
