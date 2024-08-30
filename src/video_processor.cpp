#include "video_processor.h"
#include "image_processing.h"
#include "resistor_detection.h"
#include "utility.h"


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


/**
 * @brief Função para processar o vídeo
 *
 * @param cap captura de vídeo
 */
void processVideo(cv::VideoCapture& cap) {
    VideoInfo info = getVideoInfo(cap);
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
