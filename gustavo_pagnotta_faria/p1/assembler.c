#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "assembler.h"

void criarMacros(Assembler* assembler);
void adicionarData(Assembler* assembler);
void criarVariaveis(Assembler* assembler);
uint8_t tamanhoCodigo(Assembler* assembler);
int adicionarCodigo(Assembler* assembler);

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

    if (parser == NULL) {
        return 1;
    }

    print_parser(parser);

    Assembler assembler = {{0}, parser, 0, {0}};

    assembler.arquivo[0] = 0x4e03;
    assembler.arquivo[1] = 0x5244;

    criarMacros(&assembler);

    criarVariaveis(&assembler);

    adicionarData(&assembler);

    if (!adicionarCodigo(&assembler)) {
        free_parser(assembler.parser);
        return 1;
    }


    fp = fopen("programa.bin", "wb");

    if (fp == NULL) {
        free_parser(assembler.parser);
        return 1;
    }

    if(!fwrite(assembler.arquivo, 2, TAMANHO_ARQUIVO / 2, fp)) {
        printf("ERRO: falha ao escrever no arquivo.\n");
        free_parser(assembler.parser);
        fclose(fp);
        return 1;
    }

    free_parser(parser);
    fclose(fp);

    return 0;
}

void criarMacros(Assembler* assembler) {
    Instrucao* aux = assembler->parser->code;

    int tamanho = 0;
    int JA_FOI[PSEUDO_MACROS];
    JA_FOI[0] = 0;
    JA_FOI[1] = 0;

    assembler->macros[0].pseudomnemonico = MUL;
    assembler->macros[0].pos = 0xFE;
    assembler->macros[1].pseudomnemonico = DIV;
    assembler->macros[1].pseudomnemonico = 0xFE;

    while (aux != NULL) {
        if (aux->instrucao->type == PSEUDOMNEMONICO) {
            pseudomnemonico pseudo = PegarPseudomnemonico(aux->instrucao->token);

            if (pseudo != SUB && tamanho == 0) {
                assembler->arquivo[0xFE + OFFSET] = (uint16_t) JMP;
                assembler->arquivo[0xFA + OFFSET] = (uint16_t) 1;
                assembler->arquivo[0xF9 + OFFSET] = (uint16_t) 0;

                tamanho += 6;
            }

            if (pseudo == MUL && JA_FOI[0] == 0) {
                uint16_t codigo[] = {
                    0x10, 0xFC, 0x20, 0xF9, 0x10, 0xFB,
                    0x20, 0xFD, 0xA0, 0xFE, 0x20, 0xFB,
                    0x30, 0xFC, 0x10, 0xFB, 0x20, 0xFD,
                    0x90, 0x1A, 0x60, 0x30, 0xFA, 0x60, 
                    0x80, 0x1C, 0x30, 0xFA, 0xA0, 0x22, 
                    0x10, 0xFD, 0x80, 0x0A, 0x20, 0xFD, 
                    0x90, 0x2A, 0x20, 0xFB, 0x80, 0xFE, 
                    0x20, 0xFB, 0x60, 0x30, 0xFA, 0x80, 
                    0xFE
                };
                int tamanho_codigo = sizeof(codigo) / sizeof(codigo[0]);

                tamanho += tamanho_codigo;

                // Corrigido o cálculo da posição inicial
                uint16_t posicao_inicial = 0xFF - tamanho;

                for (uint16_t i = 0; i < tamanho_codigo; i++) {
                    uint16_t valor = codigo[i];

                    if (i > 0 && (codigo[i - 1] == (uint16_t) JMP || codigo[i - 1] == (uint16_t) JN || codigo[i - 1] == (uint16_t) JZ)) {
                        if (valor < 0xF9) {
                            valor += posicao_inicial;
                        }
                    }

                    assembler->arquivo[posicao_inicial + OFFSET + i] = valor;
                }

                assembler->macros[0].pos = posicao_inicial;

                JA_FOI[0] = 1;
            }
            else if (pseudo == DIV && JA_FOI[1] == 0) {
                printf("Nao implementado\n");

                JA_FOI[1] = 1;
            }
        }
        aux = aux->prox;
    }
}

