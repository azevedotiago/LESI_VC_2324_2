#ifndef IMAGE_PROCESSING_H
#define IMAGE_PROCESSING_H

#include <string>

extern "C" {
    #include "vc.h"
}

void identifyBlobsColors(IVC* hsvCropImg, std::vector<std::pair<int, std::string>>& foundColors);

#endif //IMAGE_PROCESSING_H
