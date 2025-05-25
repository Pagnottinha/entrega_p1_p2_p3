#include "bfc.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("%s <texto>\n", argv[0]);
        return 0;
    }
    setlocale(LC_ALL, "");

    Compilador* comp = (Compilador*) malloc(sizeof(Compilador));

    if (comp == NULL) {
        printf("Erro ao alocar memória para o compilador\n");
        return 1;
    }

    comp->entrada = (wchar_t*) malloc(sizeof(wchar_t) * BUFFER);

    mbstowcs(comp->entrada, argv[1], BUFFER);

    wprintf(L"Entrada: %ls\n", comp->entrada);
    
    comp->atribuicao = NULL;

    programa(comp);

    printar_compilador(comp);

    char* arquivo_saida = "teste.bf";
    comp->saida = fopen(arquivo_saida, "w");

    if (comp->saida == NULL) {
        printf("Erro ao criar o arquivo\n");
        free_compilador(comp);
        return 1;
    }

    gerar_arquivo(comp);

    free_compilador(comp);
    return 0;
}

void proximo_token(Compilador* comp) {
    wchar_t c;
    do {
        c = *(comp->entrada++);
    } while (iswspace(c) && c != L'\n');
    

    if (c == L'\0') {
        comp->token.tipo = TOKEN_EOF;
        return;
    }

    if (c == L'\n') {
        proximo_token(comp);
        return;
    }

    if (c == L'=') {
        comp->token.tipo = TOKEN_ASSIGN;
        wcscpy(comp->token.texto, L"=");
        return;
    }

    if (wcschr(L"+-*/", c)) {
        comp->token.tipo = TOKEN_OP;
        comp->token.texto[0] = c;
        comp->token.texto[1] = '\0';
        return;
    }

    if (c == L'(') {
        comp->token.tipo = TOKEN_LPAREN;
        wcscpy(comp->token.texto, L"(");
        return;
    }

    if (c == ')') {
        comp->token.tipo = TOKEN_RPAREN;
        wcscpy(comp->token.texto, L")");
        return;
    }

    if (iswalpha(c)) {
        comp->token.tipo = TOKEN_STRING;
        int i = 0;
        comp->token.texto[i++] = c;
        while (iswalpha(c = *(comp->entrada++)) || iswdigit(c)) {
            comp->token.texto[i++] = c;
        }
        comp->entrada--;
        comp->token.texto[i] = '\0';
        return;
    }

    if (iswdigit(c) || c == L'+' || c == L'-') {
        int i = 0;
        comp->token.texto[i++] = c;
        while (iswdigit(c = *(comp->entrada++))) {
            comp->token.texto[i++] = c;
        }
        comp->entrada--;
        comp->token.texto[i] = L'\0';
        comp->token.tipo = TOKEN_NUM;
        return;
    }

    printf("Token inválido\n");
}

void espera(Compilador* comp, TokenTipo tipo, const char *esperado) {
    if (comp->token.tipo != tipo) {
        wprintf(L"Esperado: %s, mas encontrado: %ls\n", esperado, comp->token.texto);
        free_compilador(comp);
        exit(1);
    }
    proximo_token(comp);
}

