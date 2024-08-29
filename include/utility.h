#ifndef UTILITY_H
#define UTILITY_H
#define RESISTOR_TOLERANCE 5 // Tolerância de 5% - Valor fixado no enunciado

#include <opencv2/opencv.hpp>

struct VideoInfo {
    int totalFrames;
    double frameRate;
    int width;
    int height;
    int currentFrame;
};

VideoInfo getVideoInfo(const cv::VideoCapture& cap);
void displayVideoInfo(const VideoInfo& info);

#endif //UTILITY_H
