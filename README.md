## ExpressionWhizz++

__INTRODUCTION__

ExpressionWhizz++ is an extension of the [https://github.com/Nide17/ExpressionWhizz](ExpressionWhizz) C program that adds support for variables. This enhancement allows users to assign values to variables, and use these variables within their expressions. This additional functionality is achieved by integrating the [https://github.com/Nide17/CDicts](CDict program) into the [https://github.com/Nide17/ExpressionWhizz](ExpressionWhizz). The CDict library is a simple dictionary implementation that allows users to store key-value pairs. The CDict library is implemented using a hash table, which is a data structure that maps keys to values for efficient lookup. The CDict library is used to store the variables and their values. The ExpressionWhizz++ program is implemented using a recursive descent parser, which is a top-down parser that constructs a parse tree from the top and the input is read from left to right to evaluate the expressions by handling a wide range of arithmetic expressions with arbitrary nesting of parentheses. The ExpressionWhizz++ program also supports features such as addition, subtraction, multiplication, division, and exponentiation.

__DESCRIPTION__

ExpressionWhizz++ consists of the following components:

- **token.h**: Defines the Token data structure used to represent various tokens.
- **tokenize.h** and **tokenize.c**: Tokenization functions for processing user input into tokens.
- **clist.h** and **clist.c**: A simple linked list implementation that allows users to store a list of tokens. The CList library is used to store the tokens generated by the tokenizer.
- **parse.h** and **parse.c**: A parser for converting tokens into an abstract syntax tree (ExprTree) that represents the user's expression.
- **expr_tree.h** and **expr_tree.c**: A library for creating and evaluating expression trees. The ExprTree library is used to evaluate the user's expression.
- **cdict.h** and **cdict.c**: A simple dictionary implementation that allows users to store key-value pairs. The CDict library is implemented using a hash table, which is a data structure that maps keys to values for efficient lookup. The CDict library is used to store the variables and their values.
- **expr_whizz.c**: The main program that gathers input, tokenizes it, parses it, and evaluates the expressions.
- **ew_test.c**: Contains automated tests for ExpressionWhizz++. You are encouraged to add more tests to ensure the correctness of your implementation.
- **Makefile**: A Makefile for compiling the ExpressionWhizz++ program and running the automated tests.
- **README.md**: This file.

__Expression Language__

ExpressionWhizz++ consists of the same components as ExpressionWhizz (standard infix-style arithmetic expressions with the following operators: +, -, *, /, and ^ (exponentiation). Unary negation is also supported), with the addition of cdict.h and cdict.c from [https://github.com/Nide17/CDicts](CDicts). These files implement the CDict type that maps from char * to double, providing the variable functionality. 

ExpressionWhizz++ supports all expressions supported by ExpressionWhizz, and introduces a new binary operator "=" to represent assignment. It also introduces symbols, which must begin with an alphabetic letter or underscore, and can contain any combination of letters, underscores, or digits, up to a maximum length of 31 characters.

ExpressionWhizz++ accepts any amount of spaces between tokens, or none at all. The binary operators +, -, * and / are left-associative, while = and ^ are right-associative. The operator precedence is as follows:

- Parentheses
- Unary Negation
- Power
- Multiplication and Division
- Addition and Subtraction
- Assignment

Its grammar is as follows:

    assignment ⇾ symbol = assignment | additive
    additive ⇾ multiplicative { ( + | – ) multiplicative }
    multiplicative ⇾ exponential { ( * | / ) exponential }
    exponential ⇾ primary [ ^ exponential ]
    primary ⇾ constant | symbol | ( assignment ) | – primary
  
The notation above, vertical bars show options, curly braces mean the contents can be repeated 0 or more times, and square brackets mean the contents can appear 0 or 1 times.

__USAGE__

To use ExpressionWhizz++, follow these steps:

1. Compile the project using the provided Makefile. Run the following command in your terminal:
```bash
make
```
1. Run the ExpressionWhizz++ program:
```bash
./expr_whizz
```
1. Enter expressions and evaluate them interactively. Type an expression and press Enter to see the result.
2. To exit ExpressionWhizz++, press "CTRL+C".

Some example inputs and outputs:

```bash
Welcome to ExpressionWhizz++!

Expr? x=25
(x = 25) ==> 25

Expr? x
x ==> 25

Expr? x*4
(x * 4) ==> 100

Expr? x = x+3
(x = (x + 3)) ==> 28

Expr? x
x ==> 28

Expr? 5 * (y=2)
(5 * (y = 2)) ==> 10

Expr? y
y ==> 2

Expr? y = y * 2
(y = (y * 2)) ==> 4

Expr? y
y ==> 4

Expr? a = b = y
(a = (b = y)) ==> 4

Expr? b
b ==> 4

Expr? a
a ==> 4

Expr? y
y ==> 4

Expr? 3 y
Syntax error on token SYMBOL

Expr? 3y
Syntax error on token SYMBOL
```

__IMPORTANCE__

ExpressionWhizz++ is a versatile tool for evaluating arithmetic expressions interactively. It offers comprehensive support for various operators, nested expressions, and variable assignment.

__KEYWORDS__

<mark>ISSE</mark>     <mark>CMU</mark>     <mark>Assignment11</mark>     <mark>ExpressionWhizz++</mark>     <mark>C Programming</mark>     <mark>Recursion</mark>    <mark>Tokenization</mark>    <mark>Parsing</mark>  <mark>Expression Trees</mark>    <mark>Hash Tables</mark>    <mark>CDict</mark>    <mark>Linked Lists</mark>    <mark>Variables</mark>    <mark>Makefile</mark>    <mark>README</mark>    

__AUTHOR__

Howdy Pierce

__CONTRIBUTOR__

parmenin (Niyomwungeri Parmenide ISHIMWE) at CMU-Africa - MSIT

__DATE__

 November 26, 2023