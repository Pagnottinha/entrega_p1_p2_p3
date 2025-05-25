#include "parser.h"
#include <stdlib.h>
#include <string.h>
void print_parser(Parser* parser);

Parser* criar_parser(FILE* fp) {
    if (fp == NULL) return NULL;

    Token* token = pegar_tokens(fp);

    if (token == NULL) {
        return NULL;
    }

    if (token->type != ESTRUTURA || strncmp(token->token, ".data", token->len) != 0) {
        printf("Esperado .data ao invés de %s\n", token->token);
        free_token(token);
        return NULL;
    }

    Parser* parser = (Parser*) malloc(sizeof(Parser));

    if (parser == NULL) {
        printf("ERRO: falha ao realizar o malloc.\n");
        free_token(token);
        return NULL;
    }

    parser->fp = fp;
    parser->data = NULL;
    parser->code = NULL;
    parser->org = 0;

    Token* aux = token;
    token = token->next;
    aux->next = NULL;
    free_token(aux);

    while(token->type != ESTRUTURA) {
        if (token->type != STRING) {
            printf("Esperado um texto ao invés de %s\n", token->token);
            free_token(token);
            return NULL;
        }

        Data* data = (Data*) malloc(sizeof(Data));

        if (data == NULL) {
            printf("ERRO: falha ao realizar o malloc.\n");
            free_token(token);
            return NULL;
        }

        data->vazio = 0;
        data->prox = parser->data;
        parser->data = data;
        data->token = token;
        data->pos = 0;

        token = token->next;

        data->token->next = NULL;

        if (token == NULL) {
            printf("Esperado um numero, porem terminou o arquivo\n");
            free_parser(parser);
            free_token(token);
            return NULL;
        } 

        if (token->type != NUMERO && token->type != MEMORIA && !(token->type == STRING && token->token[0] == '?' && token->len == 1)) {
            printf("Esperado um numero ao inves de %s\n", token->token);
            free_parser(parser);
            free_token(token);
            return NULL;
        }

        char* endptr;
        errno = 0;

        uint16_t valor;

        if (token->type == MEMORIA) {
            valor = strtol(token->token + 1, &endptr, 0);
        }
        else {
            valor = strtol(token->token, &endptr, 0);
        }

        // valor pode ser um hexa ou indefinido (?)
        if (*endptr != '\0' && token->len == 1 && token->token[0] == '?') {
            data->vazio = 1;
            data->token->type = STRING;
        }
        else if (*endptr != '\0') {
            printf("ERRO: numero %s desconhecido\n", token->token);
            free_parser(parser);
            free_token(token);
            return NULL;
        }

        if (errno == ERANGE || valor > UINT8_MAX ) {
            printf("ERRO: numero %s fora do intervalo possivel\n", token->token);
            free_parser(parser);
            free_token(token);
            return NULL;
        }
        else if (data->vazio == 0) {
            data->token->type = token->type;
            data->valor = (uint8_t) valor;
        }

        aux = token;
        token = token->next;

        aux->next = NULL;
        free_token(aux);
    }

    if (token == NULL) {
        printf("Esperado a estrutura .code, porem terminou o arquivo\n");
        free_parser(parser);
        free_token(token);
        return NULL;
    }

    if (token->type != ESTRUTURA || strncmp(token->token, ".code", token->len) != 0) {
        printf("Esperado .code ao invés de %s\n", token->token);
        free_token(token);
        free_parser(parser);
        return NULL;
    }

    aux = token;
    token = token->next;

    aux->next = NULL;
    free_token(aux);

    if (token == NULL) {
        free_token(token);
        return parser;
    }

    if (token->type == ESTRUTURA) {
        if (strncmp(token->token, ".org", token->len) != 0) {
            printf("Esperado .org ao invés de %s\n", token->token);
            free_token(token);
            free_parser(parser);
            return NULL;
        }
        
        aux = token;
        token = token->next;

        aux->next = NULL;
        free_token(aux);

        if (token == NULL) {
            printf("Esperado um numero, porem terminou o arquivo\n");
            free_token(token);
            free_parser(parser);
            return NULL;
        }

        if (token->type != NUMERO) {
            printf("Esperado um numero ao inves de %s\n", token->token);
            free_token(token);
            free_parser(parser);
            return NULL;
        }

        char* endptr;
        errno = 0;
        uint16_t valor = strtol(token->token, &endptr, 0);

        if (*endptr != '\0') {
            printf("ERRO: numero %s desconhecido\n", token->token);
            free_parser(parser);
            free_token(token);
            return NULL;
        }

        if (errno == ERANGE || valor > UINT8_MAX ) {
            printf("ERRO: numero %s fora do intervalo possivel\n", token->token);
            free_parser(parser);
            free_token(token);
            return NULL;
        }

        parser->org = (uint8_t) valor;

        aux = token;
        token = token->next;

        aux->next = NULL;
        free_token(aux);
    }

    Instrucao* ultimo = NULL;

    //print_t(token);

    while(token != NULL) {
        Instrucao* instrucao = (Instrucao*) malloc(sizeof(Instrucao));
        
        if (instrucao == NULL) {
            printf("ERRO: falha ao realizar o malloc.\n");
            free_token(token);
            free_parser(parser);
            return NULL;
        }

        instrucao->prox = NULL;

        if (ultimo == NULL) {
            ultimo = instrucao;
            parser->code = instrucao;
        }
        else {
            ultimo->prox = instrucao;
            ultimo = instrucao;
        }

        if (token->type != MNEMONICO && token->type != PSEUDOMNEMONICO && token->type != BLOCO) {
            printf("Esperado um mnemonico ou pseudomnemonico ao inves de %s\n", token->token);
            free_token(token);
            free_parser(parser);
            return NULL;
        }

        instrucao->instrucao = token;
        
        token = token->next;
        instrucao->instrucao->next = NULL;

        if (instrucao->instrucao->type == BLOCO || (instrucao->instrucao->type == MNEMONICO && NaoPossuiValor(PegarMnemonico(instrucao->instrucao->token)))) {
            instrucao->token = NULL;
        }
        else {
            if (token == NULL) {
                printf("Esperado uma variavel, numero ou memoria.\n");
                free_token(token);
                free_parser(parser);
                return NULL;
            }

            if (token->type != NUMERO && token->type != MEMORIA && token->type != STRING && token->type != BLOCO) {
                printf("Esperado uma variavel, numero, bloco ou memoria ao inves de %s\n", token->token);
                free_token(token);
                free_parser(parser);
                return NULL;
            }

            if (token->type == BLOCO && instrucao->instrucao->type == MNEMONICO && !AceitaBloco(PegarMnemonico(instrucao->instrucao->token))) {
                printf("O mnemonico %s nao aceita bloco: %s\n", instrucao->instrucao->token, token->token);
                free_token(token);
                free_parser(parser);
                return NULL;
            }

            instrucao->token = token;
            token = token->next;
            instrucao->token->next = NULL;
        }
    }

    return parser;
}   

