#ifndef ARVORE__H
#define ARVORE__H

typedef enum { SOMA, SUB, MUL, DIV, NUMERO, VARIAVEL } TipoNo;

typedef struct no {
    TipoNo tipo;
    union {
        int valor;
        char operador;
        char* variavel;
    } dado;
    struct no* esquerda;
    struct no* direita;
} No;

No* criar_no_numero(int valor);
No* criar_no_variavel(char* variavel);
No* criar_no_operador(TipoNo tipo, char operador, No* esquerda, No* direita);
void printar_arvore(No* raiz);
void free_arvore(No* raiz);

int eh_operacao(No* no);

#endif