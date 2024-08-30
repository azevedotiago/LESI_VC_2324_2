#include "utility.h"

extern "C" {
    #include "vc.h"
}

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

/**
 * @brief Função para desenhar o texto de informação no vídeo
 *
 * @param frame frame
 * @param info informação do vídeo
 * @param framesRead frames lidos
 */
void drawInfoText(cv::Mat& frame, const VideoInfo& info, int framesRead) {
    // Definir cores
    cv::Scalar colorTitle(0, 0, 198);            // TÍTULOS: Vermelho Escuro
    cv::Scalar colorValue(0, 0, 0);              // VALORES: Preto
    cv::Scalar colorSeparator(255, 255, 255);    // SEPARADORES: Branco

    // Definir o texto
    std::string textFrames = "Frames lidos: ";
    std::string textFramesValue = std::to_string(framesRead) + "/" + std::to_string(info.totalFrames);
    std::string textFPS = "FPS";
    std::string textFPSValue = std::to_string(static_cast<int>(info.frameRate));
    std::string textResolution = "Resolucao: ";
    std::string textResolutionValue = std::to_string(info.width) + "x" + std::to_string(info.height);

    // Posição inicial do texto
    int baseLine = 0;
    cv::Size textSize = getTextSize(textFrames + textFramesValue + " | " + textFPSValue + " " + textFPS + " | " + textResolution + textResolutionValue, cv::FONT_HERSHEY_SIMPLEX, 0.7, 1, &baseLine);
    cv::Point textOrg((frame.cols - textSize.width) / 2, frame.rows - 10);

    // Desenhar cada parte do texto com a cor correspondente e espessura adequada
    putText(frame, textFrames, textOrg, cv::FONT_HERSHEY_SIMPLEX, 0.7, colorTitle, 2);
    textOrg.x += getTextSize(textFrames, cv::FONT_HERSHEY_SIMPLEX, 0.7, 2, &baseLine).width;

    putText(frame, textFramesValue, textOrg, cv::FONT_HERSHEY_SIMPLEX, 0.7, colorValue, 1);
    textOrg.x += getTextSize(textFramesValue, cv::FONT_HERSHEY_SIMPLEX, 0.7, 1, &baseLine).width;

    putText(frame, " | ", textOrg, cv::FONT_HERSHEY_SIMPLEX, 0.7, colorSeparator, 2);
    textOrg.x += getTextSize(" | ", cv::FONT_HERSHEY_SIMPLEX, 0.7, 2, &baseLine).width;

    putText(frame, textFPSValue, textOrg, cv::FONT_HERSHEY_SIMPLEX, 0.7, colorValue, 1);
    textOrg.x += getTextSize(textFPSValue, cv::FONT_HERSHEY_SIMPLEX, 0.7, 1, &baseLine).width;

    putText(frame, " ", textOrg, cv::FONT_HERSHEY_SIMPLEX, 0.7, colorSeparator, 2);
    textOrg.x += getTextSize(" ", cv::FONT_HERSHEY_SIMPLEX, 0.7, 2, &baseLine).width;

    putText(frame, textFPS, textOrg, cv::FONT_HERSHEY_SIMPLEX, 0.7, colorTitle, 2);
    textOrg.x += getTextSize(textFPS, cv::FONT_HERSHEY_SIMPLEX, 0.7, 2, &baseLine).width;

    putText(frame, " | ", textOrg, cv::FONT_HERSHEY_SIMPLEX, 0.7, colorSeparator, 2);
    textOrg.x += getTextSize(" | ", cv::FONT_HERSHEY_SIMPLEX, 0.7, 2, &baseLine).width;

    putText(frame, textResolution, textOrg, cv::FONT_HERSHEY_SIMPLEX, 0.7, colorTitle, 2);
    textOrg.x += getTextSize(textResolution, cv::FONT_HERSHEY_SIMPLEX, 0.7, 2, &baseLine).width;

    putText(frame, textResolutionValue, textOrg, cv::FONT_HERSHEY_SIMPLEX, 0.7, colorValue, 1);
}


/**
 * @brief Estrutura para armazenar cores em HSV com valores mínimos e máximos
 *
 * @param name nome da cor
 * @param hMin H minimo
 * @param hMax H maximo
 * @param sMin S minimo
 * @param sMax S maximo
 * @param vMin V minimo
 * @param vMax V maximo
 * @return Color
 */
Color init_color(const char* name,
             const int hMin,
             const int hMax,
             const int sMin,
             const int sMax,
             const int vMin,
             const int vMax)
{
    Color color;
    strncpy(color.name, name, COLOR_NAME_MAX - 1);
    color.name[COLOR_NAME_MAX - 1] = '\0';
    color.hMin = hMin;
    color.hMax = hMax;
    color.sMin = sMin;
    color.sMax = sMax;
    color.vMin = vMin;
    color.vMax = vMax;
    return color;
}
