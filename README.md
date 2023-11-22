## ExpressionWhizz

__INTRODUCTION__

The "ExpressionWhizz" is a C program that implements a simple interactive expression evaluator that can handle a wide range of arithmetic expressions with arbitrary nesting of parentheses. It reads and evaluates user-provided expressions, returning the result. The program also supports features such as addition, subtraction, multiplication, division, and exponentiation. The program is implemented using a recursive descent parser, which is a top-down parser that constructs a parse tree from the top and the input is read from left to right.

__DESCRIPTION__

ExpressionWhizz consists of the following components:

- **token.h**: Defines the Token data structure used to represent various tokens.
- **tokenize.h** and **tokenize.c**: Tokenization functions for processing user input into tokens.
- **parse.h** and **parse.c**: A parser for converting tokens into an abstract syntax tree (ExprTree) that represents the user's expression.
- **clist.h**: A linked list implementation modified to work with Token data.
- **expr_tree.h**: The ExprTree data structure and functions for building, evaluating, and converting expressions.
- **expr_whizz.c**: The main program that gathers input, tokenizes it, parses it, and evaluates the expressions.
- **ew_test.c**: Contains automated tests for ExpressionWhizz. You are encouraged to add more tests to ensure the correctness of your implementation.

__Expression Language__

ExpressionWhizz supports standard infix-style arithmetic expressions with the following operators: +, -, *, /, and ^ (exponentiation). Unary negation is also supported. Here are the operator precedence rules:

- Parentheses
- Unary Negation
- Power
- Multiplication and Division
- Addition and Subtraction
  
__USAGE__

To use ExpressionWhizz, follow these steps:

1. Compile the project using the provided Makefile. Run the following command in your terminal:
```bash
make
```
2. Run the ExpressionWhizz program:
```bash
./expr_whizz
```
3. Enter expressions and evaluate them interactively. Type an expression and press Enter to see the result.
4. To exit ExpressionWhizz, press "CTRL+C".

Some example inputs and outputs:

```plaintext
Welcome to ExpressionWhizz!

Expr? 0.123
0.123 ==> 0.123

Expr? -0.123
(-0.123) ==> -0.123

Expr? 3+2
(3 + 2) ==> 5

Expr? 5 * -(10-4)
(5 * (-(10 - 4))) ==> -30

Expr? 2^(1.5*2) / (-1.7 + (6- 0.3))
((2 ^ (1.5 * 2)) / ((-1.7) + (6 - 0.3))) ==> 2

Expr? 1 + 2 (
Syntax error on token OPEN_PAREN

Expr? sine
Position 1: unexpected character s

Expr? 2 + + 3
Unexpected token PLUS
```

__IMPORTANCE__

It is a versatile tool for evaluating arithmetic expressions interactively. It offers comprehensive support for various operators and nested expressions.

__KEYWORDS__

<mark>ISSE</mark>     <mark>CMU</mark>     <mark>Assignment9</mark>     <mark>ExpressionWhizz</mark>     <mark>C Programming</mark>     <mark>Recursion</mark>    <mark>Tokenization</mark>    <mark>Parsing</mark>

__AUTHOR__

Howdy Pierce

__CONTRIBUTOR__

parmenin (Niyomwungeri Parmenide ISHIMWE) at CMU-Africa - MSIT

__DATE__

 November 06, 2023