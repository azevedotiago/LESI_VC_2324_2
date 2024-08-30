#include "video_processor.h"
#include "utility.h"

int main() {
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