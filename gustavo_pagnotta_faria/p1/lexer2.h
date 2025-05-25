#ifndef EH__LEXER
#define EH__LEXER

#include <stdio.h>

#define MAX_TOKEN_LEN 100
#define COMENTARIO ';'

typedef enum {
    NOP = 0x0,
    STA = 0x10,
    LDA = 0x20,
    ADD = 0x30,
    OR = 0x40,
    AND = 0x50,
    NOT = 0x60,
    JMP = 0x80,
    JN = 0x90,
    JZ = 0xA0,
    HLT = 0xF0,
    DESCONHECIDO = 0xFF // CASO NÃO EXISTA
} mnemonico;

typedef enum {
    SUB,
    MUL,
    DIV,
    PSEUDO_DESCONHECIDO
} pseudomnemonico;

typedef enum {
    STRING,
    NUMERO,
    MEMORIA,
    MNEMONICO,
    PSEUDOMNEMONICO,
    BLOCO,
    ESTRUTURA
} token_type;

typedef struct _Token {
    char* token;
    size_t len;
    token_type type;
    struct _Token* next;
} Token;

mnemonico PegarMnemonico(char* token);
pseudomnemonico PegarPseudomnemonico(char* token);
int NaoPossuiValor(mnemonico m);
int AceitaBloco(mnemonico m);

Token* pegar_tokens(FILE* fp);
void free_token(Token* token);

void print_t(Token* token);


#endif