void free_parser(Parser* parser) {
    if (parser == NULL) return;

    while (parser->data != NULL) {
        Data* aux = parser->data->prox;
        free_token(parser->data->token);
        free(parser->data);
        parser->data = aux;
    }

    while (parser->code != NULL) {
        Instrucao* aux = parser->code->prox;
        free_token(parser->code->instrucao);
        free_token(parser->code->token);
        free(parser->code);
        parser->code = aux;
    }

    if (parser->fp)
        fclose(parser->fp);

    free(parser);
}

void print_parser(Parser* parser) {
    printf("Data:\n");
    Data* auxData = parser->data;
    while (auxData != NULL) {
        if (auxData->vazio) {
            printf("- %s %s\n", auxData->token->token, "Sem valor");
        }
        else {
            printf("- %s %2x %d\n", auxData->token->token, auxData->valor, auxData->token->type);
        }
        auxData = auxData->prox;
    }

    printf("Code (%x):\n", parser->org);
    Instrucao* auxInstrucao = parser->code;
    while (auxInstrucao != NULL) {
        if (auxInstrucao->instrucao->type == BLOCO) {
            printf("- BLOCO %s\n", 
                auxInstrucao->instrucao->token
            );
        }
        else {
            printf("- %x %s\n", 
                auxInstrucao->instrucao->type == MNEMONICO 
                    ? PegarMnemonico(auxInstrucao->instrucao->token) 
                    : PegarPseudomnemonico(auxInstrucao->instrucao->token), 
                auxInstrucao->token == NULL ? "NULL" : auxInstrucao->token->token
            );
        }
        
        auxInstrucao = auxInstrucao->prox;
    }
}

/*
int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Execucao: %s nome_arquivo\n", argv[0]);
        return 1;
    }

    FILE* fp = fopen(argv[1], "rb");

    if (fp == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        return 1;
    }

    Parser* parser = criar_parser(fp);

    print_parser(parser);

    free_parser(parser);

    return 0;
}
*/