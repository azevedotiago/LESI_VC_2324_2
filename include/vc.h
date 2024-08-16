#define VC_DEBUG

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#define MAXRGB(r,g,b) (r > b ? ( r > g  ? r : g ) : ( b > g ? b : g ))
#define MINRGB(r,g,b) (r < b ? ( r < g  ? r : g ) : ( b < g ? b : g ))
#define CONV_RANGE(value, range, new_range) (value / range) * new_range
#define HSV_2_RGB(value) CONV_RANGE(value, 360, 255)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                   ESTRUTURA DE UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


typedef struct {
    unsigned char* data;
    int width, height;
    int channels;			// Binario/Cinzentos=1; RGB=3
    int levels;				// Binario=1; Cinzentos [1,255]; RGB [1,255]
    int bytesperline;		// width * channels
} IVC;

typedef struct {
    int x, y, width, height;	// Caixa Delimitadora (Bounding Box)
    int area;					// Area
    int xc, yc;					// Centro-de-massa
    int perimeter;				// Perimetro
    int label;					// Etiqueta
} OVC;

#define COR_NOME_MAX 20 // Tamanho maximo do nome da cor

typedef struct {
    char nome[COR_NOME_MAX]; // Nome da cor
    int hmin, hmax; // Intervalo de matriz
    int smin, smax; // Intervalo de saturação
    int vmin, vmax; // Intervalo de valor
} Cor;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                    PROTOTIPOS DE FUNCOES                     //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

// FUNCOES: ALOCAR E LIBERTAR UMA IMAGEM
IVC* vc_image_new(int width, int height, int channels, int levels);
IVC* vc_image_free(IVC* image);

OVC* vc_binary_blob_labelling(IVC* src, IVC* dst, int* nlabels);

int vc_rgb_to_gray(IVC* src, IVC* dst);
int vc_rgb_to_hsv(IVC* srcdst, IVC* dst);
int vc_hsv_segmentation(IVC* src, IVC* dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax);
int vc_gray_to_binary_midpoint(IVC* src, IVC* dst, int kernel);
int vc_binary_erode(IVC* src, IVC* dst, int kernel);
int vc_binary_blob_info(IVC* src, OVC* blobs, int nblobs);
int vc_draw_bounding_box(int x, int y, int largura, int altura, IVC* isaida);
int vc_gray_lowpass_mean_filter(IVC* src, IVC* dst);
//int vc_draw_bounding_box(int x, int y, int largura, int altura, IVC* isaida);
int vc_center_of_mass(int x, int y, int xc, int yc, int largura, int altura, IVC* isaida);


// FUNCOES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
//IVC* vc_read_image(char* filename);
int vc_write_image(char* filename, IVC* image);
//int vc_gray_negative(IVC* srcdst);
//int vc_rgb_negative(IVC* srcdst);
//int vc_rgb_get_red_gray(IVC* srcdst);
//int vc_rgb_get_green_gray(IVC* srcdst);
//int vc_rgb_get_blue_gray(IVC* srcdst);
//int vc_scale_gray_to_rgb(IVC* src, IVC* dst);
//int vc_gray_to_binary(IVC* src, IVC* dst, int threshold);
//int vc_gray_to_binary_global_mean(IVC* srcdst);
//int vc_binary_dilate(IVC* src, IVC* dst, int kernel);
//int vc_binary_open(IVC* src, IVC* dst, int kernel);
int vc_binary_close(IVC* src, IVC* dst, int kernel);
//int vc_gray_histogram_show(IVC* src, IVC* dst);
//int vc_gray_histogram_equalization(IVC* srcdst);
//int vc_desenha_bounding_box_rgb(IVC* src, OVC* blobs, int numeroBlobs);
//int vc_desenha_centro_massa_rgb(IVC* src, OVC* blobs, int numeroBlobs);
//int vc_gray_edge_prewitt(IVC* src, IVC* dst, float th);
//int vc_gray_lowpass_median_filter(IVC* src, IVC* dst);
//int vc_gray_highpass_filter(IVC* src, IVC* dst);
//int vc_gray_to_binary_bernson(IVC* src, IVC* dst, int kernel);
//
//int vc_clean_image(IVC* src, IVC* dst, OVC blob);