// <programa> ::= <string>=<expressao>
void programa(Compilador* comp) {

    comp->atribuicao = (Atribuicao*) malloc(sizeof(Atribuicao));

    if (comp->atribuicao == NULL) {
        printf("Erro no malloc de atribuição\n");
        free_compilador(comp);
        exit(1);
    }
    proximo_token(comp);

    wcscpy(comp->atribuicao->variavel, comp->token.texto);

    espera(comp, TOKEN_STRING, "variável");

    espera(comp, TOKEN_ASSIGN, "=");
    
    comp->atribuicao->expressao = expressao(comp);

    espera(comp, TOKEN_EOF, "FIM DO TEXTO");
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

    while (comp->token.tipo == TOKEN_OP && (wcscmp(comp->token.texto, L"+") == 0 || wcscmp(comp->token.texto, L"-") == 0)) {
        TipoNo tipo;
        wchar_t simbolo = comp->token.texto[0];

        if (simbolo == L'+') tipo = SOMA;
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

    while (comp->token.tipo == TOKEN_OP && (wcscmp(comp->token.texto, L"*") == 0 || wcscmp(comp->token.texto, L"/") == 0)) {
        TipoNo tipo;
        wchar_t simbolo = comp->token.texto[0];

        if (simbolo == L'*') tipo = MUL;
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
        No* valor = criar_no_numero((int) wcstol(comp->token.texto, NULL, 0));
        proximo_token(comp);
        return valor;
    } 
    else if (comp->token.tipo == TOKEN_LPAREN) {
        proximo_token(comp);

        No* no = expressao(comp);

        espera(comp, TOKEN_RPAREN, ")");

        return no;
    }

    return NULL;
}

// Helper para gerar um caractere UTF-8 em brainfuck
void gerar_caractere(FILE* saida, wchar_t c) {
    char utf8[MB_CUR_MAX + 1];
    int len = wctomb(utf8, c);
    
    if (len <= 0) return;
    
    // Gera cada byte do caractere UTF-8
    for (int i = 0; i < len; i++) {
        fprintf(saida, "[-]"); // Zera a célula atual
        unsigned char byte = (unsigned char)utf8[i];
        for (int j = 0; j < byte; j++) {
            fprintf(saida, "+");
        }
        fprintf(saida, ".");
    }
}

// Helper para gerar string UTF-8 em brainfuck
void gerar_string(FILE* saida, const wchar_t* str) {
    for (int i = 0; str[i] != L'\0'; i++) {
        gerar_caractere(saida, str[i]);
    }
}

// Versão corrigida das funções de geração de operações

// Helper para gerar operações aritméticas
void gerar_operacao(FILE* saida, No* no) {
    if (no == NULL) return;

    if (no->tipo == NUMERO) {
        fprintf(saida, "[-]"); // Zera célula atual
        int valor = no->dado.valor;
        
        // Trata números negativos
        if (valor < 0) {
            valor = -valor;
            for (int i = 0; i < valor; i++) {
                fprintf(saida, "-");
            }
        } else {
            for (int i = 0; i < valor; i++) {
                fprintf(saida, "+");
            }
        }
        return;
    }

    gerar_operacao(saida, no->esquerda);  // gera 'a' na posição 0
    fprintf(saida, ">");                  // move para posição 1
    gerar_operacao(saida, no->direita);   // gera 'b' na posição 1

    // Para operadores binários, precisamos gerenciar melhor a memória
    switch (no->tipo) {
        case SOMA:
            // Layout: [a][b][0][result]
            
            // Soma: result = a + b
            fprintf(saida, "[<+>-]");             // move b para a (posição 0)
            fprintf(saida, "<");                  // volta para posição 0 (resultado)
            break;
            
        case SUB:
            // Layout: [a][b][0][result]
            
            // Subtração: result = a - b
            fprintf(saida, "[<->-]");             // subtrai b de a
            fprintf(saida, "<");                  // volta para posição 0 (resultado)
            break;
            
        case MUL:
            // Layout: [a][b][resultado][tmp]
            // Algoritmo: Para cada unidade de 'a', adiciona 'b' ao resultado
            
            fprintf(saida, ">[-]>[-]<<");   // limpa resultado e tmp
            fprintf(saida, "<");            // volta para 'a'
            fprintf(saida, "[>");           // enquanto a > 0, move para 'b'
            
            fprintf(saida, "[->+>+<<]");    // copia 'b' para tmp1 e tmp2
            
            fprintf(saida, ">>");           // move para tmp2
            fprintf(saida, "[-<<+>>]");     // restaura 'b' usando tmp2
            
            fprintf(saida, "<<<-]");        // decrementa 'a' e fecha loop
            fprintf(saida, ">>");           // move para posição do resultado
            fprintf(saida, "[<<+>>-]");
            fprintf(saida, "<[-]<");
            break;
            
        case DIV:
            // Layout: [a][b][0][1][0][tmp][resultado]
            // Algoritmo: Subtrai b de a repetidamente, incrementando o quociente
            
            fprintf(saida, ">[-]>[-]>[-]>[-]>[-]<<<<<<"); // limpa todas as células e volta para 'a'
            fprintf(saida, ">>>+<<<");      // inicializa célula do 1 e volta para 'a'
            fprintf(saida, "[>->>>>+<<<<"); // subtrai b de a e copia a
            fprintf(saida, "[>]>>");        // move se a >= 0
            fprintf(saida, "[>>>+<[<<<<+>>>>-]<]"); // incrementa quociente
            fprintf(saida, "<<<<-]");       // fecha loop principal
            fprintf(saida, ">>>>>>");       // move para quociente
            fprintf(saida, "[<<<<<<+>>>>>>-]"); // move resultado para célula inicial
            fprintf(saida, "<[-]<<-<<[-]<"); // limpa células temporárias
            break;
        default:
            break;
    }
}

// Versão melhorada da função gerar_arquivo
void gerar_arquivo(Compilador* comp) {
    // 1. Gera código para imprimir o nome da variável
    gerar_string(comp->saida, comp->atribuicao->variavel);
    
    // 2. Gera código para imprimir o sinal de igual
    gerar_caractere(comp->saida, L'=');
    
    // 3. Gera código para calcular e imprimir o resultado
    gerar_operacao(comp->saida, comp->atribuicao->expressao);
    
    // 4. Converte resultado numérico para string
    fprintf(comp->saida, ">[-]>[-]+>[-]+<[>[-<-<<[->+>+<<]>[-<+>]>>]");
    fprintf(comp->saida, "++++++++++>[-]+>[-]>[-]>[-]<<<<<");
    fprintf(comp->saida, "[->-[>+>>]>[[-<+>]+>+>>]<<<<<]");
    fprintf(comp->saida, ">>-[-<<+>>]");
    fprintf(comp->saida, "[->[-]>>+>+<<<]");
    fprintf(comp->saida, "<[-]++++++++[-<++++++>]");
    fprintf(comp->saida, ">>[-<<+>>]<<]");
    fprintf(comp->saida, "<[.[-]<]<");

}

void free_compilador(Compilador* comp) {
    if (comp == NULL) return;


    if (comp->saida) {
        fclose(comp->saida);
    }

    if (comp->atribuicao) {
        free_arvore(comp->atribuicao->expressao);
        free(comp->atribuicao);
    }
    
    free(comp);
    comp = NULL;
}

void printar_compilador(Compilador* comp) {
    if (comp == NULL) return;

    wprintf(L"Variável: %ls\n", comp->atribuicao->variavel);
    printf("- ");
    printar_arvore(comp->atribuicao->expressao);
    printf("\n");
}