#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "neander.h"

int main(int argc, char** argv) {
    if (argc != 2 && argc != 3) {
        printf("Execucao: %s nome_arquivo posicao_inicial(opcional) %d\n", argv[0], argc);
        return 1;
    }

    FILE* fp = fopen(argv[1], "rb+");

    if(fp == NULL) {
        printf("ERRO: falha ao achar o arquivo.\n");
        return 1;
    }

    int inicio = 0;

    if (argc == 3)
        inicio = atoi(argv[2]);

    Neander neander = {0, (uint8_t) inicio, malloc(sizeof(uint16_t) * SIZE / 2)};

    if (neander.program == NULL) {
        printf("ERRO: falha no malloc\n");
        fclose(fp);
        return 1;
    }

    fseek(fp, 0, SEEK_END);
    if (ftell(fp) != SIZE) {
        printf("ERRO: tamanho do arquivo inválido.\n");
        free(neander.program);
        fclose(fp);
        return 1;
    }

    fseek(fp, 0, SEEK_SET);

    if (!fread(neander.program, 2, SIZE / 2, fp)) {
        printf("ERRO: falha ao ler o arquivo.\n");
        free(neander.program);
        fclose(fp);
        return 1;
    }

    if (neander.program[0] != 0x4e03 || neander.program[1] != 0x5244) {
        printf("ERRO: o arquivo nao tem a verificacao de um arquivo .mem\n");
        free(neander.program);
        fclose(fp);
        return 1;
    }

    executar(&neander);

    fseek(fp, 0, SEEK_SET);

    FILE* fp_save = fopen("Resultado.mem", "wb");

    if(!fwrite(neander.program, 2, SIZE / 2, fp_save)) {
        printf("ERRO: falha ao escrever no arquivo.\n");
        free(neander.program);
        fclose(fp);
        return 1;
    }

    printf("Acc: %x\n", neander.ac);
    printf("Pointer: %x\n", neander.pc);

    free(neander.program);
    fclose(fp);
    fclose(fp_save);

    return 0;
}

void executar(Neander* neander) {
    int parar = 0;
    while(neander->pc <= 0xFF && parar == 0) {
        int16_t position;
        switch (neander->program[OFFSET + neander->pc])
        {
        case 0x0:
            printf("NOP\n");
            neander->pc++;
            break;
        case 0x10:
            neander->pc++;
            position = neander->program[OFFSET + neander->pc];
            printf("STA: %x -> %x\n", (uint8_t) neander->ac, position);
            neander->program[OFFSET + position] = (uint8_t) neander->ac;
            neander->pc++;
            break;
        case 0x20:
            neander->pc++;
            position = neander->program[OFFSET + neander->pc];
            printf("LDA: %x - %x\n", position, neander->program[OFFSET + position]);
            neander->ac = neander->program[OFFSET + position];
            neander->pc++;
            break;
        case 0x30:
            neander->pc++;
            position = neander->program[OFFSET + neander->pc];
            printf("ADD: %x - %x + %x\n", position, neander->program[OFFSET + position], (uint8_t) neander->ac);
            neander->ac += neander->program[OFFSET + position];
            neander->pc++;
            break;
        case 0x40:
            neander->pc++;
            position = neander->program[OFFSET + neander->pc];
            printf("OR: %x - %x | %x\n", position, neander->program[OFFSET + position], (uint8_t) neander->ac);
            neander->program[OFFSET + position] |= neander->ac;
            neander->pc++;
            break;
        case 0x50:
            neander->pc++;
            position = neander->program[OFFSET + neander->pc];
            printf("AND: %x - %x & %x\n", position, neander->program[OFFSET + position], (uint8_t) neander->ac);
            neander->program[OFFSET + position] &= neander->ac;
            neander->pc++;
            break;
        case 0x60:
            printf("NOT: %x -> %x\n", (uint8_t) neander->ac, (uint8_t) ~neander->ac);
            neander->ac = ~neander->ac;
            neander->pc++;     
            break;
        case 0x80:
            neander->pc++;
            position = neander->program[OFFSET + neander->pc];
            printf("JMP: %x -> %x\n", neander->pc, position);
            neander->pc = position;
            break;
        case 0x90:
            neander->pc++;
            if (neander->ac < 0) {
                position = neander->program[OFFSET + neander->pc];
                printf("JN: %x -> %x\n", neander->pc, position);
                neander->pc = position;   
            }
            else {
                printf("JN: nao deu jump\n");
                neander->pc++;
            }
            break;
        case 0xA0:
            neander->pc++;
            if (neander->ac == 0) {
                position = neander->program[OFFSET + neander->pc];
                printf("JZ: %x -> %x\n", neander->pc, position);
                neander->pc = position; 
            }
            else {
                printf("JN: nao deu jump\n");
                neander->pc++;
            }
            break;
        case 0xF0:
            printf("HTL\n");
            parar = 1;
            break;
        default:
            break;
        }      
    }
}