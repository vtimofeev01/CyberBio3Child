#pragma once

#include <iostream>
#include <fstream>
#include <thread>
#include <opencv2/opencv.hpp>

class iRect {
public:
    int x, y;
    iRect(int x, int y): x(x), y(y) {};
    iRect(): x(0), y(0) {};
};

using frame_type = cv::UMat;
//using oPoint = cv::Point2i;
using oPoint = iRect;