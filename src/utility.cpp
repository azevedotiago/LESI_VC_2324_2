#include "utility.h"

/**
 * @brief Função para obter informações do vídeo
 *
 * @param cap captura de vídeo
 * @return VideoInfo
 */
VideoInfo getVideoInfo(const cv::VideoCapture& cap) {
    VideoInfo info{};
    info.totalFrames = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_COUNT));
    info.frameRate = std::round(cap.get(cv::CAP_PROP_FPS));
    info.width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    info.height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    info.currentFrame = static_cast<int>(cap.get(cv::CAP_PROP_POS_FRAMES));
    return info;
}


/**
 * @brief Função para exibir informações do vídeo
 *
 * @param info informação do vídeo
 */
void displayVideoInfo(const VideoInfo& info) {
    std::cout << "+--------------------------------------------"	  << std::endl;
    std::cout << "| TOTAL FRAMES: " << info.totalFrames				  << std::endl;
    std::cout << "| FRAME RATE: " << info.frameRate << " FPS"		  << std::endl;
    std::cout << "| RESOLUTION: " << info.width << "x" << info.height << std::endl;
    std::cout << "+--------------------------------------------"	  << std::endl;
    std::cout << "| RESISTENCIAS ESPERADAS:"						  << std::endl;
    std::cout << "| --> 1º: 5600 Ohm  +-5%"							  << std::endl;
    std::cout << "| --> 2º: 220 Ohm  +-5%"							  << std::endl;
    std::cout << "| --> 3º: 1000 Ohm  +-5%"							  << std::endl;
    std::cout << "| --> 4º: 2200 Ohm  +-5%"							  << std::endl;
    std::cout << "| --> 5º: 10000 Ohm  +-5%"						  << std::endl;
    std::cout << "| --> 6º: 1000 Ohm  +-5%"							  << std::endl;
    std::cout << "+--------------------------------------------"	  << std::endl;
}