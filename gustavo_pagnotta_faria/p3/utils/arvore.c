#include "arvore.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

No* criar_no_numero(int valor) {
    No* novo_no = (No*)malloc(sizeof(No));
    if (!novo_no) {
        printf("Erro ao alocar memória para o nó da árvore.\n");
        return NULL;
    }
    novo_no->tipo = NUMERO;
    novo_no->dado.valor = valor;
    novo_no->esquerda = NULL;
    novo_no->direita = NULL;
    return novo_no;
}

No* criar_no_operador(TipoNo tipo, wchar_t operador, No* esquerda, No* direita) {
    No* novo_no = (No*)malloc(sizeof(No));
    if (!novo_no) {
        printf("Erro ao alocar memória para o nó da árvore.\n");
        return NULL;
    }
    novo_no->tipo = tipo;
    novo_no->dado.operador = operador;
    novo_no->esquerda = esquerda;
    novo_no->direita = direita;
    return novo_no;
}

void free_arvore(No* raiz) {
    if (raiz == NULL) return;
    
    free_arvore(raiz->esquerda);
    free_arvore(raiz->direita);

    free(raiz);
}

void _printar_arvore_rec(No* raiz, int nivel) {
    if (raiz == NULL) return;

    // Imprime os tabs para o nível atual
    for (int i = 0; i < nivel; i++) {
        printf("\t");
    }

    if (raiz->tipo == NUMERO) {
        printf("Número: %d\n", raiz->dado.valor);
    } else {
        wprintf(L"Operador: %lc\n", raiz->dado.operador);
        _printar_arvore_rec(raiz->esquerda, nivel + 1);
        _printar_arvore_rec(raiz->direita, nivel + 1);
    }
}

void printar_arvore(No* raiz) {
    _printar_arvore_rec(raiz, 0);
}

int eh_operacao(No* no) {
    if (no == NULL) return 0;
    return no->tipo == SOMA || no->tipo == SUB || no->tipo == MUL || no->tipo == DIV;
}