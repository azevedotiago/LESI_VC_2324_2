#define CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "../include/vc.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


/**
 * @brief Alocar memoria para uma imagem
 *
 * @param width Largura
 * @param height Altura
 * @param channels Canais
 * @param levels Niveis
 * @return image
 */
IVC* vc_image_new(int width, int height, int channels, int levels)
{
    IVC* image = (IVC*)malloc(sizeof(IVC));

    if (image == NULL)
        return NULL;
    if ((levels <= 0) || (levels > 255))
        return NULL;

    image->width = width;
    image->height = height;
    image->channels = channels;
    image->levels = levels;
    image->bytesperline = image->width * image->channels;
    image->data = (unsigned char*)malloc(image->width * image->height * image->channels * sizeof(char));

    if (image->data == NULL)
    {
        return vc_image_free(image);
    }

    return image;
}


/**
 * @brief Libertar memoria de uma imagem
 *
 * @param image Imagem
 * @return image
 */
IVC* vc_image_free(IVC* image)
{
    if (image != NULL)
    {
        if (image->data != NULL)
        {
            free(image->data);
            image->data = NULL;
        }

        free(image);
        image = NULL;
    }

    return image;
}


/**
 * @brief Conversao de RGB para HSV
 *
 * @param src Imagem de entrada
 * @param dst Imagem de saida
 * @return int
 */
int vc_rgb_to_hsv(const IVC* src, const IVC* dst) {
    const unsigned char * data_src = src->data;
    unsigned char* data_dst = dst->data;
    const int width_src = src->width;
    const int width_dst = dst->width;
    const int height_src = src->height;
    const int height_dst = dst->height;
    const int channels_src = src->channels;
    const int channel_dst = dst->channels;
    int bytesperline_src = width_src * channels_src, bytesperline_dst = width_dst * channel_dst;

    if (width_src <= 0 || height_src <= 0 || data_src == NULL)
        return 0;
    if (channels_src != 3 || channel_dst != 3)
        return 0;
    if (width_src != width_dst || height_src != height_dst)
        return 0;

    for (int y = 0; y < height_src; y++) {
        for (int x = 0; x < width_src; x++) {
            const int pos_src = y * bytesperline_src + x * channels_src;
            const int pos_dst = y * bytesperline_dst + x * channel_dst;
            const float rf = data_src[pos_src];
            const float gf = data_src[pos_src + 1];
            const float bf = data_src[pos_src + 2];
            const float min = MINRGB(rf, gf, bf);
            const float max = MAXRGB(rf, gf, bf);
            float hue = 0;
            float sat = 0;

            if (max > 0) {
                sat = (max - min) / max * 255;

                if (sat > 0) {
                    if (max == rf) {
                        if (gf >= bf) {
                            hue = 60 * (gf - bf) / (max - min);
                        } else {
                            hue = 360 + 60 * (gf - bf) / (max - min);
                        }
                    } else if (max == gf) {
                        hue = 120 + 60 * (bf - rf) / (max - min);
                    } else if (max == bf) {
                        hue = 240 + 60 * (rf - gf) / (max - min);
                    }
                    hue = hue / 360 * 255;
                }
            }
            data_dst[pos_dst] = hue;
            data_dst[pos_dst + 1] = sat;
            data_dst[pos_dst + 2] = max;
        }
    }
    return 1;
}


/**
 * @brief Segmentacao de uma imagem em HSV
 *
 * @param src Imagem de entrada
 * @param dst Imagem de saida
 * @param hMin Valor minimo de matiz - [0, 360]
 * @param hMax Valor maximo de matiz - [0, 360]
 * @param sMin Valor minimo de saturação - [0, 100]
 * @param sMax Valor maximo de saturação - [0, 100]
 * @param vMin Valor minimo de valor - [0, 100]
 * @param vMax Valor maximo de valor - [0, 100]
 * @return int
 */
int vc_hsv_segmentation(const IVC* src, const IVC* dst, int hMin, int hMax, int sMin, int sMax, int vMin, int vMax)
{
    const unsigned char* data = src->data;
    unsigned char* dstdata = dst->data;
    const int width = src->width;
    const int height = src->height;
    src->bytesperline;
    const int channels = src->channels;

    // Verificacao de erros
    if (src->width <= 0 || src->height <= 0 || src->data == NULL)
        return 0;
    if (channels != 3)
        return 0;

    const int size = width * height * channels;

    for (int i = 0; i < size; i = i + channels)
    {
        const int h = (float)data[i] / 255.0f * 360.0f;
        const int s = (float)data[i + 1] / 255.0f * 100.0f;
        const int v = (float)data[i + 2] / 255.0f * 100.0f;

        if (h > hMin && h <= hMax && s >= sMin && s <= sMax && v >= vMin && v <= vMax) {
            dstdata[i] = 255;
            dstdata[i + 1] = 255;
            dstdata[i + 2] = 255;
        } else {
            dstdata[i] = 0;
            dstdata[i + 1] = 0;
            dstdata[i + 2] = 0;
        }
    }
    return 1;
}


/**
 * @brief Conversao de RGB para escala de cinza
 *
 * @param src Imagem de entrada
 * @param dst Imagem de saida
 * @return int
 */
int vc_rgb_to_gray(const IVC* src, const IVC* dst)
{
    unsigned char* datasrc = src->data;
    int bytesperline_src = src->bytesperline;
    const int channels_src = src->channels;
    unsigned char* datadst = dst->data;
    int bytesperline_dst = dst->bytesperline;
    const int channels_dst = dst->channels;
    const int width = src->width;
    const int height = src->height;

    //verificacao de erros
    if (src->width <= 0 || src->height <= 0 || src->data == NULL)
        return 0;
    if (src->width != dst->width || src->height != dst->height)
        return 0;
    if (src->channels != 3 || dst->channels != 1)
        return 0;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            const long int pos_src = y * bytesperline_src + x * channels_src;
            const long int pos_dst = y * bytesperline_dst + x * channels_dst;
            const float rf = datasrc[pos_src];
            const float gf = datasrc[pos_src + 1];
            const float bf = datasrc[pos_src + 2];

            datadst[pos_dst] = (unsigned char)(rf * 0.299 + gf * 0.587 + bf * 0.114);
        }
    }
    return 1;
}


/**
 * @brief Conversao de escala de cinza para binario
 *
 * @param src Imagem de entrada
 * @param dst Imagem de saida
 * @param kernel Tamanho do kernel
 * @return int
 */
int vc_gray_to_binary_midpoint(const IVC* src, const IVC* dst, const int kernel)
{
    unsigned char* datasrc = src->data;
    unsigned char* datadst = dst->data;
    const int width = src->width;
    const int height = src->height;
    int bytesperline = src->bytesperline;
    const int channels = src->channels;
    const int offset = (kernel - 1) / 2;

    // Verificacao de erros
    if (src->width <= 0 || src->height <= 0 || src->data == NULL)
        return 0;
    if (src->width != dst->width || src->height != dst->height || src->channels != dst->channels)
        return 0;
    if (channels != 1)
        return 0;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            const long int pos = y * bytesperline + x * channels;
            int max = 0;
            int min = 255;

            // NxM Vizinhos
            for (int ky = -offset; ky <= offset; ky++) {
                for (int kx = -offset; kx <= offset; kx++) {
                    if (y + ky >= 0 && y + ky < height && x + kx >= 0 && x + kx < width) {
                        long int posk = (y + ky) * bytesperline + (x + kx) * channels;

                        // Calculo do minimo e do maximo a estudar na vizinhanca
                        if (datasrc[posk] > max)
                            max = datasrc[posk];
                        if (datasrc[posk] < min)
                            min = datasrc[posk];
                    }
                }
            }
            //Calculo do midpoint
            const unsigned char threshold = (unsigned char)((float)(max + min) / (float)2);

            if (datasrc[pos] > threshold) {
                datadst[pos] = 255;
            } else {
                datadst[pos] = 0;
            }
        }
    }
    return 1;
}


