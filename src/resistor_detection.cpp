#include "resistor_detection.h"
#include "utility.h"
#include <iostream>
#include <opencv2/opencv.hpp>


/**
 * @brief Funcao para calcular o valor da resistencia com base nas cores encontradas
 *
 * @param foundColors vetor de cores encontradas
 * @return std::string
 */
std::string calculateResistorValue(const std::vector<std::pair<int, std::string>>& foundColors) {
    // Verifica se foram encontradas pelo menos 3 cores
    if (foundColors.size() < 3) {
        return "Resistencia invalida";
    }

    // Mapeamento das cores para valores numéricos
    std::map<std::string, int> colorToValue = {
        {"Black", 0}, {"Brown", 1}, {"Red", 2} ,{"Orange", 3}, {"Yellow", 4},
        {"Green", 5}, {"Blue", 6}, {"Purple", 7}, {"Gray", 8}, {"White", 9}
    };

    //calcula o valor das resistencias tendo em consideração as 3 primeiras cores > x0
    const int value = colorToValue[foundColors[0].second] * 10 + colorToValue[foundColors[1].second];
    const int multiplier = static_cast<int>(std::pow(10, colorToValue[foundColors[2].second]));
    const int resistor = value * multiplier;

    return std::to_string(resistor) + " Ohm  +-" + std::to_string(RESISTOR_TOLERANCE) + "%";
}