void criarVariaveis(Assembler* assembler) {
    Instrucao* aux = assembler->parser->code;

    if (aux != NULL && aux->instrucao->type == BLOCO) {
        Data* novoData = (Data*) malloc(sizeof(Data));
        if (novoData == NULL) {
            printf("ERRO: falha ao realizar o malloc.\n");
            return;
        }
        
        novoData->token = aux->instrucao;
        novoData->token->type = MEMORIA;
        novoData->vazio = 0;
        novoData->valor = assembler->parser->org;

        novoData->prox = assembler->parser->data;
        assembler->parser->data = novoData;

        assembler->parser->code = aux->prox;
        free(aux);

        aux = assembler->parser->code;
    }

    int qnt = 0;
    int cont = 0;

    while (aux != NULL) {
        // Só processa se tiver um token e for do tipo NUMERO
        if (aux->token != NULL && aux->token->type == NUMERO) {
            char* endptr;
            errno = 0;
            int16_t valor = strtol(aux->token->token, &endptr, 0);

            if (*endptr != '\0') {
                printf("ERRO: numero desconhecido %s\n", aux->token->token);
                return;
            }

            if (errno == ERANGE || valor > INT8_MAX || valor < INT8_MIN) {
                printf("ERRO: numero fora do intervalo possivel %x\n", valor);
                return;
            }

            Data* auxData = assembler->parser->data;

            while (auxData != NULL) {
                if (!auxData->vazio && auxData->valor == (uint16_t) valor) {
                    aux->token->type = STRING;
                    aux->token->token = strdup(auxData->token->token);
                    aux->token->len = auxData->token->len;
                    break;
                }
                auxData = auxData->prox;
            }

            if (auxData == NULL) {
                char tempToken[16];
                snprintf(tempToken, sizeof(tempToken), "_n%d", qnt);
                
                Data* novoData = (Data*) malloc(sizeof(Data));
                if (novoData == NULL) {
                    printf("ERRO: falha ao realizar o malloc.\n");
                    return;
                }
                
                novoData->token = (Token*) malloc(sizeof(Token));
                if (novoData->token == NULL) {
                    printf("ERRO: falha ao realizar o malloc.\n");
                    free(novoData);
                    return;
                }
                
                novoData->token->token = strdup(tempToken);

                if (novoData->token->token == NULL) {
                    printf("ERRO: falha ao realizar o malloc.\n");
                    free(novoData->token);
                    free(novoData);
                    return;
                }

                novoData->token->len = strlen(tempToken);
                novoData->token->type = NUMERO;
                novoData->token->next = NULL;
                novoData->prox = assembler->parser->data;
                novoData->vazio = 0;
                novoData->valor = valor;
                
                novoData->prox = assembler->parser->data;
                assembler->parser->data = novoData;
                
                aux->token->type = STRING;
                aux->token->token = strdup(tempToken);
                aux->token->len = strlen(tempToken);
                
                qnt++;
            }
        }
        else if (
            aux->instrucao->type == MNEMONICO && 
            aux->token != NULL &&  // Add this check
            AceitaBloco(PegarMnemonico(aux->instrucao->token)) && 
            aux->token->type == BLOCO
        ) {
            aux->token->type = STRING;
        }

        if (aux->instrucao->type == PSEUDOMNEMONICO) {
            pseudomnemonico pseudo = PegarPseudomnemonico(aux->instrucao->token);
            if (pseudo == SUB) {
                cont += 4;
            }
            else if (pseudo == MUL) {
                cont += 16;
            }
        }
        else if (aux->token != NULL) {
            cont += 2;
        }
        else {
            cont++;
        }
        
        if (aux->prox != NULL && aux->prox->instrucao->type == BLOCO) {
            Data* novoData = (Data*) malloc(sizeof(Data));
            if (novoData == NULL) {
                printf("ERRO: falha ao realizar o malloc.\n");
                return;
            }
            
            novoData->token = aux->prox->instrucao;
            novoData->token->type = MEMORIA;
            novoData->vazio = 0;
            novoData->valor = cont + assembler->parser->org;
            
            novoData->prox = assembler->parser->data;
            assembler->parser->data = novoData;

            Instrucao* temp = aux->prox;
            aux->prox = aux->prox->prox;
            free(temp);  // Free the whole instruction node
        }

        aux = aux->prox;
    }
}

