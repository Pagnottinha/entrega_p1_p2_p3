#include "compilador.h"

#include <string.h>
#include <ctype.h>

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("%s <arquivo_entrada>\n", argv[0]);
        return 0;
    }

    Compilador* comp = (Compilador*) malloc(sizeof(Compilador));

    if (comp == NULL) {
        printf("Erro ao alocar memória para o compilador\n");
        return 1;
    }

    comp->fp = fopen(argv[1], "r");
    comp->atribuicoes = NULL;
    comp->nome = NULL;
    comp->atribuicoes_criadas = 0;
    
    if (comp->fp == NULL) {
        printf("Erro ao abrir o arquivo %s\n", argv[1]);
        free(comp);
        return 1;
    }

    programa(comp);

    printar_compilador(comp);

    char* arquivo_saida = malloc(sizeof(char) * (strlen(comp->nome) + 5));

    if (arquivo_saida == NULL) {
        printf("Erro ao alocar memória para o nome do arquivo de saída\n");
        free_compilador(comp);
        return 1;
    }

    sprintf(arquivo_saida, "%s.asm", comp->nome);
    comp->saida = fopen(arquivo_saida, "w");

    if (comp->saida == NULL) {
        printf("Erro ao gerar o arquivo %s\n", comp->nome);
        free_compilador(comp);
        return 1;
    }

    gerar_arquivo(comp);

    free_compilador(comp);
    return 0;
}

void proximo_token(Compilador* comp) {

    char c;
    do {
        c = fgetc(comp->fp);
    } while (isspace(c) && c != '\n');
    

    if (c == EOF) {
        comp->token.tipo = TOKEN_EOF;
        return;
    }

    if (c == '\n') {
        proximo_token(comp); // Ignorar nova linha
        return;
    }

    if (c == '=') {
        comp->token.tipo = TOKEN_ASSIGN;
        strcpy(comp->token.texto, "=");
        return;
    }

    if (strchr("+-*/", c)) {
        comp->token.tipo = TOKEN_OP;
        comp->token.texto[0] = c;
        comp->token.texto[1] = '\0';
        return;
    }

    if (c == '(') {
        comp->token.tipo = TOKEN_LPAREN;
        strcpy(comp->token.texto, "(");
        return;
    }

    if (c == ')') {
        comp->token.tipo = TOKEN_RPAREN;
        strcpy(comp->token.texto, ")");
        return;
    }

    if (c == '\"') {
        comp->token.tipo = TOKEN_STRING;
        int i = 0;
        while ((c = fgetc(comp->fp)) != '"' && c != EOF) {
            comp->token.texto[i++] = c;
        }
        comp->token.texto[i] = '\0';
        return;
    }

    if (isalpha(c)) {
        int i = 0;
        comp->token.texto[i++] = c;
        while (isalpha(c = fgetc(comp->fp)) || isdigit(c)) {
            comp->token.texto[i++] = c;
        }
        ungetc(c, comp->fp);
        comp->token.texto[i] = '\0';

        if (strcmp(comp->token.texto, "PROGRAMA") == 0 ||
            strcmp(comp->token.texto, "INICIO") == 0 ||
            strcmp(comp->token.texto, "FIM") == 0) {
            comp->token.tipo = TOKEN_KEYWORD;
        } else {
            comp->token.tipo = TOKEN_ID;
        }
        return;
    }

    if (isdigit(c) || c == '+' || c == '-') {
        int i = 0;
        comp->token.texto[i++] = c;
        while (isdigit(c = fgetc(comp->fp))) {
            comp->token.texto[i++] = c;
        }
        ungetc(c, comp->fp);
        comp->token.texto[i] = '\0';
        comp->token.tipo = TOKEN_NUM;
        return;
    }

    if (c == ':') {
        comp->token.tipo = TOKEN_KEYWORD;
        strcpy(comp->token.texto, ":");
        return;
    }

    printf("Token inválido\n");
}

void espera(Compilador* comp, TokenTipo tipo, const char *esperado) {
    if (comp->token.tipo != tipo) {
        printf("Esperado: %s, mas encontrado: %s\n", esperado, comp->token.texto);
        free_compilador(comp);
        exit(1);
    }
    proximo_token(comp);
}

// <programa> ::= <comeco> <pularLinha>* <corpo> <pularLinha>* <fim>
void programa(Compilador* comp) {
    proximo_token(comp);
    comeco(comp);

    corpo(comp);
    
    espera(comp, TOKEN_KEYWORD, "FIM");
}

// <comeco> ::= "PROGRAMA \"" <label> "\":" <pularLinha> "INICIO" <pularLinha>
void comeco(Compilador* comp) {
    espera(comp, TOKEN_KEYWORD, "PROGRAMA");

    comp->nome = malloc(strlen(comp->token.texto) * sizeof(char));
    strncpy(comp->nome, comp->token.texto, strlen(comp->token.texto));
    comp->nome[strlen(comp->token.texto)] = '\0';

    espera(comp, TOKEN_STRING, "nome do programa");
    espera(comp, TOKEN_KEYWORD, ":");

    espera(comp, TOKEN_KEYWORD, "INICIO");
}

