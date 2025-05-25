#ifndef COMPILADOR_H
#define COMPILADOR_H

#include <stdio.h>
#include <stdlib.h>

#include "./utils/arvore.h"

#define MAX_TOKEN 256
#define MAX_VAR 100

typedef enum { TOKEN_ID, TOKEN_NUM, 
    TOKEN_OP, TOKEN_ASSIGN, TOKEN_LPAREN, 
    TOKEN_RPAREN, TOKEN_KEYWORD, 
    TOKEN_STRING, TOKEN_EOF } TokenTipo;

typedef struct {
    TokenTipo tipo;
    char texto[MAX_TOKEN];
} Token;

typedef struct atribuicao {
    char variavel[MAX_VAR];
    No* expressao;
    struct atribuicao* proxima;
} Atribuicao;

typedef struct 
{
    FILE* fp;
    FILE* saida;
    Token token;
    char* nome;
    Atribuicao* atribuicoes;
    int atribuicoes_criadas;
} Compilador;

void proximo_token(Compilador* comp);
void programa(Compilador* comp);
void comeco(Compilador* comp);
void corpo(Compilador* comp);
void comando(Compilador* comp);
void atribuicao(Compilador* comp);
No* expressao(Compilador* comp);
No* adicao(Compilador* comp);
No* multiplicacao(Compilador* comp);
No* primaria(Compilador* comp);

void iterar_expressao(No* no, Compilador* comp);
void gerar_arquivo(Compilador* comp);

void free_compilador(Compilador* comp);
void printar_compilador(Compilador* comp);

#endif