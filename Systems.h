#pragma once

//System headers
#include <iostream>
#include <fstream>
#include <thread>
#include <opencv2/opencv.hpp>
//#include <filesystem>


//#include <vector>
//#include <mutex>
//#include <atomic>


//ImGUI
//#include "imgui.h"
//#include "../imgui_sdl.h"

//SDL + OpenGL
//#include <glad/glad.h>
//#include <SDL.h>
//#include <SDL_ttf.h>

class iRect {
public:
    int x, y;
    iRect(int x, int y): x(x), y(y) {};
    iRect(): x(0), y(0) {};
};

using frame_type = cv::UMat;
//using oPoint = cv::Point2i;
using oPoint = iRect;