/**
 * @brief Filtro de media passa-baixa em escala de cinza
 *
 * @param src Imagem de entrada
 * @param dst Imagem de saida
 * @return int
 */
int vc_gray_lowpass_mean_filter(const IVC* src, const IVC* dst)
{
    unsigned char* datasrc = src->data;
    unsigned char* datadst = dst->data;
    const int width = src->width;
    const int height = src->height;
    int bytesperline = src->bytesperline;
    const int channels = src->channels;

    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            const long int posA = (y - 1) * bytesperline + (x - 1) * channels;
            const long int posB = (y - 1) * bytesperline + x * channels;
            const long int posC = (y - 1) * bytesperline + (x + 1) * channels;
            const long int posD = y * bytesperline + (x - 1) * channels;
            const long int posX = y * bytesperline + x * channels;
            const long int posE = y * bytesperline + (x + 1) * channels;
            const long int posF = (y + 1) * bytesperline + (x - 1) * channels;
            const long int posG = (y + 1) * bytesperline + x * channels;
            const long int posH = (y + 1) * bytesperline + (x + 1) * channels;

            int sum = datasrc[posA] * +1;
            sum += datasrc[posB] * +1;
            sum += datasrc[posC] * +1;
            sum += datasrc[posD] * +1;
            sum += datasrc[posX] * +1;
            sum += datasrc[posE] * +1;
            sum += datasrc[posF] * +1;
            sum += datasrc[posG] * +1;
            sum += datasrc[posH] * +1;

            datadst[posX] = (unsigned char)(sum / 9);
        }
    }
    return 1;
}


/**
 * @brief Erosao binaria em escala de cinza
 *
 * @param src Imagem de entrada
 * @param dst Imagem de saida
 * @param kernel Tamanho do kernel
 * @return int
 */
int vc_binary_erode(const IVC* src, const IVC* dst, int kernel)
{
    unsigned char* datasrc = src->data;
    int bytesperline_src = src->width * src->channels;
    const int channels_src = src->channels;
    unsigned char* datadst = dst->data;
    const int width = src->width;
    const int height = src->height;
    const int bytesperline_dst = dst->width * dst->channels;
    const int channels_dst = dst->channels;
    kernel *= 0.5;

    if (src->width <= 0 || src->height <= 0 || src->data == NULL) return 0;
    if (src->width != dst->width || src->height != dst->height) return 0;
    if (src->channels != 1 || dst->channels != 1) return 0;

    for (int y = kernel; y < height - kernel; y++) {
        for (int x = kernel; x < width - kernel; x++) {
            const long int pos_dst = y * bytesperline_dst + x * channels_dst;
            int verifica = 0;

            for (int y2 = y - kernel; y2 <= y + kernel; y2++) {
                for (int x2 = x - kernel; x2 <= x + kernel; x2++) {
                    const long int pos_src = y2 * bytesperline_src + x2 * channels_src;

                    if (datasrc[pos_src] == 0) {
                        verifica = 1;
                    }
                }
            }

            if (verifica == 1) {
                datadst[pos_dst] = 0;
            } else {
                datadst[pos_dst] = 255;
            }
        }
    }
    return 1;
}


/**
 * @brief Etiquetagem de blobs
 *
 * @param src Imagem binaria de entrada
 * @param dst Imagem grayscale (ira conter as etiquetas)
 * @param nlabels Endereco de memoria de uma variavel, onde sera armazenado o numero de etiquetas encontradas.
 * @return OVC*
 */
OVC* vc_binary_blob_labelling(const IVC* src, const IVC* dst, int* nlabels)
{
    unsigned char* datasrc = src->data;
    unsigned char* datadst = dst->data;
    const int width = src->width;
    const int height = src->height;
    int bytesperline = src->bytesperline;
    const int channels = src->channels;
    int x, y, a;
    long int i, size;
    long int posX;
    int labeltable[256] = { 0 };
    int labelarea[256] = { 0 };
    int label = 1; // Etiqueta inicial.
    int tmplabel;

    // Verificacao de erros
    if (src->width <= 0 || src->height <= 0 || src->data == NULL)
        return 0;
    if (src->width != dst->width || src->height != dst->height || src->channels != dst->channels)
        return NULL;
    if (channels != 1)
        return NULL;

    // Copia dados da imagem binaria para imagem grayscale
    memcpy(datadst, datasrc, bytesperline * height);

    // Todos os pixeis de plano de fundo devem obrigatoriamente ter valor 0
    // Todos os pixeis de primeiro plano devem obrigatoriamente ter valor 255
    // Serao atribuidas etiquetas no intervalo [1,254]
    // Este algoritmo esta assim limitado a 255 labels
    for (i = 0, size = bytesperline * height; i < size; i++) {
        if (datadst[i] != 0) datadst[i] = 255;
    }

    // Limpa os rebordos da imagem binaria
    for (y = 0; y < height; y++) {
        datadst[y * bytesperline + 0 * channels] = 0;
        datadst[y * bytesperline + (width - 1) * channels] = 0;
    }

    for (x = 0; x < width; x++) {
        datadst[0 * bytesperline + x * channels] = 0;
        datadst[(height - 1) * bytesperline + x * channels] = 0;
    }

    // Efetua a etiquetagem
    for (y = 1; y < height - 1; y++) {
        for (x = 1; x < width - 1; x++) {
            // Kernel:
            // A B C
            // D X
            const long int posA = (y - 1) * bytesperline + (x - 1) * channels;  // A
            const long int posB = (y - 1) * bytesperline + x * channels;	    // B
            const long int posC = (y - 1) * bytesperline + (x + 1) * channels;  // C
            const long int posD = y * bytesperline + (x - 1) * channels;		// D
            posX = y * bytesperline + x * channels;				                // X

            // Se o pixel foi marcado
            if (datadst[posX] != 0) {
                if (datadst[posA] == 0 && datadst[posB] == 0 && datadst[posC] == 0 && datadst[posD] == 0) {
                    datadst[posX] = label;
                    labeltable[label] = label;
                    label++;
                } else {
                    int num = 255;
                    // Se A esta marcado
                    if (datadst[posA] != 0)
                        num = labeltable[datadst[posA]];
                    // Se B esta marcado, e o menor que a etiqueta "num"
                    if (datadst[posB] != 0 && labeltable[datadst[posB]] < num)
                        num = labeltable[datadst[posB]];
                    // Se C esta marcado, e o menor que a etiqueta "num"
                    if (datadst[posC] != 0 && labeltable[datadst[posC]] < num)
                        num = labeltable[datadst[posC]];
                    // Se D esta marcado, e o menor que a etiqueta "num"
                    if (datadst[posD] != 0 && labeltable[datadst[posD]] < num)
                        num = labeltable[datadst[posD]];

                    // Atribui a etiqueta ao pixel
                    datadst[posX] = num;
                    labeltable[num] = num;

                    // Atualiza a tabela de etiquetas
                    if (datadst[posA] != 0) {
                        if (labeltable[datadst[posA]] != num) {
                            for (tmplabel = labeltable[datadst[posA]], a = 1; a < label; a++) {
                                if (labeltable[a] == tmplabel) labeltable[a] = num;
                            }
                        }
                    }

                    if (datadst[posB] != 0) {
                        if (labeltable[datadst[posB]] != num) {
                            for (tmplabel = labeltable[datadst[posB]], a = 1; a < label; a++) {
                                if (labeltable[a] == tmplabel) labeltable[a] = num;
                            }
                        }
                    }

                    if (datadst[posC] != 0) {
                        if (labeltable[datadst[posC]] != num) {
                            for (tmplabel = labeltable[datadst[posC]], a = 1; a < label; a++) {
                                if (labeltable[a] == tmplabel) labeltable[a] = num;
                            }
                        }
                    }

                    if (datadst[posD] != 0) {
                        if (labeltable[datadst[posD]] != num) {
                            for (tmplabel = labeltable[datadst[posC]], a = 1; a < label; a++) {
                                if (labeltable[a] == tmplabel) labeltable[a] = num;
                            }
                        }
                    }
                }
            }
        }
    }

    // Volta a etiquetar a imagem
    for (y = 1; y < height - 1; y++) {
        for (x = 1; x < width - 1; x++) {
            posX = y * bytesperline + x * channels; // X
            if (datadst[posX] != 0) datadst[posX] = labeltable[datadst[posX]];
        }
    }

    // Contagem do numero de blobs
    // Passo 1: Eliminar, da tabela, etiquetas repetidas
    for (a = 1; a < label - 1; a++) {
        for (int b = a + 1; b < label; b++) {
            if (labeltable[a] == labeltable[b]) labeltable[b] = 0;
        }
    }

    // Passo 2: Conta etiquetas e organiza a tabela de etiquetas, para que nao hajam valores vazios (zero) entre etiquetas
    *nlabels = 0;
    for (a = 1; a < label; a++) {
        if (labeltable[a] != 0) {
            labeltable[*nlabels] = labeltable[a]; // Organiza tabela de etiquetas
            (*nlabels)++;						  // Conta etiquetas
        }
    }

    // Se nao ha blobs
    if (*nlabels == 0) return NULL;

    // Cria lista de blobs (objetos) e preenche a etiqueta
    OVC* blobs = calloc(*nlabels, sizeof(OVC));
    if (blobs != NULL) {
        for (a = 0; a < *nlabels; a++) blobs[a].label = labeltable[a];
    } else {
        return NULL;
    }
    return blobs;
}


