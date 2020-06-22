#define simcqca_VERSION_MAJOR 0
#define simcqca_VERSION_MINOR 3
#define simcqca_PROG_NAME "SimCQCA"

#define STRINGIFY2(X) #X
#define STRINGIFY(X) STRINGIFY2(X)

#define VERSION_LITERAL "Version " STRINGIFY(simcqca_VERSION_MAJOR) "." STRINGIFY(simcqca_VERSION_MINOR)

#define VERTEX_ARRAY_MAX_SIZE 5*100*100 // Feel free to change that constant given your GPU/CPU configuration
