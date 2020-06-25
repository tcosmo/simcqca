#define simcqca_VERSION_MAJOR 0
#define simcqca_VERSION_MINOR 3
#define simcqca_PROG_NAME "SimCQCA"
#define simcqca_PROG_NAME_EXEC "simcqca"

#define STRINGIFY2(X) #X
#define STRINGIFY(X) STRINGIFY2(X)

#define VERSION_LITERAL "Version " STRINGIFY(simcqca_VERSION_MAJOR) "." STRINGIFY(simcqca_VERSION_MINOR)

// Feel free to change that constant given your GPU/CPU configuration
// It corresponds to the number of quads which are sent at once to the GPU for rendering
#define VERTEX_ARRAY_MAX_SIZE 5 * 100 * 100

// Change to the FPS you want to achieve given your GPU/CPU configuration
#define TARGET_FPS 80

#define DEFAULT_FONT "arial.ttf"
#ifndef _WIN32
#define DEFAULT_FONT_PATH "assets/fonts/" DEFAULT_FONT
#define DEFAULT_FONT_TEXTURE_PATH "assets/fonts/font.png"
#else
#define DEAULT_FONT_PATH "assets\\fonts\\" DEFAULT_FONT
#define DEFAULT_FONT_TEXTURE_PATH "assets\\fonts\\font.png"
#endif

#define DEFAULT_FONT_TEXTURE_CHAR_W 32
#define DEFAULT_FONT_TEXTURE_CHAR_H 32
#define DEFAULT_FONT_TEXTURE_W 16
