#include <inttypes.h>

#define OFFSET 2
#define SIZE 516

typedef struct {
    int8_t ac;
    uint16_t pc; // para poder passar de 0xFF
    uint16_t* program;
} Neander;

void executar(Neander* neander);