void adicionarData(Assembler* assembler) {
    uint8_t tamanho = tamanhoCodigo(assembler);
    uint8_t contador = 0;

    Data* aux = assembler->parser->data;
    while(aux != NULL) {
        uint8_t posicao = (assembler->parser->org + tamanho + contador) % (TAMANHO_ARQUIVO / 2 - OFFSET) + OFFSET;
        //printf("%s %x\n", aux->token->token, posicao - OFFSET);
        if (!aux->vazio && aux->token->type != MEMORIA) {
            assembler->arquivo[posicao] = (uint16_t) aux->valor;
        }

        if (aux->token->type != MEMORIA) {
            contador++;
        }
        
        aux = aux->prox;
    }

    assembler->qnt_variaveis = contador;

    return;
}

uint8_t tamanhoCodigo(Assembler* assembler) {
    uint8_t contagem = 0;

    Instrucao* aux = assembler->parser->code;
    while(aux != NULL) {
        if (aux->instrucao->type == PSEUDOMNEMONICO) {
            pseudomnemonico pseudo = PegarPseudomnemonico(aux->instrucao->token);
            if (pseudo == SUB) {
                contagem += 4;
            }
            else if (pseudo == MUL) {
                contagem += 16;
            }
        }
        else if (aux->token != NULL) {
            contagem += 2;
        }
        else {
            contagem++;
        }
        
        aux = aux->prox;
    }

    return contagem;
}

int pegarValor(Assembler* assembler, Instrucao* instrucao, int posicao, int tamanho) {
    if (instrucao->token != NULL) {
        uint8_t contagemData = 0;

        Data* auxData = assembler->parser->data;
        while(auxData != NULL) {
            if (strcmp(instrucao->token->token, auxData->token->token) == 0) {

                if (auxData->token->type == MEMORIA) {
                    assembler->arquivo[posicao + OFFSET] = (uint16_t) auxData->valor;
                    break;
                }
                uint8_t pos = (assembler->parser->org + tamanho + contagemData) % (TAMANHO_ARQUIVO / 2 - OFFSET);
                //printf("%s %x\n", aux->token->token, pos);
                assembler->arquivo[posicao + OFFSET] = (uint16_t) pos;
                break;
            }

            if (auxData->token->type != MEMORIA) {
                contagemData++;
            }
            
            auxData = auxData->prox;
        }

        if (auxData == NULL) {
            char* endptr;
            errno = 0;
            int16_t valor;

            // Verifica se é um endereço de memória
            if (instrucao->token->type == MEMORIA) {
                valor = strtol(instrucao->token->token + 1, &endptr, 0); // Pula o '#'
            }
            else {
                valor = strtol(instrucao->token->token, &endptr, 0);
            }

            // valor pode ser um hexa ou indefinido (?)
            if (*endptr != '\0') {
                printf("ERRO: numero desconhecido %s\n", instrucao->token->token);
                return 0;
            }

            if (errno == ERANGE || valor > INT8_MAX || valor < INT8_MIN) {
                printf("ERRO: numero fora do intervalo possivel %x\n", valor);
                return 0;
            }
            else {
                //printf("%s %x\n", instrucao->token->token, valor);
                assembler->arquivo[posicao + OFFSET] = (uint16_t) (
                    instrucao->token->type == MEMORIA 
                    ? (uint16_t) valor :  
                    (uint16_t) valor % (TAMANHO_ARQUIVO / 2 - OFFSET)
                );
            }
        }
    }

    return 1;
}

