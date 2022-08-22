#include "Main.h"
#include <ctime>

int main(int argc, char *argv[]){
    srand(time(nullptr));
    int m{0};
    Field field = Field();
    frame_type frame(FieldX * 2 + FieldCellsWidth * FieldCellSize, FieldY * 2 + FieldHeight, CV_8UC3);
    const char* wind_name = "Filed";
    namedWindow( wind_name, cv::WINDOW_AUTOSIZE );
    field.SpawnControlGroup();
    for (;;)
    {
        field.tick(m);
        if (field.frame_number % field.show_frame != 0) continue;
        field.draw(frame);
        imshow(wind_name, frame);
        auto k_ = cv::waitKey(2);
        if (k_ == 27) break;
        else if (k_ == int('n')) field.SpawnControlGroup();
        else if (k_ == int('v')) {field.NextView();}
        else if (k_ == 32) {field.run = !field.run;}
        else if (k_ == int('+')) {field.show_frame *= 2;}
        else if (k_ == int('-')) {field.show_frame =std::max(1, field.show_frame / 2);}
            }
}