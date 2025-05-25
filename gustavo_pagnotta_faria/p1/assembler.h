#ifndef EH__ASSEMBLER
#define EH__ASSEMBLER

#include "parser.h"

#define TAMANHO_ARQUIVO 516
#define OFFSET 2

#define PSEUDO_MACROS 2

typedef struct macro {
    pseudomnemonico pseudomnemonico;
    uint8_t pos;
} Macro;

typedef struct {
    uint16_t arquivo[TAMANHO_ARQUIVO / 2];
    Parser* parser;
    int qnt_variaveis;
    Macro macros[PSEUDO_MACROS];
} Assembler;

#endif