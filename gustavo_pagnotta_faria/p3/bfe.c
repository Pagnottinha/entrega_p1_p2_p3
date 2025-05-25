#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEMORY_SIZE 300000
#define STACK_SIZE 1000

// Função para imprimir um array
void print_array(unsigned char* arr, int size) {
    printf("[");
    for (int i = 0; i < size; i++) {
        printf("%d", arr[i]);
        if (i < size - 1) printf(", ");
    }
    printf("]\n");
}

// Função para imprimir um byte como parte de um caractere UTF-8
void print_utf8_byte(unsigned char byte) {
    putchar(byte);
    fflush(stdout);
}

// Função principal do interpretador Brainfuck
int interpret_brainfuck(const char* code) {
    unsigned char memory[MEMORY_SIZE] = {0};
    int pos = 0;
    const char* ip = code; // instruction pointer
    
    // Stack para loops (colchetes)
    const char* loop_stack[STACK_SIZE];
    int loop_top = -1;
    
    while (*ip) {
        // printf("MEMORIA[%d]=%d / %c\n", pos, memory[pos], *ip);
        // print_array(memory, 10);
        switch (*ip) {
            case '>':
                pos++;
                if (pos >= MEMORY_SIZE) {
                    fprintf(stderr, "\nErro: Ponteiro fora dos limites (muito à direita)\n");
                    return 1;
                }
                break;
                
            case '<':
                pos--;
                if (pos < 0) {
                    fprintf(stderr, "\nErro: Ponteiro fora dos limites (muito à esquerda)\n");
                    return 1;
                }
                break;
                
            case '+': memory[pos]++; break;
            case '-': memory[pos]--; break;
            case '.': 
                print_utf8_byte(memory[pos]); 
                break;
                
            case ',':
                int c = getchar();
                if (c != EOF) {
                    memory[pos] = (unsigned char)c;
                }
                break;
                
            case '[':
                if (memory[pos] == 0) {
                    // Pular para depois do ] correspondente
                    int bracket_count = 1;
                    ip++;
                    while (*ip && bracket_count > 0) {
                        if (*ip == '[') bracket_count++;
                        else if (*ip == ']') bracket_count--;
                        ip++;
                    }
                    ip--; // Ajustar para o incremento no final do loop
                } else {
                    // Empilhar posição atual para loop
                    if (loop_top >= STACK_SIZE - 1) {
                        fprintf(stderr, "\nErro: Stack overflow - muitos loops aninhados\n");
                        return 1;
                    }
                    loop_stack[++loop_top] = ip;
                }
                break;
                
            case ']':
                if (memory[pos] != 0) {
                    // Voltar para o [ correspondente
                    if (loop_top < 0) {
                        fprintf(stderr, "\nErro: ] sem [ correspondente\n");
                        return 1;
                    }
                    ip = loop_stack[loop_top];
                } else {
                    // Desempilhar
                    if (loop_top < 0) {
                        fprintf(stderr, "\nErro: ] sem [ correspondente\n");
                        return 1;
                    }
                    loop_top--;
                }
                break;
                
            default:
                // Ignorar outros caracteres (comentários)
                break;
        }
        ip++;
    }
    
    if (loop_top >= 0) {
        fprintf(stderr, "Erro: [ sem ] correspondente\n");
        return 1;
    }
    
    return 0;
}

// Função para ler arquivo
char* read_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Erro: Não foi possível abrir o arquivo %s\n", filename);
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* content = malloc(length + 1);
    if (!content) {
        fprintf(stderr, "Erro: Não foi possível alocar memória\n");
        fclose(file);
        return NULL;
    }
    
    fread(content, 1, length, file);
    content[length] = '\0';
    fclose(file);
    
    return content;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Interpretador Brainfuck com suporte UTF-8\n");
        printf("Uso: %s <arquivo.bf> ou %s -c \"<código>\"\n\n", argv[0], argv[0]);
        printf("Comandos Brainfuck:\n");
        printf("  >  : Move ponteiro para direita\n");
        printf("  <  : Move ponteiro para esquerda\n");
        printf("  +  : Incrementa valor na célula atual\n");
        printf("  -  : Decrementa valor na célula atual\n");
        printf("  .  : Imprime valor da célula atual como byte UTF-8\n");
        printf("  ,  : Lê um byte da entrada\n");
        printf("  [  : Início de loop (executa se célula != 0)\n");
        printf("  ]  : Fim de loop (volta ao [ se célula != 0)\n\n");
        printf("Exemplo: %s -c \"++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++.\"\n", argv[0]);
        return 1;
    }
    
    char* code = NULL;
    
    if (strcmp(argv[1], "-c") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Erro: Código não fornecido após -c\n");
            return 1;
        }
        code = strdup(argv[2]);
    } else {
        code = read_file(argv[1]);
    }
    
    if (!code) {
        return 1;
    }
    
    printf("Executando código Brainfuck...\n");
    printf("Saída:\n");
    
    int result = interpret_brainfuck(code);
    
    printf("\n\nExecução %s.\n", result == 0 ? "concluída com sucesso" : "falhou");
    
    free(code);
    return result;
}