#include <iostream>
#include <opencv2/opencv.hpp>
#include "../include/vc.h"

int main(int argc, char** argv) {
    // Inicia a captura de vídeo
    cv::VideoCapture cap("../data/video_resistors.mp4");

    // Verifica se o vídeo foi carregado corretamente
    if (!cap.isOpened()) {
        std::cerr << "Erro ao abrir o vídeo!" << std::endl;
        return -1;
    }

    // Janela para exibir o vídeo
    cv::namedWindow("Video", cv::WINDOW_AUTOSIZE);

    cv::Mat frame;
    while (true) {
        // Captura cada frame
        cap >> frame;

        // Se o frame estiver vazio, significa que o vídeo terminou
        if (frame.empty()) {
            break;
        }

        // Exibe o frame capturado
        cv::imshow("Video", frame);

        // Aguarda 30ms antes de exibir o próximo frame
        if (cv::waitKey(30) >= 0) {
            break; // Interrompe se uma tecla for pressionada
        }
    }

    // Liberta a captura de vídeo e fecha as janelas
    cap.release();
    cv::destroyAllWindows();

    return 0;
}
