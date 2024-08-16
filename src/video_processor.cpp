#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include "video_processor.h"
#include <string>
#include <chrono>
#include <vector>
#include <map>
#include <cmath>

extern "C" {
#include "../include/vc.h"
}

// Função para inicializar uma estrutura Cor
Cor init_cor(const char* nome,
			 const int hmin,
			 const int hmax,
			 const int smin,
			 const int smax,
			 const int vmin,
			 const int vmax)
{
	Cor cor;
	strncpy(cor.nome, nome, COR_NOME_MAX - 1);
	cor.nome[COR_NOME_MAX - 1] = '\0';
	cor.hmin = hmin;
	cor.hmax = hmax;
	cor.smin = smin;
	cor.smax = smax;
	cor.vmin = vmin;
	cor.vmax = vmax;
	return cor;
}

// Inicialização das cores possíveis nas resistências
Cor cores[] = {
	{"Red",	   0,	 21,	63, 75,  70, 100},
	{"Red",	   353, 359,	54, 63,  71, 77},
	{"Red",	   353, 359,	59, 67,  70, 75},
	{"Green",  102, 108,	29, 35,  37, 42},
	{"Blue",   192, 200,	26, 33,  34, 40},
	{"Black",  0,	 80,	0,	 100, 0,  30},
	{"Brown",  7,	 27,	26, 51,  31, 47},
	{"Orange", 7,	 14,	67, 72,  85, 95},
};

// Número de cores definidas
constexpr int num_cores = sizeof(cores) / sizeof(Cor);

// Estrutura para armazenar etiquetas de cor e posições
struct LabelCor {
	int label{};
	std::vector<std::pair<int, std::string>> cores_encontradas; // Armazena posição x e cor
};

// Função para calcular o valor da resistência com base nas cores encontradas
std::string calcularValorResistencia(const std::vector<std::pair<int, std::string>>& cores_encontradas) {
	if (cores_encontradas.size() < 3) {
		return "Invalid resistor";
	}

	// Mapeamento das cores para valores numéricos
	std::map<std::string, int> corParaValor = {
		{"Black", 0}, {"Brown", 1}, {"Red", 2} ,{"Orange", 3}, {"Yellow", 4},
		{"Green", 5}, {"Blue", 6}, {"Purple", 7}, {"Gray", 8}, {"White", 9}
	};

	//calcula o valor das resistencias tendo em consideração as 3 primeiras cores > x0
	const int valor = corParaValor[cores_encontradas[0].second] * 10 + corParaValor[cores_encontradas[1].second];
	const int multiplicador = pow(10, corParaValor[cores_encontradas[2].second]);
	constexpr int tolerancia = 5; // Tolerância de 5% - Valor fixado no enunciado

	const int resistencia = valor * multiplicador;


	return std::to_string(resistencia) + " Ohm  ~" + std::to_string(tolerancia) + "%";
}


//Conta resistencias
int contaResistencia(const int valor, bool primeiraVez) {
	int anterior = 0;

	if (valor != anterior && primeiraVez == true)
	{
		int nResistencia = 0;
		anterior = valor;
		primeiraVez = false;
		return nResistencia++;
	}

}

// Função para desenhar caixa de delimitação, rótulos e centro de massa
void drawBoundingBoxAndLabel(cv::Mat& image, const OVC& blob, const std::vector<std::pair<int, std::string>>& labels, const std::string& valorResistencia) {
	// Desenha retângulo ao redor do blob
	cv::rectangle(image, cv::Point(blob.x, blob.y), cv::Point(blob.x + blob.width, blob.y + blob.height), cv::Scalar(0, 0, 0), 2);

	// Adiciona texto do valor da resistência
	cv::putText(image, valorResistencia, cv::Point(blob.x, blob.y + blob.height + 15), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 1);

	// Desenha o centro de massa
	cv::circle(image, cv::Point(blob.xc, blob.yc), 3, cv::Scalar(0, 0, 0), -1);

}

// Função para identificar blobs e cores
void identificarBlobsCores(IVC* hsvCropImg, std::vector<std::pair<int, std::string>>& cores_encontradas) {
	int total_pixels = hsvCropImg->width * hsvCropImg->height;
	int count_Blue = 0;

	for (int j = 0; j < num_cores; j++) {
		// Cria nova imagem para segmentação
		IVC* segmentedCropImg = vc_image_new(hsvCropImg->width, hsvCropImg->height, 3, hsvCropImg->levels);

		// Realiza segmentação HSV
		vc_hsv_segmentation(hsvCropImg, segmentedCropImg, cores[j].hmin, cores[j].hmax, cores[j].smin, cores[j].smax, cores[j].vmin, cores[j].vmax);

		// Salva imagem segmentada
		char filename[50];
		//sprintf_s(filename, "crop_segmented_%s.pgm", cores[j].nome);
		vc_write_image(filename, segmentedCropImg);

		int cor_presente = 0;
		// Verifica se a cor está presente na imagem segmentada
		for (int y = 0; y < segmentedCropImg->height; y++) {
			for (int x = 0; x < segmentedCropImg->width; x++) {
				int pos = (y * segmentedCropImg->bytesperline) + (x * segmentedCropImg->channels);
				if (segmentedCropImg->data[pos] == 255) {
					if (strcmp(cores[j].nome, "Blue") == 0) {
						//    count_Blue++;
					}
					cores_encontradas.push_back({ x, cores[j].nome });
					cor_presente = 1;
					break;
				}
			}
			if (cor_presente) break;
		}

		vc_image_free(segmentedCropImg);
	}

	if ((float(count_Blue) / total_pixels) > 0.5) {
		cores_encontradas.clear(); // Se mais de 50% dos pixels são azuis, descarta essa blob
	}

	// Ordena as cores encontradas pela posição x
	if (!cores_encontradas.empty()) {
		std::sort(cores_encontradas.begin(), cores_encontradas.end());
	}
}

