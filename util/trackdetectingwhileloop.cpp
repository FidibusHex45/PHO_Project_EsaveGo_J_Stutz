
diffFrame = frame - lastFrame;
lastFrame = frame.clone();
cv::threshold(diffFrame, binaryDiffImg, 50, 255, cv::THRESH_BINARY);
boundingBox = cv::boundingRect(binaryDiffImg);
if (boundingBox.area() > 1000 && boundingBox.area() < 50000) {
    cv::rectangle(frame, boundingBox, cv::Scalar(255), 3);
    cv::Point center = (boundingBox.br() + boundingBox.tl()) * 0.5;
    track.push_back(center);
}

cv::imshow(windowName, frame);
// cv::imshow(windowName1, binaryFrame);