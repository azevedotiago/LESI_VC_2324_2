#pragma once
#define VC_DEBUG

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#define MAXRGB(r,g,b) (r > b ? ( r > g  ? r : g ) : ( b > g ? b : g ))
#define MINRGB(r,g,b) (r < b ? ( r < g  ? r : g ) : ( b < g ? b : g ))

#define CONV_RANGE(value, range, new_range) ((value / range) * new_range)
#define HSV_2_RGB(value) CONV_RANGE(value, 360, 255)

#define COLOR_NAME_MAX 20 // Tamanho maximo do nome da cor

// Estrutura de uma imagem
typedef struct {
    unsigned char* data;
    int width, height;
    int channels;			// Binario/Cinzentos=1; RGB=3
    int levels;				// Binario=1; Cinzentos [1,255]; RGB [1,255]
    int bytesperline;		// width * channels
} IVC;

// Estrutura de um objeto
typedef struct {
    int x, y, width, height;	// Caixa Delimitadora (Bounding Box)
    int area;					// Area
    int xc, yc;					// Centro-de-massa
    int perimeter;				// Perimetro
    int label;					// Etiqueta
} OVC;

typedef struct {
    char name[COLOR_NAME_MAX]; // Nome da cor
    int hMin, hMax; // Intervalo de matriz
    int sMin, sMax; // Intervalo de saturação
    int vMin, vMax; // Intervalo de valor
} Color;

// Alocar e Libertar uma Imagen
IVC* vc_image_new(int width, int height, int channels, int levels);
IVC* vc_image_free(IVC* image);
OVC* vc_binary_blob_labelling(const IVC* src, const IVC* dst, int* nlabels);

// Funções de Processamento de Imagem
int vc_rgb_to_gray(const IVC* src, const IVC* dst);
int vc_rgb_to_hsv(const IVC* srcdst, const IVC* dst);
int vc_hsv_segmentation(const IVC* src, const IVC* dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax);
int vc_gray_to_binary_midpoint(const IVC* src, const IVC* dst, int kernel);
int vc_binary_erode(const IVC* src, const IVC* dst, int kernel);
int vc_binary_blob_info(const IVC* src, OVC* blobs, int nblobs);
int vc_draw_bounding_box(int x, int y, int largura, int altura, const IVC* isaida);
int vc_gray_lowpass_mean_filter(const IVC* src, const IVC* dst);
int vc_center_of_mass(int x, int y, int xc, int yc, int largura, int altura, const IVC* isaida);


// Leitura e Escrita de Imagens (PBM / PGM / PPM)
IVC* vc_read_image(const char* filename);
int vc_write_image(const char* filename, const IVC* image);
int vc_gray_negative(const IVC* srcdst);
int vc_rgb_negative(const IVC* srcdst);
int vc_rgb_get_red_gray(const IVC* srcdst);
int vc_rgb_get_green_gray(const IVC* srcdst);
int vc_rgb_get_blue_gray(const IVC* srcdst);
int vc_scale_gray_to_rgb(const IVC* src, const IVC* dst);
int vc_gray_to_binary(const IVC* src, const IVC* dst, int threshold);
int vc_gray_to_binary_global_mean(const IVC* srcdst);
int vc_binary_dilate(const IVC* src, const IVC* dst, int kernel);
int vc_binary_open(const IVC* src, const IVC* dst, int kernel);
int vc_binary_close(const IVC* src, const IVC* dst, int kernel);
int vc_gray_histogram_show(const IVC* src, const IVC* dst);
int vc_gray_histogram_equalization(IVC* srcdst);
int vc_desenha_bounding_box_rgb(const IVC* src, const OVC* blobs, int numeroBlobs);
int vc_desenha_centro_massa_rgb(const IVC* src, const OVC* blobs, int numeroBlobs);
int vc_gray_edge_prewitt(const IVC* src, const IVC* dst, float th);
int vc_gray_lowpass_median_filter(const IVC* src, const IVC* dst);
int vc_gray_highpass_filter(const IVC* src, const IVC* dst);
int vc_gray_to_binary_bernson(const IVC* src, const IVC* dst, int kernel);
int vc_clean_image(const IVC* src, const IVC* dst, OVC blob);