VideoInfo getVideoInfo(cv::VideoCapture& cap) {
    VideoInfo info{};
    info.totalFrames = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_COUNT));
    info.frameRate = std::round(cap.get(cv::CAP_PROP_FPS));
    info.width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    info.height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
	info.currentFrame = static_cast<int>(cap.get(cv::CAP_PROP_POS_FRAMES));
    return info;
}

void displayVideoInfo(const VideoInfo& info) {
    std::cout << "Total Frames: " << info.totalFrames << std::endl;
    std::cout << "Frame rate: " << info.frameRate << " FPS" << std::endl;
    std::cout << "Resolucao: " << info.width << "x" << info.height << " pixels" << std::endl;
}

void processVideo(cv::VideoCapture& cap) {
    // Configurar o VideoWriter
    VideoInfo info = getVideoInfo(cap);
    std::string outputPath = "../data/samples/output_video.mp4";
    std::string str;
    cv::VideoWriter writer(outputPath, cv::VideoWriter::fourcc('a', 'v', 'c', '1'), info.frameRate, cv::Size(info.width, info.height));

    int nResistencias = 0;
    std::vector<OVC> blob_list; // Lista para armazenar blobs detectados anteriormente

    if (!writer.isOpened()) {
        std::cerr << "Erro ao abrir o arquivo de saída de vídeo." << std::endl;
        return;
    }

    cv::Mat frame, frameRGB;
    std::vector<LabelCor> labelsCores;

    bool resistenciaCalculada = false;
	int framesRead = 0;

    while (cap.read(frame)) {
        if (frame.empty()) break;
    	framesRead++;

    	// Cria o texto com a informação do vídeo
    	std::string infoText = "Frames lidos: " + std::to_string(framesRead) + "/" + std::to_string(info.totalFrames) +
							   " | " + std::to_string(static_cast<int>(info.frameRate)) + " FPS" +
							   " | " + std::to_string(info.width) + "x" + std::to_string(info.height) + " pixels";

        info.currentFrame = static_cast<int>(cap.get(cv::CAP_PROP_POS_FRAMES));
        // Converte frame de BGR para RGB
        cvtColor(frame, frameRGB, cv::COLOR_BGR2RGB);

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

                // Inicializa uma estrutura LabelCor para armazenar as cores encontradas
                LabelCor labelCor;
                labelCor.label = blob.label;

                // Identifica as cores no recorte HSV
                identificarBlobsCores(hsvCropImg, labelCor.cores_encontradas);

                // Se cores forem encontradas, calcula o valor da resistência e desenha os rótulos
                if (!labelCor.cores_encontradas.empty()) {
                    std::string valorResistencia = calcularValorResistencia(labelCor.cores_encontradas);

                    labelsCores.push_back(labelCor);
                    // desenha a bounding box
                    drawBoundingBoxAndLabel(frame, blob, labelCor.cores_encontradas, valorResistencia);
                }

                vc_image_free(cropImg);
                vc_image_free(hsvCropImg);
            }
        }

    	// Desenhar o texto da informação no centro ao fundo do vídeo
    	int baseline = 0;
    	cv::Size textSize = cv::getTextSize(infoText, cv::FONT_HERSHEY_SIMPLEX, 0.7, 1, &baseline);
    	cv::Point textOrg((frame.cols - textSize.width) / 2, frame.rows - 10);

    	// Altera a escala para 0.7 para negrito e a cor para azul (BGR: 255, 0, 0)
    	putText(frame, infoText, textOrg, cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 0, 0), 2);

        // Exibe o frame processado
        imshow("VC - TP", frame);
        if (cv::waitKey(10) == 'q') break; // ajuste velocidade do vídeo

    }

    cv::destroyWindow("VC - TP");
    cap.release();

    // Imprime as etiquetas e as cores encontradas
    for (const auto& labelCor : labelsCores) {
        std::cout << "Label " << labelCor.label << " found colors: ";
        for (const auto& cor : labelCor.cores_encontradas) {
            std::cout << cor.second << " ";
        }
        std::cout << std::endl;
    }

    // Libertar o VideoWriter
    writer.release();
}