int adicionarCodigo(Assembler* assembler) {
    uint8_t tamanho = tamanhoCodigo(assembler);
    uint8_t posicao = assembler->parser->org;
    Instrucao* aux = assembler->parser->code;

    int qnt_novos_jumps = 0;
    
    while(aux != NULL) {
        if (aux->instrucao->type == MNEMONICO) {
            mnemonico mnemonico = PegarMnemonico(aux->instrucao->token);
            assembler->arquivo[posicao + OFFSET] = (uint16_t) mnemonico;

            posicao++;

            if (!NaoPossuiValor(mnemonico)) {
                if (!pegarValor(assembler, aux, posicao, tamanho)) return 0;
                posicao++;
            }
        }
        else if (aux->instrucao->type == PSEUDOMNEMONICO) {
            pseudomnemonico pseudomnemonico = PegarPseudomnemonico(aux->instrucao->token);
            if (pseudomnemonico == SUB) {
                assembler->arquivo[posicao + OFFSET] = (uint16_t) NOT;
                posicao++;
                assembler->arquivo[posicao + OFFSET] = (uint16_t) ADD;
                posicao++;
                if (!pegarValor(assembler, aux, posicao, tamanho)) return 0;
                posicao++;
                assembler->arquivo[posicao + OFFSET] = (uint16_t) NOT;
                posicao++;
            }
            else {
                for (int i = 0; i < PSEUDO_MACROS; i++) {
                    Macro macro = assembler->macros[i];
                    if (macro.pseudomnemonico == pseudomnemonico) {
                        assembler->arquivo[posicao + OFFSET] = (uint16_t) STA;
                        posicao++;
                        assembler->arquivo[posicao + OFFSET] = (uint16_t) 0xFB;
                        posicao++;

                        assembler->arquivo[posicao + OFFSET] = (uint16_t) LDA;
                        posicao++;
                        if (!pegarValor(assembler, aux, posicao, tamanho)) return 0;
                        posicao++;

                        assembler->arquivo[posicao + OFFSET] = (uint16_t) STA;
                        posicao++;
                        assembler->arquivo[posicao + OFFSET] = (uint16_t) 0xFD;
                        posicao++;

                        assembler->arquivo[posicao + OFFSET] = (uint16_t) LDA;
                        posicao++;
                        assembler->arquivo[posicao + OFFSET] = (uint16_t) 0xFB;
                        posicao++;

                        uint16_t pos_jump = (uint16_t) tamanho + assembler->qnt_variaveis + qnt_novos_jumps;

                        assembler->arquivo[posicao + OFFSET] = (uint16_t) LDA;
                        posicao++;
                        assembler->arquivo[posicao + OFFSET] = pos_jump;
                        posicao++;

                        assembler->arquivo[posicao + OFFSET] = (uint16_t) STA;
                        posicao++;
                        assembler->arquivo[posicao + OFFSET] = (uint16_t) 0xFF;
                        posicao++;

                        assembler->arquivo[posicao + OFFSET] = (uint16_t) LDA;
                        posicao++;
                        assembler->arquivo[posicao + OFFSET] = (uint16_t) 0xFB;
                        posicao++;

                        assembler->arquivo[posicao + OFFSET] = (uint16_t) JMP;
                        posicao++;
                        assembler->arquivo[posicao + OFFSET] = (uint16_t) macro.pos;
                        posicao++;

                        //printf("%d %d %d\n", tamanho, pos_jump, posicao);
                        assembler->arquivo[pos_jump + OFFSET] = (uint16_t) posicao;

                        break;
                    }
                }

                qnt_novos_jumps++;
            }
        }
        aux = aux->prox;
    }

        

    return 1;
}