/**
 * @brief Informacao de blobs
 *
 * @param src Imagem binaria de entrada
 * @param blobs Lista de blobs
 * @param nblobs Numero de blobs
 * @return int
 */
int vc_binary_blob_info(const IVC* src, OVC* blobs, int nblobs)
{
    const unsigned char* data = src->data;
    const int width = src->width;
    const int height = src->height;
    int bytesperline = src->bytesperline;
    const int channels = src->channels;

    // Verificacao de erros
    if (src->width <= 0 || src->height <= 0 || src->data == NULL) return 0;
    if (channels != 1) return 0;

    // Conta area de cada blob
    for (int i = 0; i < nblobs; i++) {
        int xmin = width - 1;
        int ymin = height - 1;
        int xmax = 0;
        int ymax = 0;
        long int sumx = 0;
        long int sumy = 0;
        blobs[i].area = 0;

        for (int y = 1; y < height - 1; y++) {
            for (int x = 1; x < width - 1; x++) {
                const long int pos = y * bytesperline + x * channels;

                if (data[pos] == blobs[i].label) {
                    blobs[i].area++; // Area

                    // Centro de Gravidade
                    sumx += x;
                    sumy += y;

                    // Bounding Box
                    if (xmin > x) xmin = x;
                    if (ymin > y) ymin = y;
                    if (xmax < x) xmax = x;
                    if (ymax < y) ymax = y;

                    // Perimetro
                    // Se pelo menos um dos quatro vizinhos nao pertence ao mesmo label, entao e um pixel de contorno
                    if (data[pos - 1] != blobs[i].label
                        || data[pos + 1] != blobs[i].label
                        || data[pos - bytesperline] != blobs[i].label
                        || data[pos + bytesperline] != blobs[i].label
                        ) {
                        blobs[i].perimeter++;
                    }
                }
            }
        }

        // Bounding Box
        blobs[i].x = xmin;
        blobs[i].y = ymin;
        blobs[i].width = xmax - xmin + 1;
        blobs[i].height = ymax - ymin + 1;

        // Centro de Gravidade
        blobs[i].xc = sumx / MAX(blobs[i].area, 1);
        blobs[i].yc = sumy / MAX(blobs[i].area, 1);
    }
    return 1;
}


/**
 * @brief Calcula e identifica bounding box
 *
 * @param x Coordenada x
 * @param y Coordenada y
 * @param largura Largura
 * @param altura Altura
 * @param isaida Imagem de saida
 * @return int
 */
int vc_draw_bounding_box(const int x, const int y, int largura, int altura, const IVC* isaida)
{
    unsigned char* datadst = isaida->data;
    int bytesperline_dst = isaida->width * isaida->channels;
    const int channels_dst = isaida->channels;
    int width_dst = isaida->width;
    int height_dst = isaida->height;
    long int pos;

    for (int i = x; i < x + largura; i++) {
        pos = y * bytesperline_dst + i * channels_dst;
        datadst[pos] = 0;
        datadst[pos + 1] = 0;
        datadst[pos + 2] = 0;

        pos = (y + altura) * bytesperline_dst + i * channels_dst;
        datadst[pos] = 0;
        datadst[pos + 1] = 0;
        datadst[pos + 2] = 0;
    }

    for (int i = y; i < y + altura; i++) {
        pos = i * bytesperline_dst + x * channels_dst;
        datadst[pos] = 0;
        datadst[pos + 1] = 0;
        datadst[pos + 2] = 0;

        pos = i * bytesperline_dst + (x + largura) * channels_dst;
        datadst[pos] = 0;
        datadst[pos + 1] = 0;
        datadst[pos + 2] = 0;
    }
    return 1;
}


/**
 * @brief Calcula e identifica centro da imagem
 *
 * @param x Coordenada x
 * @param y Coordenada y
 * @param xc Coordenada x do centro
 * @param yc Coordenada y do centro
 * @param largura Largura
 * @param altura Altura
 * @param isaida Imagem de saida
 * @return int
 */
int vc_center_of_mass(const int x, const int y, const int xc, const int yc, int largura, int altura, const IVC* isaida)
{
    unsigned char* datadst = isaida->data;
    int bytesperline_dst = isaida->width * isaida->channels;
    const int channels_dst = isaida->channels;
    const int width_dst = isaida->width;
    const int height_dst = isaida->height;

    // Limites para evitar estouro de memoria
    for (int i = x; i < x + largura; i++) {
        for (int j = y; j < y + altura; j++) {
            if (xc == i && yc == j) {
                if (i >= 0 && i < width_dst && j >= 0 && j < height_dst) {
                    long int pos = j * bytesperline_dst + i * channels_dst;
                    datadst[pos] = 0;
                    datadst[pos + 1] = 0;
                    datadst[pos + 2] = 0;

                    for (int k = 1; k < 6; k++) {
                        // Verificacao para os limites superiores e inferiores ao centro de massa
                        if (j - k >= 0 && j - k < height_dst) {
                            pos = (j - k) * bytesperline_dst + i * channels_dst;
                            datadst[pos] = 0;
                            datadst[pos + 1] = 0;
                            datadst[pos + 2] = 0;
                        }

                        if (j + k >= 0 && j + k < height_dst) {
                            pos = (j + k) * bytesperline_dst + i * channels_dst;
                            datadst[pos] = 0;
                            datadst[pos + 1] = 0;
                            datadst[pos + 2] = 0;
                        }

                        // Verificacao para os limites a esquerda e a direita do centro de massa
                        if (i + k >= 0 && i + k < width_dst) {
                            pos = j * bytesperline_dst + (i + k) * channels_dst;
                            datadst[pos] = 0;
                            datadst[pos + 1] = 0;
                            datadst[pos + 2] = 0;
                        }

                        if (i - k >= 0 && i - k < width_dst) {
                            pos = j * bytesperline_dst + (i - k) * channels_dst;
                            datadst[pos] = 0;
                            datadst[pos + 1] = 0;
                            datadst[pos + 2] = 0;
                        }
                    }
                }
            }
        }
    }
    return 1;
}


