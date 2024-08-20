#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include "video_processor.h"
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <set>

extern "C" {
	#include "vc.h"
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
		return "Resistencia invalida";
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

// Função para desenhar caixa de delimitação, rótulos e centro de massa
void drawBoundingBoxAndLabel(cv::Mat& image, const OVC& blob, const std::vector<std::pair<int, std::string>>& labels, const std::string& valorResistencia) {
	// Desenha retângulo ao redor do blob
	rectangle(image, cv::Point(blob.x, blob.y), cv::Point(blob.x + blob.width, blob.y + blob.height), cv::Scalar(0, 255, 0), 2);

	// Adiciona texto do valor da resistência
	putText(image, valorResistencia, cv::Point(blob.x, blob.y + blob.height + 30), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 255), 2);

	// Desenha o centro de massa
	circle(image, cv::Point(blob.xc, blob.yc), 3, cv::Scalar(255, 255, 255), -1);
}


// Função para identificar blobs e cores
void identificarBlobsCores(IVC* hsvCropImg, std::vector<std::pair<int, std::string>>& cores_encontradas) {
	const int total_pixels = hsvCropImg->width * hsvCropImg->height;
	constexpr int count_Blue = 0;

	for (auto & core : cores) {
		// Cria nova imagem para segmentação
		IVC* segmentedCropImg = vc_image_new(hsvCropImg->width, hsvCropImg->height, 3, hsvCropImg->levels);

		// Realiza segmentação HSV
		vc_hsv_segmentation(hsvCropImg, segmentedCropImg, core.hmin, core.hmax, core.smin, core.smax, core.vmin, core.vmax);

		// Salva imagem segmentada
		char filename[50];
		vc_write_image(filename, segmentedCropImg);

		int cor_presente = 0;
		// Verifica se a cor está presente na imagem segmentada
		for (int y = 0; y < segmentedCropImg->height; y++) {
			for (int x = 0; x < segmentedCropImg->width; x++) {
				const int pos = y * segmentedCropImg->bytesperline + x * segmentedCropImg->channels;
				if (segmentedCropImg->data[pos] == 255) {
					cores_encontradas.emplace_back( x, core.nome );
					cor_presente = 1;
					break;
				}
			}
			if (cor_presente) break;
		}

		vc_image_free(segmentedCropImg);
	}

	if (static_cast<float>(count_Blue) / total_pixels > 0.5) {
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

void displayVideoInfo(const VideoInfo& info) {
	std::cout << "+--------------------------------------------" << std::endl;
    std::cout << "| TOTAL FRAMES: " << info.totalFrames << std::endl;
    std::cout << "| FRAME RATE: " << info.frameRate << " FPS" << std::endl;
    std::cout << "| RESOLUTION: " << info.width << "x" << info.height << std::endl;
    std::cout << "+--------------------------------------------" << std::endl;
}

void processVideo(cv::VideoCapture& cap) {
    VideoInfo info = getVideoInfo(cap);
    std::string outputPath = "../data/samples/output_video.mp4";
    std::string str;
    std::vector<OVC> blob_list;
    std::vector<LabelCor> labelsCores;
    std::set<std::string> resistenciasUnicas;
    std::map<std::string, int> resistenciaMap; // Mapear resistência para número
    cv::VideoWriter writer(outputPath, cv::VideoWriter::fourcc('a', 'v', 'c', '1'), info.frameRate, cv::Size(info.width, info.height));
    cv::Mat frame, frameRGB;
    int framesRead = 0;
    int resistenciaCount = 1;

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

                // Inicializa uma estrutura LabelCor para armazenar as cores encontradas
                LabelCor labelCor;
                labelCor.label = blob.label;

                // Identifica as cores no recorte HSV
                identificarBlobsCores(hsvCropImg, labelCor.cores_encontradas);

                // Se cores forem encontradas, calcula o valor da resistência e desenha os rótulos
                if (!labelCor.cores_encontradas.empty()) {
                    std::string valorResistencia = calcularValorResistencia(labelCor.cores_encontradas);

                    labelsCores.push_back(labelCor);

                    // Verifica se a resistência já foi adicionada
                    if (resistenciasUnicas.find(valorResistencia) == resistenciasUnicas.end()) {
                        // Adiciona a resistência ao conjunto de resistências únicas
                        resistenciasUnicas.insert(valorResistencia);

                        // Mapeia a resistência para um número
                        resistenciaMap[valorResistencia] = resistenciaCount;

                        // Desenha a bounding box com o número da resistência
                        drawBoundingBoxAndLabel(frame, blob, labelCor.cores_encontradas, "#" + std::to_string(resistenciaCount) + " --> " + valorResistencia);

                        // Incrementa o contador de resistências
                        resistenciaCount++;
                    } else {
                        // Desenha a bounding box usando o número já mapeado
                        drawBoundingBoxAndLabel(frame, blob, labelCor.cores_encontradas, "#" + std::to_string(resistenciaMap[valorResistencia]) + " --> " + valorResistencia);
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
        imshow("VC - TP", frame);
        if (cv::waitKey(10) == 'q') break; // ajuste velocidade do vídeo
    }

    cv::destroyWindow("VC - TP");
    cap.release();

    // Imprime as resistências únicas encontradas
    int index = 1;
    for (const auto& resistencia : resistenciasUnicas) {
        std::cout << "| Resistência #" << index++ << ": " << resistencia << std::endl;
    }
	std::cout << "+--------------------------------------------" << std::endl;


    // Libertar o VideoWriter
    writer.release();
}
