#ifndef EH__PARSER
#define EH__PARSER

#include "lexer2.h"
#include <inttypes.h>
#include <stdio.h>
#include <ctype.h>

typedef struct data {
    Token* token;
    uint8_t valor;
    int vazio;
    struct data* prox;
    uint8_t pos;
} Data;

typedef struct _instrucao {
    Token* instrucao;
    Token* token;
    struct _instrucao* prox;
} Instrucao;

typedef struct {
    FILE* fp;
    uint8_t org;
    Data* data;
    Instrucao* code;
} Parser;

Parser* criar_parser(FILE* fp);
void print_parser(Parser* parser);
void free_parser(Parser* parser);

#endif