/**
 * @brief Conta pixels brancos da imagem
 *
 * @param image Imagem de entrada
 * @return int
 */
int countWhitePixels(const IVC* image) {
    int count = 0;
    const int bytesPerLine = image->channels * image->width;

    for (int y = 0; y < image->height; y++) {
        for (int x = 0; x < image->width; x++) {
            const long int pos = y * bytesPerLine + x * image->channels;
            if (image->data[pos] == 255) count++;
        }
    }
    return count;
}


/**
 * @brief Verifica a cor predominante da imagem
 *
 * @param image Imagem de entrada
 * @return int
 */
int colors_check(const IVC* image)
{
    const IVC* imageRED = vc_image_new(image->width, image->height, 3, image->levels);
    const IVC* imageBLUE = vc_image_new(image->width, image->height, 3, image->levels);
    int countRed = 0;
    int countBlue = 0;
    int aux = -1;

    vc_hsv_segmentation(image, imageRED, 240, 255, 17, 100, 20, 100);

    countRed = countWhitePixels(imageRED);

    vc_hsv_segmentation(image, imageRED, 0, 15, 17, 100, 20, 100);
    countRed += countWhitePixels(imageRED);
    printf("Count Red: %d\n", countRed);

    vc_hsv_segmentation(image, imageBLUE, 15, 60, 40, 100, 10, 100);

    countBlue = countWhitePixels(imageBLUE);
    printf("Count Blue: %d\n", countBlue);

    if (countRed > countBlue && countRed > 1000) {
        aux = 0;  // Vermelho
    } else if (countBlue > countRed && countBlue > 500) {
        aux = 1;  // Azul
    }

    return aux;
}


/**
 * @brief Calcula a redondeza da imagem
 *
 * @param binaryImage Imagem binaria de entrada
 * @return float
 */
float vc_calculate_roundness(const IVC* binaryImage)
{
    const unsigned char* data = (unsigned char*)binaryImage->data;
    int bytesperline = binaryImage->width * binaryImage->channels;
    const int channels = binaryImage->channels;
    const int width = binaryImage->width;
    const int height = binaryImage->height;
    int area = 0;
    int perimeter = 0;

    // Itera sobre todos os pixeis da imagem
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            const long int pos = y * bytesperline + x * channels;

            // Se o pixel faz parte do objeto (diferente de zero), incrementa a area
            if (data[pos] != 0) {
                area++;

                // Verifica se o pixel faz parte do contorno do objeto
                if (x > 0 && data[pos - channels] == 0
                    || x < width - 1 && data[pos + channels] == 0
                    || y > 0 && data[pos - bytesperline] == 0
                    || y < height - 1 && data[pos + bytesperline] == 0
                    ) {
                    perimeter++;
                }
            }
        }
    }
    if (perimeter == 0) return 0;  // Evita divisao por zero
    const float roundness = 4 * M_PI * area / (float)(perimeter * perimeter);

    return roundness;
}


/**
 * @brief Verifica se a imagem e um circulo
 *
 * @param image Imagem de entrada
 * @return int
 */
int vc_stop_forbiden_distinction(const IVC* image)
{
    const float roundness = vc_calculate_roundness(image);
    int aux = -1;

    printf("Roundness: %f\n", roundness);

    // caso se verifique que a roundness o maior que 0.15 o Sentido Proibido
    if (roundness >= 0.15) {
        aux = 1;
    } else if (roundness < 0.15) { // Se for menor que 0.15, o STOP
        aux = 0;
    }

    return aux;
}


/**
 * @brief Verifica a cor predominante da imagem
 *
 * @param image Imagem de entrada
 * @param xbb Coordenada x da bounding box
 * @param ybb Coordenada y da bounding box
 * @param widthbb Largura da bounding box
 * @param heightbb Altura da bounding box
 * @return int
 */
int vc_arrows_distinction(const IVC* image, const int xbb, const int ybb, int widthbb, int heightbb)
{
    const unsigned char* data = (unsigned char*)image->data;
    int bytesperline = image->width * image->channels;
    const int height = image->height;
    const int width = image->width;
    int countLeft = 0;
    int countRight = 0;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            const long int pos = y * bytesperline + x * image->channels;

            if (x >= xbb && x <= xbb + widthbb && (y >= ybb && y <= ybb + heightbb)) { // Dentro da bounding box
                if (x < xbb + widthbb / 2) {
                    if (data[pos] == 0) countLeft++;
                } else {
                    if (data[pos] == 0) countRight++;
                }
            }
        }
    }

    printf("Count Left: %d\n", countLeft);
    printf("Count Right: %d\n", countRight);

    if (countLeft > countRight) return 1;
    return 2;
}


/**
 * @brief Get token
 *
 * @param file Ficheiro
 * @param tok
 * @param len
 * @return char*
 */
char* netpbm_get_token(FILE* file, char* tok, const int len)
{
    int c;

    for (;;) {
        while (isspace(c = getc(file)))
            ;
        if (c != '#') break;
        do c = getc(file);
        while (c != '\n' && c != EOF);
        if (c == EOF) break;
    }

    char* t = tok;

    if (c != EOF) {
        do {
            *t++ = c;
            c = getc(file);
        } while (!isspace(c) && c != '#' && c != EOF && t - tok < len - 1);

        if (c == '#') ungetc(c, file);
    }
    *t = 0;

    return tok;
}


/**
 * @brief Converte unsigned char para bit
 *
 * @param datauchar
 * @param databit
 * @param width Largura
 * @param height Altura
 * @return long int
 */
long int unsigned_char_to_bit(const unsigned char* datauchar, unsigned char* databit, const int width, const int height)
{
    unsigned char* p = databit;

    *p = 0;
    int countbits = 1;
    long int counttotalbytes = 0;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            const long int pos = width * y + x;

            if (countbits <= 8) {
                // Numa imagem PBM:
                // 1 = Preto
                // 0 = Branco
                //*p |= (datauchar[pos] != 0) << (8 - countbits);

                // Na nossa imagem:
                // 1 = Branco
                // 0 = Preto
                *p |= (datauchar[pos] == 0) << 8 - countbits;

                countbits++;
            }

            if (countbits > 8 || x == width - 1) {
                p++;
                *p = 0;
                countbits = 1;
                counttotalbytes++;
            }
        }
    }
    return counttotalbytes;
}


/**
 * @brief Converte bit para unsigned char
 *
 * @param databit
 * @param datauchar
 * @param width Largura
 * @param height Altura
 */
void bit_to_unsigned_char(const unsigned char* databit, unsigned char* datauchar, const int width, const int height)
{
    const unsigned char* p = databit;
    int countbits = 1;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            const long int pos = width * y + x;

            if (countbits <= 8) {
                // Numa imagem PBM:
                // 1 = Preto
                // 0 = Branco
                //datauchar[pos] = (*p & (1 << (8 - countbits))) ? 1 : 0;

                // Na nossa imagem:
                // 1 = Branco
                // 0 = Preto
                datauchar[pos] = *p & 1 << 8 - countbits ? 0 : 1;

                countbits++;
            }

            if (countbits > 8 || x == width - 1) {
                p++;
                countbits = 1;
            }
        }
    }
}


