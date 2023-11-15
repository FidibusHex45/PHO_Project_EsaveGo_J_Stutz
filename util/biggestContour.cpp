std::vector<std::vector<cv::Point>> biggestContour = {{contours.at(10)}};
std::cout << biggestContour.at(0).size() << std::endl;

for (std::vector<cv::Point> contour : contours)
{
    // drawContours(drawing, std::vector<std::vector<cv::Point>>(1, contour), -1, cv::Scalar(0, 0, 255), 1, 8);
    /* if (contour.size() > biggestContour.at(0).size())
    {
        biggestContour.at(0) = contour;
    } */
}

drawContours(drawing, biggestContour, 0, cv::Scalar(0, 0, 255), 2, cv::LINE_8, 0);