// <corpo> ::= (<linha_codigo> | <linha_vazia>)*
// <linha_codigo> ::= <espaco> <comando> <pularLinha>
// <linha_vazia>  ::= <espaco> <pularLinha>
void corpo(Compilador* comp) {
    while (comp->token.tipo != TOKEN_KEYWORD || strcmp(comp->token.texto, "FIM") != 0) {
        comando(comp);
    }
}

// <comando> ::= <atribuicao> | <expressao>
void comando(Compilador* comp) {
    if (comp->token.tipo == TOKEN_ID) {
        atribuicao(comp);
    }
    else {
        proximo_token(comp);
    }
}

// <atribuicao> ::= <label> <espaco> "=" <espaco> <expressao>
void atribuicao(Compilador* comp) {

    if (strncmp(comp->token.texto, "expr_", 5) == 0) {
        printf("Erro: Nome de variável 'expr_' é reservado pelo compilador\n");
        free_compilador(comp);
        exit(1);
    }

    Atribuicao* atribuicao = (Atribuicao*) malloc(sizeof(Atribuicao));

    if (atribuicao == NULL) {
        printf("Erro no malloc de atribuição\n");
        free_compilador(comp);
        exit(1);
    }

    atribuicao->proxima = NULL;
    atribuicao->expressao = NULL;

    strcpy(atribuicao->variavel, comp->token.texto);

    espera(comp, TOKEN_ID, "variável");
    espera(comp, TOKEN_ASSIGN, "=");
    atribuicao->expressao = expressao(comp);
    //fprintf(saida, "STA %s\n", nomeVar);

    if (comp->atribuicoes == NULL) {
        comp->atribuicoes = atribuicao;
    } else {
        Atribuicao* atual = comp->atribuicoes;
        while (atual->proxima != NULL) {
            atual = atual->proxima;
        }
        atual->proxima = atribuicao;
    }
}

// <expressao> ::= <expressao_adicao>
No* expressao(Compilador* comp) {
    return adicao(comp);
}

// <expressao_adicao> ::= <expressao_multiplicacao> 
//  | <expressao_adicao> <espaco> "+" <espaco> <expressao_multiplicacao>
//  | <expressao_adicao> <espaco> "-" <espaco> <expressao_multiplicacao>
No* adicao(Compilador* comp) {
    No* retorno = multiplicacao(comp);

    while (comp->token.tipo == TOKEN_OP && (strcmp(comp->token.texto, "+") == 0 || strcmp(comp->token.texto, "-") == 0)) {
        TipoNo tipo;
        char simbolo = comp->token.texto[0];

        if (simbolo == '+') tipo = SOMA;
        else tipo = SUB;

        proximo_token(comp);
        No* dir = multiplicacao(comp);

        retorno = criar_no_operador(tipo, simbolo, retorno, dir);
    }

    return retorno;
}

// <expressao_multiplicacao> ::= <expressao_primaria>
//  | <expressao_multiplicacao> <espaco> "*" <espaco> <expressao_primaria>
//  | <expressao_multiplicacao> <espaco> "/" <espaco> <expressao_primaria>
No* multiplicacao(Compilador* comp) {
    No* retorno = primaria(comp);

    while (comp->token.tipo == TOKEN_OP && (strcmp(comp->token.texto, "*") == 0 || strcmp(comp->token.texto, "/") == 0)) {
        TipoNo tipo;
        char simbolo = comp->token.texto[0];

        if (simbolo == '*') tipo = MUL;
        else tipo = DIV;

        proximo_token(comp);
        No* dir = primaria(comp);

        retorno = criar_no_operador(tipo, simbolo, retorno, dir);
    }

    return retorno;
}

// <expressao_primaria> ::= <valor> | "(" <espaco> <expressao> <espaco> ")"
No* primaria(Compilador* comp) {
    if (comp->token.tipo == TOKEN_NUM) {
        //fprintf(saida, "LDA %s\n", tokenAtual.texto);
        No* valor = criar_no_numero((int) strtol(comp->token.texto, NULL, 0));
        proximo_token(comp);
        return valor;
    } 
    else if (comp->token.tipo == TOKEN_ID) {
        No* variavel = criar_no_variavel(comp->token.texto);
        proximo_token(comp);
        return variavel;
    }
    else if (comp->token.tipo == TOKEN_LPAREN) {
        proximo_token(comp);

        No* no = expressao(comp);
        espera(comp, TOKEN_RPAREN, ")");

        Atribuicao* nova = (Atribuicao*) malloc(sizeof(Atribuicao));

        if (nova == NULL) {
            printf("Erro no malloc de atribuição\n");
            free_compilador(comp);
            exit(1);
        }

        nova->expressao = no;
        char nome[MAX_VAR];
        sprintf(nome, "expr_%d", comp->atribuicoes_criadas++);
        strcpy(nova->variavel, nome);
        nova->proxima = comp->atribuicoes;

        comp->atribuicoes = nova;

        No* novo_no = criar_no_variavel(nome);

        return novo_no;
    }

    return NULL;
}