/**
 * @brief Le imagem
 *
 * @param filename Nome do ficheiro
 * @return IVC*
 */
IVC* vc_read_image(const char* filename)
{
    FILE* file = NULL;
    IVC* image = NULL;
    int width, height;
    int levels = 255;
    int v;

    // Abre o ficheiro
    if ((file = fopen(filename, "rb")) != NULL) {
        int channels;
        char tok[20];

        // Efetua a leitura do header
        netpbm_get_token(file, tok, sizeof(tok));

        if (strcmp(tok, "P4") == 0) { // Se PBM (Binary [0,1])
            channels = 1;
            levels = 1;
        } else if (strcmp(tok, "P5") == 0)
            channels = 1; // Se PGM (Gray [0,MAX(level,255)])
        else if (strcmp(tok, "P6") == 0)
            channels = 3; // Se PPM (RGB [0,MAX(level,255)])
        else {
#ifdef VC_DEBUG
            printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM, PGM or PPM file.\n\tBad magic number!\n");
#endif

            fclose(file);
            return NULL;
        }

        if (levels == 1) { // PBM
            if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1
                ||
                sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1)
            {
#ifdef VC_DEBUG
                printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM file.\n\tBad size!\n");
#endif

                fclose(file);
                return NULL;
            }

            // Aloca memoria para imagem
            image = vc_image_new(width, height, channels, levels);
            if (image == NULL) return NULL;

            long int sizeofbinarydata = (image->width / 8 + (image->width % 8 ? 1 : 0)) * image->height;
            unsigned char* tmp = malloc(sizeofbinarydata);
            if (tmp == NULL) return 0;

#ifdef VC_DEBUG
            printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
#endif

            if ((v = fread(tmp, sizeof(unsigned char), sizeofbinarydata, file)) != sizeofbinarydata){
#ifdef VC_DEBUG
                printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
#endif

                vc_image_free(image);
                fclose(file);
                free(tmp);
                return NULL;
            }

            bit_to_unsigned_char(tmp, image->data, image->width, image->height);
            free(tmp);
        } else { // PGM ou PPM
            long int size;
            if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
                sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1 ||
                sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &levels) != 1 || levels <= 0 || levels > 255)
            {
#ifdef VC_DEBUG
                printf("ERROR -> vc_read_image():\n\tFile is not a valid PGM or PPM file.\n\tBad size!\n");
#endif

                fclose(file);
                return NULL;
            }

            // Aloca memoria para imagem
            image = vc_image_new(width, height, channels, levels);
            if (image == NULL) return NULL;

#ifdef VC_DEBUG
            printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
#endif

            size = image->width * image->height * image->channels;

            if ((v = fread(image->data, sizeof(unsigned char), size, file)) != size) {
#ifdef VC_DEBUG
                printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
#endif

                vc_image_free(image);
                fclose(file);
                return NULL;
            }
        }
        fclose(file);
    } else {
#ifdef VC_DEBUG
        printf("ERROR -> vc_read_image():\n\tFile not found.\n");
#endif
    }
    return image;
}


/**
 * @brief Escreve imagem
 *
 * @param filename Nome do ficheiro
 * @param image Imagem
 * @return int
 */
int vc_write_image(const char* filename, const IVC* image)
{
    FILE* file = NULL;

    if (image == NULL) return 0;

    if ((file = fopen(filename, "wb")) != NULL) {
        if (image->levels == 1) {
            long int sizeofbinarydata = (image->width / 8 + (image->width % 8 ? 1 : 0)) * image->height + 1;
            unsigned char* tmp = malloc(sizeofbinarydata);

            if (tmp == NULL) return 0;
            fprintf(file, "%s %d %d\n", "P4", image->width, image->height);

            long int totalbytes = unsigned_char_to_bit(image->data, tmp, image->width, image->height);
            printf("Total = %ld\n", totalbytes);
            if (fwrite(tmp, sizeof(unsigned char), totalbytes, file) != totalbytes) {
#ifdef VC_DEBUG
                fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
#endif

                fclose(file);
                free(tmp);
                return 0;
            }
            free(tmp);
        } else {
            fprintf(file, "%s %d %d 255\n", (image->channels == 1) ? "P5" : "P6", image->width, image->height);

            if (fwrite(image->data, image->bytesperline, image->height, file) != image->height) {
#ifdef VC_DEBUG
                fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
#endif

                fclose(file);
                return 0;
            }
        }
        fclose(file);
        return 1;
    }
    return 0;
}


/**
 * @brief Converte imagem para grayscale
 *
 * @param srcdst
 * @return int
 */
int vc_gray_negative(const IVC* srcdst)
{
    if (srcdst != NULL) {
        // Validacao de erros
        if (srcdst->channels != 1 || srcdst->width <= 0 || srcdst->height <= 0 || srcdst->data == NULL) return 0;

        for (int x = 0; x < srcdst->width; x++) {
            for (int y = 0; y < srcdst->bytesperline + x * srcdst->channels; y++) {
                //calcular a posicao do byte na posicao do pixel
                const long int pos = y * srcdst->bytesperline + x * srcdst->channels;
                srcdst->data[pos] = 255 - srcdst->data[pos];
            }
        }
    }
    return 1;
}


/**
 * @brief Converte imagem para grayscale
 *
 * @param srcdst
 * @return int
 */
int vc_rgb_negative(const IVC* srcdst)
{
    if (srcdst != NULL) {
        // Validacao de erros
        if (srcdst->channels != 3 || srcdst->width <= 0 || srcdst->height <= 0 || srcdst->data == NULL) return 0;

        for (int x = 0; x < srcdst->width; x++) {
            for (int y = 0; y < srcdst->bytesperline + x * srcdst->channels; y++) {
                const long int pos = y * srcdst->bytesperline + x * srcdst->channels;

                srcdst->data[pos] = 255 - srcdst->data[pos];
                srcdst->data[pos + 1] = 255 - srcdst->data[pos + 1];
                srcdst->data[pos + 2] = 255 - srcdst->data[pos + 2];
            }
        }
    }
    return 1;
}


/**
 * @brief Converte imagem para grayscale
 *
 * @param srcdst
 * @return int
 */
int vc_rgb_get_red_gray(const IVC* srcdst)
{
    if (srcdst != NULL) {
        // Validacao de erros
        if (srcdst->channels != 3 || srcdst->width <= 0 || srcdst->height <= 0 || srcdst->data == NULL) return 0;

        for (int x = 0; x < srcdst->width; x++) {
            for (int y = 0; y < srcdst->bytesperline + x * srcdst->channels; y++) {
                const long int pos = y * srcdst->bytesperline + x * srcdst->channels;

                //se fosse obter apenas o vermelho basta apenas colocar a 0 o valor do verde e azul
                srcdst->data[pos + 1] = srcdst->data[pos]; //verde
                srcdst->data[pos + 2] = srcdst->data[pos]; //azul
            }
        }
    }
    return 1;
}


/**
 * @brief Converte imagem para grayscale
 *
 * @param srcdst
 * @return int
 */
int vc_rgb_get_green_gray(const IVC* srcdst)
{
    if (srcdst != NULL) {
        // Validacao de erros
        if (srcdst->channels != 3 || srcdst->width <= 0 || srcdst->height <= 0 || srcdst->data == NULL) return 0;

        for (int x = 0; x < srcdst->width; x++) {
            for (int y = 0; y < srcdst->bytesperline + x * srcdst->channels; y++) {
                const long int pos = y * srcdst->bytesperline + x * srcdst->channels;

                //se fosse obter apenas o verde basta apenas colocar a 0 o valor do vermelho e azul
                srcdst->data[pos] = srcdst->data[pos + 1];	   //vermelho
                srcdst->data[pos + 2] = srcdst->data[pos + 1]; //azul
            }
        }
    }
    return 1;
}


