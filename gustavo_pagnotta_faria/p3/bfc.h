#ifndef COMPILADOR_H
#define COMPILADOR_H

#include <stdio.h>
#include <stdlib.h>
#include <wctype.h>
#include <locale.h>
#include <wchar.h>

#include "./utils/arvore.h"

#define MAX_TOKEN 256
#define MAX_VAR 100
#define BUFFER 1024

typedef enum { TOKEN_NUM, 
    TOKEN_OP, TOKEN_ASSIGN, TOKEN_LPAREN, 
    TOKEN_RPAREN,
    TOKEN_STRING, TOKEN_EOF } TokenTipo;

typedef struct {
    TokenTipo tipo;
    wchar_t texto[MAX_TOKEN];
} Token;

typedef struct atribuicao {
    wchar_t variavel[MAX_VAR];
    No* expressao;
} Atribuicao;

typedef struct 
{
    wchar_t* entrada;
    FILE* saida;
    Token token;
    Atribuicao* atribuicao;
} Compilador;

void proximo_token(Compilador* comp);
void programa(Compilador* comp);
No* expressao(Compilador* comp);
No* adicao(Compilador* comp);
No* multiplicacao(Compilador* comp);
No* primaria(Compilador* comp);

void gerar_arquivo(Compilador* comp);

void free_compilador(Compilador* comp);
void printar_compilador(Compilador* comp);

#endif