#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include "video_processor.h"
#include "resistor_detection.h"
#include "utility.h"
#include <string>
#include <vector>
#include <map>
#include <set>

extern "C" {
	#include "vc.h"
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

// Inicialização das cores possíveis nas resistências
Color colors[] = {
	{"Red",	   0,	 21,	63, 75,  70, 100},
	{"Red",	   353, 359,	54, 63,  71, 77},
	{"Red",	   353, 359,	59, 67,  70, 75},
	{"Green",  102, 108,	29, 35,  37, 42},
	{"Blue",   192, 200,	26, 33,  34, 40},
	{"Black",  0,	 80,	0,	 100, 0,  30},
	{"Brown",  7,	 27,	26, 51,  31, 47},
	{"Orange", 7,	 14,	67, 72,  85, 95},
// 	{"Yellow", 50,	 60,	40, 50,  80, 100},
// 	{"Purple", 270, 280,	40, 50,  50, 70},
// 	{"Gray",   0,	 360,	0,	 0,   50, 60},
// 	{"White",  0,	 360,	0,	 0,   90, 100}
};

// Estrutura para armazenar etiquetas de cor e posições
struct LabelColor {
	int label{};
	std::vector<std::pair<int, std::string>> foundColors; // Armazena posição x e cor
};


/**
 * @brief Função para desenhar a caixa de delimitação, rótulos e centro de massa
 *
 * @param image imagem
 * @param blob blob
 * @param labels vetor de etiquetas
 * @param resistorValue valor da resistência
 */
void drawBoundingBoxLabelCentroid(cv::Mat& image, const OVC& blob, const std::vector<std::pair<int, std::string>>& labels, const std::string& resistorValue) {
	// Desenha retângulo ao redor do blob
	rectangle(image, cv::Point(blob.x, blob.y), cv::Point(blob.x + blob.width, blob.y + blob.height), cv::Scalar(0, 255, 0), 2);

	// Adiciona texto do valor da resistência
	putText(image, resistorValue, cv::Point(blob.x, blob.y + blob.height + 30), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 255), 2);

	// Desenha o centro de massa
	circle(image, cv::Point(blob.xc, blob.yc), 3, cv::Scalar(255, 255, 255), -1);
}


// Função para identificar blobs e cores
/**
 * @brief Função para identificar as cores presentes nas blobs
 *
 * @param hsvCropImg imagem recortada em HSV
 * @param foundColors vetor de cores encontradas
 */
void identifyBlobsColors(IVC* hsvCropImg, std::vector<std::pair<int, std::string>>& foundColors) {
	const int totalPixels = hsvCropImg->width * hsvCropImg->height;
	constexpr int blueCounter = 0;

	for (auto & core : colors) {
		// Cria uma nova imagem para segmentar
		IVC* segmentedCropImg = vc_image_new(hsvCropImg->width, hsvCropImg->height, 3, hsvCropImg->levels);

		// Segmentação HSV
		vc_hsv_segmentation(hsvCropImg, segmentedCropImg, core.hMin, core.hMax, core.sMin, core.sMax, core.vMin, core.vMax);

		// Guarda a imagem segmentada
		char filename[50];
		vc_write_image(filename, segmentedCropImg);

		bool isColorPresent = false;
		
		// Verifica se a cor está presente na imagem segmentada
		for (int y = 0; y < segmentedCropImg->height; y++) {
			for (int x = 0; x < segmentedCropImg->width; x++) {
				const int pos = y * segmentedCropImg->bytesperline + x * segmentedCropImg->channels;
				if (segmentedCropImg->data[pos] == 255) {
					foundColors.emplace_back( x, core.name );
					isColorPresent = true;
					break;
				}
			}
			if (isColorPresent) break;
		}

		vc_image_free(segmentedCropImg);
	}

	// Se +50% dos pixels são azuis, ignora o blob
	if (static_cast<float>(blueCounter) / static_cast<float>(totalPixels) > 0.5) {
		foundColors.clear();
	}

	// Ordena as cores
	if (!foundColors.empty()) {
		std::sort(foundColors.begin(), foundColors.end());
	}
}