/**
 * @brief Converte imagem para grayscale
 *
 * @param srcdst
 * @return int
 */
int vc_rgb_get_blue_gray(const IVC* srcdst)
{
    if (srcdst != NULL) {
        // Validacao de erros
        if (srcdst->channels != 3 || srcdst->width <= 0 || srcdst->height <= 0 || srcdst->data == NULL) return 0;

        for (int x = 0; x < srcdst->width; x++) {
            for (int y = 0; y < srcdst->bytesperline + x * srcdst->channels; y++) {
                const long int pos = y * srcdst->bytesperline + x * srcdst->channels;

                //se fosse obter apenas o verde basta apenas colocar a 0 o valor do vermelho e azul
                srcdst->data[pos] = srcdst->data[pos + 2];	   //vermelho
                srcdst->data[pos + 1] = srcdst->data[pos + 2]; //verde
            }
        }
    }
    return 1;
}


/**
 * @brief Converte imagem de grayscale para RGB
 *
 * @param src Imagem de entrada
 * @param dst Imagem de saida
 * @return int
 */
int vc_scale_gray_to_rgb(const IVC* src, const IVC* dst)
{
    unsigned char* datasrc = src->data;
    int bytesperline_src = src->width * src->channels;
    unsigned char* datadst = dst->data;
    const int height = src->height;
    int d, i;
    long int pos_src, pos_dst;
    unsigned char red[255], green[255], blue[255];

    for (d = 0, i = 0; i < 64; i++, d += 4) {
        red[i] = 0;
        green[i] = d;
        blue[i] = 255;
    }

    for (d = 255, i = 64; i < 128; i++, d -= 4) {
        red[i] = 0;
        green[i] = 255;
        blue[i] = d;
    }

    for (d = 0, i = 128; i < 192; i++, d += 4) {
        red[i] = d;
        green[i] = 255;
        blue[i] = 0;
    }

    for (d = 255, i = 192; i < 255; i++, d -= 4) {
        red[i] = 255;
        green[i] = d;
        blue[i] = 0;
    }

    for (pos_src = 0, pos_dst = 0; pos_src < bytesperline_src * height; pos_src++, pos_dst += 3) {
        const unsigned char brilho = datasrc[pos_src];
        datadst[pos_dst] = red[brilho];
        datadst[pos_dst + 1] = green[brilho];
        datadst[pos_dst + 2] = blue[brilho];
    }

    return 1;
}


/**
 * @brief Converte imagem de grayscale para binario
 *
 * @param src Imagem de entrada
 * @param dst Imagem de saida
 * @param threshold Limiar
 * @return int
 */
int vc_gray_to_binary(const IVC* src, const IVC* dst, const int threshold)
{
    const unsigned char * data_src = (unsigned char*)src->data;
    unsigned char* data_dst = dst->data;
    const int width_src = src->width;
    const int width_dst = dst->width;
    const int height_src = src->height;
    const int height_dst = dst->height;
    const int channels_src = src->channels;
    const int channel_dst = dst->channels;
    int bytesperline_src = width_src * channels_src, bytesperline_dst = width_dst * channel_dst;

    if (width_src <= 0 || height_src <= 0 || data_src == NULL) return 0;
    if (src->levels != 255 || channels_src != 1 || channel_dst != 1) return 0;
    if (width_src != width_dst || height_src != height_dst) return 0;

    for (int y = 0; y < height_src; y++) {
        for (int x = 0; x < width_src; x++) {
            const int pos_src = y * bytesperline_src + x * channels_src;
            const int pos_dst = y * bytesperline_dst + x * channel_dst;

            data_dst[pos_dst] = 255 * (threshold > data_src[pos_src]);
        }
    }
    return 1;
}


/**
 * @brief Converte imagem de grayscale para binario
 *
 * @param srcdst
 * @return int
 */
int vc_gray_to_binary_global_mean(const IVC* srcdst)
{
    if (srcdst != NULL) {
        int x, y;
        long int pos, sum = 0, average = 0;

        // Validacao de erros
        if (srcdst->channels != 1 || srcdst->width <= 0 || srcdst->height <= 0 || srcdst->data == NULL) return 0;

        for (x = 0; x < srcdst->width; x++) {
            for (y = 0; y < srcdst->bytesperline + x * srcdst->channels; y++) {
                pos = y * srcdst->bytesperline + x * srcdst->channels;
                sum += srcdst->data[pos];
            }
        }

        average = sum / (srcdst->width * srcdst->height);

        for (x = 0; x < srcdst->width; x++) {
            for (y = 0; y < srcdst->bytesperline + x * srcdst->channels; y++) {
                pos = y * srcdst->bytesperline + x * srcdst->channels;
                if (srcdst->data[pos] > average) srcdst->data[pos] = 255;
                else srcdst->data[pos] = 0;
            }
        }
    }
    return 1;
}


/**
 * @brief Converte imagem de grayscale para binario (threshold automatico Midpoint)
 *
 * @param src Imagem de entrada
 * @param dst Imagem de saida
 * @param kernel Tamanho do kernel
 * @return int
 */
int vc_gray_to_binary_bernson(const IVC* src, const IVC* dst, const int kernel)
{
    unsigned char* datasrc = src->data;
    unsigned char* datadst = dst->data;
    const int width = src->width;
    const int height = src->height;
    int bytesperline = src->bytesperline;
    const int channels = src->channels;
    const int offset = (kernel - 1) / 2;

    // Verificacao de erros
    if (src->width <= 0 || src->height <= 0 || src->data == NULL) return 0;
    if (src->width != dst->width || src->height != dst->height || src->channels != dst->channels) return 0;
    if (channels != 1) return 0;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            const long int pos = y * bytesperline + x * channels;
            int max = 0;
            int min = 255;

            // NxM Vizinhos
            for (int ky = -offset; ky <= offset; ky++) {
                for (int kx = -offset; kx <= offset; kx++) {
                    if (y + ky >= 0 && y + ky < height && x + kx >= 0 && x + kx < width) {
                        long int posk = (y + ky) * bytesperline + (x + kx) * channels;

                        //calculo do minimo e do maximo a estudar na vizinhanca
                        if (datasrc[posk] > max) max = datasrc[posk];
                        if (datasrc[posk] < min) min = datasrc[posk];
                    }
                }
            }

            //Calculo do midpoint
            const unsigned char threshold = (unsigned char)((float)(max + min) / (float)2);

            if (datasrc[pos] > threshold) datadst[pos] = 255;
            else datadst[pos] = 0;
        }
    }
    return 1;
}


/**
 * @brief Dilatacao binaria
 *
 * @param src Imagem de entrada
 * @param dst Imagem de saida
 * @param kernel Tamanho do kernel
 * @return int
 */
