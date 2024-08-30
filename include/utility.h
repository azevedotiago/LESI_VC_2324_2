#ifndef UTILITY_H
#define UTILITY_H
#define RESISTOR_TOLERANCE 5 // Tolerância de 5% - Valor fixado no enunciado
#define COLOR_NAME_MAX 20

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

const std::string videoPath = "../data/video_resistors.mp4";
const std::string outputPath = "../data/output_video.mp4";

struct VideoInfo {
    int totalFrames;
    double frameRate;
    int width;
    int height;
    int currentFrame;
};

// Estrutura para armazenar etiquetas de cor e posições
struct LabelColor {
    int label{};
    std::vector<std::pair<int, std::string>> foundColors; // Armazena posição x e cor
};

VideoInfo getVideoInfo(const cv::VideoCapture& cap);
void displayVideoInfo(const VideoInfo& info);
void drawInfoText(cv::Mat& frame, const VideoInfo& info, int framesRead);

#endif //UTILITY_H
