#include "lexer2.h"
#include <stdlib.h>
#include <string.h>

int VerificaNumero(char ch) { return ch >= '0' && ch <= '9'; }
int VerificaLetra(char ch) { return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'); }
int VerificaAlfaNumerico(char ch) { return VerificaLetra(ch) || VerificaNumero(ch); }
int VerificaEspeciais(char ch) { return (ch >= '!' && ch <= '&') || ch == '-' || ch == '_' || ch == '.' || ch == '?' || ch == '@'; }
int VerificaCharValido(char ch) { return VerificaAlfaNumerico(ch) || VerificaEspeciais(ch); }

Token* next_token(FILE* fp);
token_type pegar_tipo(Token* token);

int pularLinha(FILE* fp);

Token* pegar_tokens(FILE* fp) {
    if (fp == NULL) return NULL;

    Token* primeiro = next_token(fp);
    Token* ultimo = primeiro;
    Token* atual = NULL;

    while ((atual = next_token(fp)) != NULL) {
        ultimo->next = atual;
        ultimo = atual;
    }

    return primeiro;
}

Token* next_token(FILE* fp) {
    if (fp == NULL) return NULL;

    char aux[MAX_TOKEN_LEN + 1] = {0};

    char ch;
    unsigned int index = 0;

    while((ch = fgetc(fp)) != -1) {
        if (ch == ' ' || ch == '\t' || ch == '\n') {
            if (index == 0) continue;
            else break;
        }
        else if (ch == COMENTARIO && !pularLinha(fp)) {
            ch = EOF;
            break;
        }
        else if (ch != COMENTARIO && VerificaCharValido(ch)){
            aux[index] = ch;
            index++;
        }
    }

    aux[index] = '\0';

    if (ch == EOF && index == 0)
        return NULL;

    Token* token = (Token*) malloc(sizeof(Token));

    if (token == NULL) {
        printf("ERRO: falha ao realizar malloc (token).");
        return NULL;
    }

    //printf("TESTE: %s\n", aux);
    token->len = index;
    token->token = (char*) malloc(sizeof(char) * index + 1);

    if (token->token == NULL) {
        printf("ERRO: falha ao realizar malloc (string do token).");
        free(token);
        return NULL;
    }

    strncpy(token->token, aux, index);
    token->token[index] = 0;
    //printf("STRINGS: %s %d\n", token->token, index);
    token->type = pegar_tipo(token);
    token->next = NULL;

    //print_t(token);

    return token;
}

mnemonico PegarMnemonico(char* token) {
    if (strcmp(token, "NOP") == 0) return NOP;
    else if (strcmp(token, "STA") == 0) return STA;
    else if (strcmp(token, "LDA") == 0) return LDA;
    else if (strcmp(token, "ADD") == 0) return ADD;
    else if (strcmp(token, "AND") == 0) return AND;
    else if (strcmp(token, "OR") == 0) return OR;
    else if (strcmp(token, "NOT") == 0) return NOT;
    else if (strcmp(token, "JMP") == 0) return JMP;
    else if (strcmp(token, "JN") == 0) return JN;
    else if (strcmp(token, "JZ") == 0) return JZ;
    else if (strcmp(token, "HLT") == 0) return HLT;
    else return DESCONHECIDO;
}

pseudomnemonico PegarPseudomnemonico(char* token) {
    if (strcmp(token, "@SUB") == 0) return SUB;
    else if (strcmp(token, "@MUL") == 0) return MUL;
    else if (strcmp(token, "@DIV") == 0) return DIV;
    else return PSEUDO_DESCONHECIDO;
}

token_type pegar_tipo(Token* token) {
    if (token->token[0] == '.') return ESTRUTURA;
    else if (VerificaNumero(token->token[0]) || strncmp(token->token, "0x", 2) == 0) return NUMERO;
    else if (token->token[0] == '#') return MEMORIA;
    else if (token->token[0] == '!') return BLOCO;
    else if (PegarMnemonico(token->token) != DESCONHECIDO) return MNEMONICO;
    else if (PegarPseudomnemonico(token->token) != PSEUDO_DESCONHECIDO) return PSEUDOMNEMONICO;
    else return STRING;
}

int AceitaBloco(mnemonico m) {
    return m == JMP || m == JN || m == JZ;
}

int pularLinha(FILE* fp) {
    char ch;
    while((ch = fgetc(fp)) != -1) {
        if (ch == '\n') {
            return 1;
        }
    }

    return 0;
}

void free_token(Token* token) {
    if (token == NULL) return;

    while(token != NULL) {
        Token* aux = token->next;
        free(token->token);
        free(token);
        token = aux;
    }

    return;
}

int NaoPossuiValor(mnemonico m) {
    return m == NOP || m == NOT || m == HLT || m == DESCONHECIDO;
}

void print_t(Token* token) {
    Token* aux = token;
    while(aux != NULL) {
        printf("TOKEN: %s TIPO: %d\n", aux->token, aux->type);
        aux = aux->next;
    }
}

// int main(int argc, char** argv) {
//     if (argc != 2) {
//         printf("Execucao: %s nome_arquivo\n", argv[0]);
//         return 1;
//     }

//     FILE* fp = fopen(argv[1], "rb");

//     if (fp == NULL) {
//         printf("Erro ao abrir o arquivo.\n");
//         return 1;
//     }

//     Token* token = pegar_tokens(fp);

//     if (token == NULL) {
//         return 1;
//     }

//     //print_t(token);

//     free_token(token);

//     return 0;
// }