int vc_binary_dilate(const IVC* src, const IVC* dst, const int kernel)
{
    unsigned char* datasrc = src->data;
    int bytesperline_src = src->width * src->channels;
    const int channels_src = src->channels;
    unsigned char* datadst = dst->data;
    const int width = src->width;
    const int height = src->height;
    const int offset = kernel / 2;

    // Verificacao de erros
    if (src->width <= 0 || src->height <= 0 || src->data == NULL) return 0;
    if (src->width != dst->width || src->height != dst->height || src->channels != dst->channels) return 0;
    if (channels_src != 1) return 0;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            const long int pos = y * bytesperline_src + x * channels_src;
            int aux = 0;

            // NxM Vizinhos
            for (int ky = -offset; ky <= offset; ky++) {
                for (int kx = -offset; kx <= offset; kx++) {
                    if (y + ky >= 0 && y + ky < height && x + kx >= 0 && x + kx < width) {
                        long int posk = (y + ky) * bytesperline_src + (x + kx) * channels_src;
                        if (datasrc[posk] == 255) aux = 255;
                    }
                }
            }
            if (aux == 255) datadst[pos] = 255;
            else datadst[pos] = 0;
        }
    }
    return 1;
}


/**
 * @brief Abertura binaria
 *
 * @param src Imagem de entrada
 * @param dst Imagem de saida
 * @param kernel Tamanho do kernel
 * @return int
 */
int vc_binary_open(const IVC* src, const IVC* dst, const int kernel)
{
    int ret = 1;

    IVC* aux = vc_image_new(src->width, src->height, src->channels, src->levels);
    ret &= vc_binary_erode(src, aux, kernel);
    ret &= vc_binary_dilate(aux, dst, kernel);
    vc_image_free(aux);

    return ret;
}


/**
 * @brief Fechamento binario
 *
 * @param src Imagem de entrada
 * @param dst Imagem de saida
 * @param kernel Tamanho do kernel
 * @return int
 */
int vc_binary_close(const IVC* src, const IVC* dst, const int kernel)
{
    int ret = 1;

    IVC* aux = vc_image_new(src->width, src->height, src->channels, src->levels);
    ret &= vc_binary_dilate(src, aux, kernel);
    ret &= vc_binary_erode(aux, dst, kernel);
    vc_image_free(aux);

    return ret;
}


/**
 * @brief Histograma de uma imagem
 *
 * @param src Imagem de entrada
 * @param dst Imagem de saida
 * @return int
 */
int vc_gray_histogram_show(const IVC* src, const IVC* dst)
{
    const unsigned char* data = (unsigned char*)src->data;
    const int width = src->width;
    const int height = src->height;
    int byteperline = src->width * src->channels;
    const int channels = src->channels;
    int x, y;
    long int pos;
    int contarpixeis[256] = { 0 };
    float pdf[256];
    float conta = 0;
    float max = 0;
    float cdf[256] = { 0 };
    float equalizacao[256] = { 0 };

    if (src->width <= 0 || src->height <= 0 || src->data == NULL) return 0;
    if (channels != 1) return 0;

    // Numero de pixeis repetidos
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            pos = y * byteperline + x * channels;
            contarpixeis[(int)data[pos]]++;
        }
    }

    // Calcula PDF
    for (y = 0; y < 256; y++) {
        pdf[y] = (float)contarpixeis[y] / (float)(width * height);
        conta += pdf[y];

        if (max < pdf[y]) max = pdf[y];
    }

    // Calcula Grafico CDF
    for (x = 0; x < 256; x++) {
        for (y = x; y >= 0; y--) {
            cdf[x] += pdf[y];
        }
    }

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            pos = y * byteperline + x * channels;
            dst->data[pos] = cdf[data[pos]] * 255;
        }
    }
    return 1;
}


/**
 * @brief Equalizacao de histograma
 *
 * @param src Imagem de entrada
 * @param dst Imagem de saida
 * @return int
 */
int vc_histograma_equalization(IVC* src, IVC* dst)
{
    const unsigned char* data = src->data;
    const int width = src->width;
    const int height = src->height;
    int byteperline = src->width * src->channels;
    const int channels = src->channels;
    int x, y;
    long int pos;
    int contarpixeis[256] = { 0 };
    float pdf[256];
    float conta = 0;
    float max = 0;
    float cdf[256] = { 0 };
    float equalizacao[256] = { 0 };

    if (src->width <= 0 || src->height <= 0 || src->data == NULL) return 0;
    if (channels != 1) return 0;

    // Numero de pixeis repetidos
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            pos = y * byteperline + x * channels;
            contarpixeis[(int)data[pos]]++;
        }
    }

    // Calcula PDF
    for (y = 0; y < 256; y++) {
        pdf[y] = (float)contarpixeis[y] / (float)(width * height);
        conta += pdf[y];

        if (max < pdf[y]) max = pdf[y];
    }

    // Calcula grafico CDF
    for (int i = 1; i < 256; i++) {
        cdf[i] = cdf[i - 1] + pdf[i];
    }

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            pos = y * byteperline + x * channels;
            dst->data[pos] = cdf[data[pos]] * src->levels;
        }
    }
    return 1;
}


/**
 * @brief Detecao de contornos pelos operadores Prewitt
 *
 * @param src Imagem de entrada
 * @param dst Imagem de saida
 * @param th Threshold [0.001, 1.000]
 * @return int
 */
int vc_gray_edge_prewitt(const IVC* src, const IVC* dst, const float th)
{
    unsigned char* datasrc = src->data;
    unsigned char* datadst = dst->data;
    const int width = src->width;
    const int height = src->height;
    int bytesperline = src->bytesperline;
    const int channels = src->channels;
    const int size = width * height;
    int x, y;
    long int posX;
    int i;
    int hist[256] = { 0 };

    // Verificacao de erros
    if (src->width <= 0 || src->height <= 0 || src->data == NULL) return 0;
    if (src->width != dst->width || src->height != dst->height || src->channels != dst->channels) return 0;
    if (channels != 1) return 0;

    for (y = 1; y < height - 1; y++) {
        for (x = 1; x < width - 1; x++) {
            const long int posA = (y - 1) * bytesperline + (x - 1) * channels;
            const long int posB = (y - 1) * bytesperline + x * channels;
            const long int posC = (y - 1) * bytesperline + (x + 1) * channels;
            const long int posD = y * bytesperline + (x - 1) * channels;
            const long int posE = y * bytesperline + (x + 1) * channels;
            const long int posF = (y + 1) * bytesperline + (x - 1) * channels;
            const long int posG = (y + 1) * bytesperline + x * channels;
            const long int posH = (y + 1) * bytesperline + (x + 1) * channels;
            int sumx = datasrc[posA] * -1;

            posX = y * bytesperline + x * channels;
            sumx += datasrc[posD] * -1;
            sumx += datasrc[posF] * -1;
            sumx += datasrc[posC] * +1;
            sumx += datasrc[posE] * +1;
            sumx += datasrc[posH] * +1;
            sumx = sumx / 3; // 3 = 1 + 1 + 1

            int sumy = datasrc[posA] * -1;
            sumy += datasrc[posB] * -1;
            sumy += datasrc[posC] * -1;
            sumy += datasrc[posF] * +1;
            sumy += datasrc[posG] * +1;
            sumy += datasrc[posH] * +1;
            sumy = sumy / 3; // 3 = 1 + 1 + 1

            datadst[posX] = (unsigned char)sqrt(sumx * sumx + sumy * sumy);
        }
    }

    // Compute a grey level histogram
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            hist[datadst[y * bytesperline + x * channels]]++;
        }
    }

    // Threshold at the middle of the occupied levels
    int histmax = 0;
    for (i = 0; i <= 255; i++) {
        histmax += hist[i];
        if (histmax >= (float)size * th) break; // th = Prewitt Threshold
    }
    int histthreshold = i;

    // Apply the threshold
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            posX = y * bytesperline + x * channels;

            if (datadst[posX] >= histthreshold) datadst[posX] = 255;
            else datadst[posX] = 0;
        }
    }
    return 1;
}