// /**
//  * @brief Função para obter informações do vídeo
//  *
//  * @param cap captura de vídeo
//  * @return VideoInfo
//  */
// VideoInfo getVideoInfo(const cv::VideoCapture& cap) {
//     VideoInfo info{};
//     info.totalFrames = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_COUNT));
//     info.frameRate = std::round(cap.get(cv::CAP_PROP_FPS));
//     info.width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
//     info.height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
// 	info.currentFrame = static_cast<int>(cap.get(cv::CAP_PROP_POS_FRAMES));
//     return info;
// }

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
 * @brief Função para processar o vídeo
 *
 * @param cap captura de vídeo
 */
void processVideo(cv::VideoCapture& cap) {
    VideoInfo info = getVideoInfo(cap);
    std::string outputPath = "../data/output_video.mp4";
    std::string str;
    std::vector<LabelColor> labelsColors;
    std::set<std::string> uniqueResistors;
    std::map<std::string, int> resistorMap; // Mapear resistência para número
    cv::VideoWriter writer(outputPath, cv::VideoWriter::fourcc('a', 'v', 'c', '1'), info.frameRate, cv::Size(info.width, info.height));
    cv::Mat frame, frameRGB;
    int framesRead = 0;
    int resistorCounter = 1;
	const std::vector<std::pair<int, int>> expectedResistorValues = {
		{220, 1},
		{1000, 2},
		{2200, 1},
		{5600, 1},
		{10000, 1}
	};

    if (!writer.isOpened()) {
        std::cerr << "Erro ao abrir o ficheiro de saída de vídeo." << std::endl;
        return;
    }

    while (cap.read(frame)) {
        if (frame.empty()) break;

        framesRead++;
        info.currentFrame = static_cast<int>(cap.get(cv::CAP_PROP_POS_FRAMES));

        cvtColor(frame, frameRGB, cv::COLOR_BGR2RGB); // Frame BGR --> RGB

        // Criação de imagens para processamento
        IVC* unsegmentedImg = vc_image_new(info.width, info.height, 3, 255);
        IVC* segmentedImage = vc_image_new(info.width, info.height, 3, 255);
        IVC* grayImage = vc_image_new(info.width, info.height, 1, 255);
        IVC* erodedImage = vc_image_new(info.width, info.height, 1, 255);
        IVC* originalImage = vc_image_new(info.width, info.height, 3, 255);

        // Copia dados do frame para as imagens IVC
        memcpy(segmentedImage->data, frameRGB.data, info.width * info.height * 3);
        memcpy(unsegmentedImg->data, frameRGB.data, info.width * info.height * 3);
        memcpy(originalImage->data, frameRGB.data, info.width * info.height * 3);

        // RGB --> HSV
        vc_rgb_to_hsv(segmentedImage, unsegmentedImg);

        // Segmentação HSV
        vc_hsv_segmentation(unsegmentedImg, unsegmentedImg, 15, 360, 30, 100, 30, 100);

        // Conversao imagem segmentada --> escala de cinza
        vc_rgb_to_gray(unsegmentedImg, grayImage);

        // Operações morfológicas de fechamento usando OpenCV
        cv::Mat matGrayImage(info.height, info.width, CV_8UC1, grayImage->data);
        cv::Mat matClosed;
        morphologyEx(matGrayImage, matClosed, cv::MORPH_CLOSE, cv::Mat(), cv::Point(-1, -1), 25);

        // Copia os dados de volta para a estrutura IVC
        memcpy(grayImage->data, matClosed.data, info.width * info.height);
        vc_write_image(const_cast<char*>("Close.pgm"), grayImage);

        // Operação de erosão usando OpenCV
        cv::Mat matEroded;
        erode(matClosed, matEroded, cv::Mat(), cv::Point(-1, -1), 5);
        memcpy(erodedImage->data, matEroded.data, info.width * info.height);
        vc_write_image(const_cast<char*>("erode.pgm"), erodedImage);
        int numero;

        // Etiquetamento de blobs na imagem binária
        OVC* nobjetos = vc_binary_blob_labelling(erodedImage, grayImage, &numero);
        vc_binary_blob_info(grayImage, nobjetos, numero);

        // Filtragem de blobs por área
        std::vector<OVC> filteredBlobs;
        for (int i = 0; i < numero; i++) {
            if (nobjetos[i].area > 1600) {
                filteredBlobs.push_back(nobjetos[i]);
            }
        }

        // Processamento dos blobs filtrados
        for (const auto& blob : filteredBlobs) {
            if (blob.area > 1200 && blob.area < 8000) { // exclui o que não é resistência
                int x1 = blob.x;
                int y1 = blob.y;
                int x2 = blob.x + blob.width;
                int y2 = blob.y + blob.height;

                // Recorte da imagem original ao redor do blob
                IVC* cropImg = vc_image_new(x2 - x1, y2 - y1, originalImage->channels, originalImage->levels);
                for (int y = y1; y < y2; y++) {
                    for (int x = x1; x < x2; x++) {
                        int pos_src = (y * originalImage->bytesperline) + (x * originalImage->channels);
                        int pos_dst = ((y - y1) * cropImg->bytesperline) + ((x - x1) * cropImg->channels);
                        for (int c = 0; c < originalImage->channels; c++) {
                            cropImg->data[pos_dst + c] = originalImage->data[pos_src + c];
                        }
                    }
                }

                // Converte o recorte para HSV
                IVC* hsvCropImg = vc_image_new(cropImg->width, cropImg->height, cropImg->channels, cropImg->levels);
                vc_rgb_to_hsv(cropImg, hsvCropImg);

                // Inicializa uma estrutura LabelColor para armazenar as cores encontradas
                LabelColor labelColor;
                labelColor.label = blob.label;

                // Identifica as cores no recorte HSV
                identifyBlobsColors(hsvCropImg, labelColor.foundColors);

                // Se cores forem encontradas, calcula o valor da resistência e desenha os rótulos
                if (!labelColor.foundColors.empty()) {
                    std::string resistorValue = calculateResistorValue(labelColor.foundColors);

                    labelsColors.push_back(labelColor);

                    // Verifica se a resistência já foi adicionada
                    if (uniqueResistors.find(resistorValue) == uniqueResistors.end()) {
                        // Adiciona a resistência ao conjunto de resistências únicas
                        uniqueResistors.insert(resistorValue);

                        // Mapeia a resistência para um número
                        resistorMap[resistorValue] = resistorCounter;

                        // Desenha a bounding box com o número da resistência
                        drawBoundingBoxLabelCentroid(frame, blob, labelColor.foundColors, "[" + std::to_string(resistorCounter) + "] " + resistorValue);

                        resistorCounter++;
                    } else {
                        // Desenha a bounding box usando o número já mapeado
                        drawBoundingBoxLabelCentroid(frame, blob, labelColor.foundColors, "[" + std::to_string(resistorMap[resistorValue]) + "] " + resistorValue);
                    }
                }
                vc_image_free(cropImg);
                vc_image_free(hsvCropImg);
            }
        }

        // Desenhar o texto da informação no centro ao fundo do vídeo
        drawInfoText(frame, info, framesRead);

        // Escreve o frame processado no vídeo de saída
        writer.write(frame);

        // Exibe o frame processado
        imshow("VC - Resistors", frame);
        if (cv::waitKey(10) == 'q') break;
    }

    cv::destroyWindow("VC - Resistors");
    cap.release();

    // Imprime as resistências encontradas
    int index = 1;
	std::cout << "| RESISTÊNCIAS DETETADAS:" << std::endl;
    for (const auto& resistor : uniqueResistors) {
        std::cout << "| --> " << index++ << "º: " << resistor << std::endl;
    }
	std::cout << "+--------------------------------------------" << std::endl;

	// Imprime as etiquetas e as cores encontradas
	std::cout << "| LABELS ANALISADAS:" << std::endl;
	for (const auto& labelColor : labelsColors) {
		std::cout << "| --> #" << labelColor.label << ": ";
		for (const auto& color : labelColor.foundColors) {
			std::cout << color.second << " ";
		}
		std::cout << std::endl;
	}
	std::cout << "+--------------------------------------------" << std::endl;


    // Libertar o VideoWriter
    writer.release();
}
