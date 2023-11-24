/*
 * expr_whizz.c
 *
 * A recursive descent parser for a simple arithmetic calculator, with
 * the operators + - * / % ^, and unary minus. Values are held as
 * doubles.
 *
 * Author: Niyomwungeri Parmenide Ishimwe <parmenin@andrew.cmu.edu>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "clist.h"
#include "token.h"
#include "tokenize.h"
#include "expr_tree.h"
#include "parse.h"
#include "cdict.h"

int main(int argc, char *argv[])
{
  char *input = NULL;
  char errmsg[128];
  bool time_to_quit = false;
  char expr_buf[1024];
  CList tokens = NULL;
  ExprTree tree = NULL;
  CDict vars = CD_new();

  printf("Welcome to ExpressionWhizz!\n");

  while (!time_to_quit)
  {
    errmsg[0] = '\0';

    input = readline("\nExpr? ");

    if (input == NULL || strcasecmp(input, "quit") == 0)
    {
      time_to_quit = true;
      goto loop_end;
    }

    if (*input == '\0') // user just hit enter, no content
      goto loop_end;

    add_history(input);

    tokens = TOK_tokenize_input(input, errmsg, sizeof(errmsg));

    if (tokens == NULL)
    {
      fprintf(stderr, "%s\n", errmsg);
      goto loop_end;
    }

    if (CL_length(tokens) == 0)
      goto loop_end;

    tree = Parse(tokens, errmsg, sizeof(errmsg));

    if (tree == NULL)
    {
      fprintf(stderr, "%s\n", errmsg);
      goto loop_end;
    }

    ET_tree2string(tree, expr_buf, sizeof(expr_buf));
    printf("%s  ==> %g\n", expr_buf, ET_evaluate(tree, vars, errmsg, sizeof(errmsg)));

  loop_end:
    free(input);
    input = NULL;
    CL_free(tokens);
    tokens = NULL;
  }

    ET_free(tree);
    tree = NULL;
    CD_free(vars);
    vars = NULL;
  return 0;
}