/**
 * @brief Desenha bounding-box
 *
 * @param src Imagem de entrada
 * @param blobs Blobs
 * @param numeroBlobs Numero de blobs
 * @return int
 */
int vc_desenha_bounding_box_rgb(const IVC* src, const OVC* blobs, int numeroBlobs)
{
    unsigned char* datasrc = src->data;
    int bytesperline_src = src->width * src->channels;
    const int channels_src = src->channels;
    int width = src->width;
    int height = src->height;

    // Verificaoao de erros
    if (src->width <= 0 || src->height <= 0 || src->data == NULL) return 0;
    if (src->channels != 3) return 0;

    // Percorre os blobs da imagem e para cada blob vai percorrer a altura e o comprimento da sua bounding box
    for (int i = 0; i < numeroBlobs; i++) {
        // Percorre a altura da box
        for (int yy = blobs[i].y; yy <= blobs[i].y + blobs[i].height; yy++) {
            // Percorre a largura da box
            for (int xx = blobs[i].x; xx <= blobs[i].x + blobs[i].width; xx++){
                long int posk = yy * bytesperline_src + xx * channels_src;
                // Condicao para colocar a 255 apenas os pixeis do limite da caixa
                if (yy == blobs[i].y || yy == blobs[i].y + blobs[i].height || xx == blobs[i].x || xx == blobs[i].x + blobs[i].width) {
                    datasrc[posk] = 255;
                    datasrc[posk + 1] = 255;
                    datasrc[posk + 2] = 255;
                }
            }
        }
    }
    return 1;
}


/**
 * @brief Desenha centro de massa
 *
 * @param src Imagem de entrada
 * @param blobs Blobs
 * @param numeroBlobs Numero de blobs
 * @return int
 */
int vc_desenha_centro_massa_rgb(const IVC* src, const OVC* blobs, int numeroBlobs)
{
    unsigned char* datasrc = src->data;
    int bytesperline_src = src->width * src->channels;
    const int channels_src = src->channels;
    int width = src->width;
    int height = src->height;

    // Verificaoao de erros
    if (src->width <= 0 || src->height <= 0 || src->data == NULL) return 0;
    if (src->channels != 3) return 0;

    // Percorre os blobs da imagem e para cada blob vai percorrer a altura
    // e o comprimento de uma area a volta das coordenadas do centro de massa
    for (int i = 0; i < numeroBlobs; i++) {
        for (int yy = blobs[i].yc - 3; yy <= blobs[i].yc + 3; yy++) {
            for (int xx = blobs[i].xc - 3; xx <= blobs[i].xc + 3; xx++) {
                long int posk = yy * bytesperline_src + xx * channels_src;
                datasrc[posk] = 255;
                datasrc[posk + 1] = 255;
                datasrc[posk + 2] = 255;
            }
        }
    }
    return 1;
}


/**
 * @brief Passa-baixo de mediana
 *
 * @param src Imagem de entrada
 * @param dst Imagem de saida
 * @return int
 */
int vc_gray_lowpass_median_filter(const IVC* src, const IVC* dst)
{
    unsigned char* datasrc = src->data;
    unsigned char* datadst = dst->data;
    const int width = src->width;
    const int height = src->height;
    int bytesperline = src->bytesperline;
    const int channels = src->channels;
    int mediana[9];

    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            const long int posA = (y - 1) * bytesperline + (x - 1) * channels;
            const long int posB = (y - 1) * bytesperline + x * channels;
            const long int posC = (y - 1) * bytesperline + (x + 1) * channels;
            const long int posD = y * bytesperline + (x - 1) * channels;
            const long int posX = y * bytesperline + x * channels;
            const long int posE = y * bytesperline + (x + 1) * channels;
            const long int posF = (y + 1) * bytesperline + (x - 1) * channels;
            const long int posG = (y + 1) * bytesperline + x * channels;
            const long int posH = (y + 1) * bytesperline + (x + 1) * channels;

            mediana[0] = datasrc[posA];
            mediana[1] = datasrc[posB];
            mediana[2] = datasrc[posC];
            mediana[3] = datasrc[posD];
            mediana[4] = datasrc[posX];
            mediana[5] = datasrc[posE];
            mediana[6] = datasrc[posF];
            mediana[7] = datasrc[posG];
            mediana[8] = datasrc[posH];

            for (int i = 0; i < 9 - 1; i++) {
                for (int j = 0; j < i + 1; j++) {
                    if (mediana[i] > mediana[j]) {
                        const int aux = mediana[i];
                        mediana[i] = mediana[j];
                        mediana[j] = aux;
                    }
                }
            }
            datadst[posX] = (unsigned char)mediana[4];
        }
    }
    return 1;
}


/**
 * @brief Filtro passa-alto de mediana
 *
 * @param src Imagem de entrada
 * @param dst Imagem de saida
 * @return int
 */
int vc_gray_highpass_filter(const IVC* src, const IVC* dst)
{
    unsigned char* datasrc = src->data;
    unsigned char* datadst = dst->data;
    const int width = src->width;
    const int height = src->height;
    int bytesperline = src->bytesperline;
    const int channels = src->channels;

    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            const long int posB = (y - 1) * bytesperline + x * channels;
            const long int posD = y * bytesperline + (x - 1) * channels;
            const long int posX = y * bytesperline + x * channels;
            const long int posE = y * bytesperline + (x + 1) * channels;
            const long int posG = (y + 1) * bytesperline + x * channels;
            int sum = datasrc[posB] * -1;

            sum += datasrc[posD] * -1;
            sum += datasrc[posX] * 4;
            sum += datasrc[posE] * -1;
            sum += datasrc[posG] * -1;

            datadst[posX] = (unsigned char)(sum / 6);
        }
    }
    return 1;
}


/**
 * @brief Elimina pequenos fragmentos da imagem
 *
 * @param src Imagem de entrada
 * @param dst Imagem de saida
 * @param blob Blobs
 * @return int
 */
int vc_clean_image(const IVC* src, const IVC* dst, const OVC blob) {
    unsigned char* datasrc = src->data;
    unsigned char* datadst = dst->data;
    int width = src->width;
    const int height = src->height;
    int bytesperline = src->bytesperline;
    const int channels = src->channels;

    // Verificacao de erros
    if (src == NULL) {
        printf("Error -> vc_clean_image():\n\tImage is empty!\n");
        getchar();
        return 0;
    }

    if (src->width <= 0 || src->height <= 0 || src->data == NULL) {
        printf("Error -> vc_clean_image():\n\tImage Dimensions or data are missing!\n");
        getchar();
        return 0;
    }

    if (dst->width <= 0 || dst->height <= 0 || dst->data == NULL) {
        printf("Error -> vc_clean_image():\n\tDestination Image Dimensions or data are missing!\n");
        getchar();
        return 0;
    }

    if (channels != 3 && channels != 1) {
        printf("Error -> vc_clean_image():\n\tImages with incorrect format!\n");
        getchar();
        return 0;
    }

    // Copiar os dados da imagem de origem para a imagem de destino
    memcpy(datadst, datasrc, bytesperline * height);

    // Limpar a area do blob na imagem de destino
    for (int y = blob.y; y < blob.y + blob.height; y++) {
        for (int x = blob.x; x < blob.x + blob.width; x++) {
            const long int pos_blob = y * bytesperline + x * channels;

            if (channels == 1) {
                datadst[pos_blob] = 0; // Imagem em escala de cinza
            } else if (channels == 3) {
                datadst[pos_blob] = 0; // Canal R
                datadst[pos_blob + 1] = 0; // Canal G
                datadst[pos_blob + 2] = 0; // Canal B
            }
        }
    }
    return 1;
}
