# Compilador e Montador NEANDER

Este projeto implementa um compilador e montador para a arquitetura NEANDER, junto com uma máquina virtual simples para executar o código gerado.

## Componentes

1. **Compilador** (compilador.c/compilador.h)
   - Analisa expressões de alto nível e gera código assembly NEANDER
   - Suporta operações aritméticas básicas (+, -, *)
   - Utiliza árvores sintáticas abstratas para análise de expressões

2. **Montador** (assembler.c/assembler.h) 
   - Traduz assembly NEANDER para código de máquina
   - Suporta mnemônicos NEANDER padrão e extensões:
     - Todos os mnemônicos NEANDER (NOP, STA, LDA, ADD, OR, AND, NOT, JMP, JN, JZ, HLT)
     - @SUB: Operação de subtração
     - @MULT: Operação de multiplicação
     - @DIV (não totalmente implementado)

3. **Máquina Virtual** (neander.c/neander.h)
   - Executa código de máquina NEANDER
   - Implementa o conjunto completo de instruções NEANDER
   - Tamanho da memória: 516 bytes (256 posições endereçáveis)

## Compilação

Use o Makefile fornecido para compilar os componentes:

```sh
# Compilar parser
make parser

# Compilar montador
make assembler

# Compilar compilador
make compilador 

# Compilar VM NEANDER
make neander
```

## Uso

1. Escreva seu programa usando a sintaxe de alto nível (veja programa.pln como exemplo)

2. Compile para assembly:
```sh
./compilador programa.pln
```

3. Monte para código de máquina:
```sh 
./assembler programa.asm
```

4. Execute na VM:
```sh
./neander programa.bin
```

## Formato do Arquivo de Entrada

A linguagem de alto nível usa a seguinte sintaxe:

```
PROGRAMA "NOME":
INICIO
    # Atribuições de variáveis
    # Expressões com +,-,*,/
FIM
```

Veja bnf.txt para a especificação completa da gramática.

## Estrutura do Projeto

- utils: Módulos auxiliares como implementação de AST
- lexer2.c: Analisador léxico
- parser.c: Parser para código assembly
- assembler.*: Implementação do montador
- neander.*: Implementação da máquina virtual
- compilador.*: Compilador de alto nível

# Observações

- Não consegui implementar a divisão por questão de tempo, e por querer aperfeiçoar o projeto num todo. A implementação será parecida como fiz com o MUL, e já tem o formato para adicionar.