void gerar_arquivo(Compilador* comp) {
    Atribuicao* atual = comp->atribuicoes;

    fprintf(comp->saida, ".data\n");
    fprintf(comp->saida, "\tAUX ?\n");
    while (atual != NULL) {
        if (atual->expressao == NULL) {
            atual = atual->proxima;
            continue;
        }

        if (atual->expressao->esquerda == NULL && atual->expressao->direita == NULL) {
            if (atual->expressao->tipo == NUMERO)
                fprintf(comp->saida, "\t%s %d\n", atual->variavel, atual->expressao->dado.valor);
            atual = atual->proxima;
            continue;
        }
        else {
            fprintf(comp->saida, "\t%s ?\n", atual->variavel);
        }

        atual = atual->proxima;
    }

    atual = comp->atribuicoes;
    
    fprintf(comp->saida, ".code\n");
    while (atual != NULL) {
        if (atual->expressao == NULL) {
            atual = atual->proxima;
            continue;
        }

        if (atual->expressao->esquerda == NULL && atual->expressao->direita == NULL) {
            if (atual->expressao->tipo == VARIAVEL)
                fprintf(comp->saida, "\tLDA %s\n", atual->expressao->dado.variavel);
            atual = atual->proxima;
            continue;
        }

        iterar_expressao(atual->expressao, comp);
        fprintf(comp->saida, "\tSTA %s\n", atual->variavel);
        atual = atual->proxima;
    }
    fprintf(comp->saida, "\tHLT\n");
}

char* texto_tipo(TipoNo tipo) {
    switch (tipo)
    {
    case SOMA:
        return "ADD";
    case SUB:
        return "@SUB";
    case MUL:
        return "@MUL";
    case DIV:
        return "@DIV";
    case VARIAVEL:
        return "VARIAVEL";
    case NUMERO:
        return "NUMERO";
    default:
        return "INDEFINIDO";
    }
}

void printar_operacao_no(TipoNo tipo,  No* no, Compilador* comp) {
    if (no->tipo == VARIAVEL) {
        fprintf(comp->saida, "\t%s %s\n", texto_tipo(tipo), no->dado.variavel);
    } else {
        fprintf(comp->saida, "\t%s %d\n", texto_tipo(tipo), no->dado.valor);
    }
}

void iterar_expressao(No* no, Compilador* comp) {
    if (no == NULL || no->tipo == VARIAVEL || no->tipo == NUMERO) return;

    iterar_expressao(no->esquerda, comp);
    iterar_expressao(no->direita, comp);

    if (eh_operacao(no->esquerda)) {
        printar_operacao_no(no->tipo, no->direita, comp);
    }
    else if (eh_operacao(no->direita)) {
        if (no->tipo == SUB || no->tipo == DIV) {
            fprintf(comp->saida, "\tSTA AUX\n");

            if (no->esquerda->tipo == VARIAVEL)
                fprintf(comp->saida, "\tLDA %s\n", no->esquerda->dado.variavel);
            else 
                fprintf(comp->saida, "\tLDA %d\n", no->esquerda->dado.valor);
                
            fprintf(comp->saida, "\t%s AUX\n", texto_tipo(no->tipo));
        } else {
            printar_operacao_no(no->tipo, no->esquerda, comp);
        }
    }
    else {
        if (no->esquerda->tipo == VARIAVEL)
            fprintf(comp->saida, "\tLDA %s\n", no->esquerda->dado.variavel);
        else 
            fprintf(comp->saida, "\tLDA %d\n", no->esquerda->dado.valor);

        printar_operacao_no(no->tipo, no->direita, comp);
    }
    
    return;
}

void free_compilador(Compilador* comp) {
    if (comp == NULL) return;

    if (comp->fp) {
        fclose(comp->fp);
    }

    if (comp->saida) {
        fclose(comp->saida);
    }

    Atribuicao* atual = comp->atribuicoes;
    while (atual != NULL) {
        Atribuicao* proxima = atual->proxima;
        free(atual->variavel);
        free_arvore(atual->expressao);
        free(atual);
        atual = proxima;
    }

    free(comp->nome);
    free(comp);
    comp = NULL;
}

void printar_compilador(Compilador* comp) {
    if (comp == NULL) return;

    printf("Nome do programa: %s\n", comp->nome);
    printf("Atribuições:\n");

    Atribuicao* atual = comp->atribuicoes;
    while (atual != NULL) {
        printf("Variável: %s\n", atual->variavel);
        printf("- ");
        printar_arvore(atual->expressao);
        printf("\n");
        atual = atual->proxima;
    }
}