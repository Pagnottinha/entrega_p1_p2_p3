# Compilador e Interpretador Brainfuck

Este projeto fornece um **Compilador Brainfuck** (`bfc`) e um **Interpretador Brainfuck** (`bfe`) escritos em C. Ele inclui ferramentas para compilar expressões aritméticas de alto nível em código Brainfuck e interpretar programas Brainfuck.

## Funcionalidades

- **Compilador (`bfc`)**: Converte expressões aritméticas de alto nível em código Brainfuck.
- **Interpretador (`bfe`)**: Executa programas Brainfuck com suporte a UTF-8.
- **Tratamento de Erros**: Detecta erros de sintaxe, estouro de memória e colchetes não correspondentes.

## Estrutura do Projeto

```
Brainfuck/
├── bfc.c               # Implementação do compilador
├── bfc.h               # Cabeçalho do compilador
├── bfe.c               # Implementação do interpretador
├── Makefile            # Automação de build
├── teste.bf            # Exemplo de programa Brainfuck
├── utils/
│   ├── arvore.c        # Implementação da árvore de expressões
│   ├── arvore.h        # Cabeçalho da árvore de expressões
```

## Requisitos

- GCC (GNU Compiler Collection)
- Make (para automação de build)

## Instruções de Build

1. Abra um terminal no diretório do projeto.
2. Execute o seguinte comando para compilar o compilador e o interpretador:
   ```bash
   make compiler runner
   ```

## Uso

### Interpretador (`bfe`)

Para interpretar um programa Brainfuck:
```bash
./bfe teste.bf
```

### Compilador (`bfc`)

Para compilar uma expressão aritmética em Brainfuck:
```bash
./bfc x=3+5*2
```

A expressão não pode conter espaços. Isso gera um arquivo `teste.bf` contendo o código Brainfuck.

## Exemplo

### Expressão de Entrada
```
x=3+5*2
```

### Código Brainfuck Gerado (`teste.bf`)
```brainfuck
[-]++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++.[-]+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++.[-]+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++.[-]++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++.[-]+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++.[-]++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++.[-]+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++.[-]+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++.[-]++>[-]+++++>[-]+++>[-]>[-]<<<[>[->+>+<<]>>[-<<+>>]<<<-]>>[<<+>>-]<[-]<[<+>-]<>[-]>[-]+>[-]+<[>[-<-<<[->+>+<<]>[-<+>]>>]++++++++++>[-]+>[-]>[-]>[-]<<<<<[->-[>+>>]>[[-<+>]+>+>>]<<<<<]>>-[-<<+>>][->[-]>>+>+<<<]<[-]++++++++[-<++++++>]>>[-<<+>>]<<]<[.[-]<]<
```

### Saída da Execução
```
x=13
```

## Notas de Desenvolvimento

- O interpretador suporta saída UTF-8 para o comando `.` do Brainfuck.
- O compilador utiliza uma árvore de expressões para gerar código Brainfuck para operações aritméticas.
