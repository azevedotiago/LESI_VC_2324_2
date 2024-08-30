#include "image_processing.h"
#include <opencv2/opencv.hpp>


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