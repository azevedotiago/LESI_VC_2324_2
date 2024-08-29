#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include "video_processor.h"
#include "utility.h"
#include <string>

int main(int argc, char** argv) {
	const std::string videoPath = "../data/video_resistors.mp4";
	cv::VideoCapture cap(videoPath);

	// Verificar se o vídeo foi aberto corretamente
	if (!cap.isOpened()) {
		std::cerr << "Erro ao abrir o vídeo." << std::endl;
		return -1;
	}

	// Obter e exibir informações do vídeo
	const VideoInfo info = getVideoInfo(cap);
	displayVideoInfo(info);

	// Processar o vídeo
	processVideo(cap);

	return 0;
}