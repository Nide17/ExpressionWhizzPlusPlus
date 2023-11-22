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
  CList tokens = NULL;
  ExprTree tree = NULL;
  CDict vars = CD_new();
  char errmsg[128];
  bool time_to_quit = false;
  char expr_buf[1024];

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

    TOK_print(tokens);

    // find variables
    if (TOK_next_type(tokens) == TOK_SYMBOL)
    {
      Token var_name = TOK_next(tokens);
      if (TOK_next_type(tokens) == TOK_EQUAL)
      {
        TOK_next(tokens); // skip the '='
        double value = NAN;
        if (TOK_next_type(tokens) == TOK_VALUE)
        {
          Token t = TOK_next(tokens);
          value = t.t.value;
        }
        else if (TOK_next_type(tokens) == TOK_SYMBOL)
        {
          Token t = TOK_next(tokens);
          if (CD_contains(vars, t.t.symbol))
          {
            value = CD_retrieve(vars, t.t.symbol);
          }
          else
          {
            fprintf(stderr, "Unknown variable '%s'\n", t.t.symbol);
            goto loop_end;
          }
        }
        else
        {
          fprintf(stderr, "Expected a number or variable name after '='\n");
          goto loop_end;
        }
        CD_store(vars, var_name.t.symbol, value);
        printf("Variable '%s' set to %g\n", var_name.t.symbol, value);
        goto loop_end;
      }
      else
      {
        if (CD_contains(vars, var_name.t.symbol))
        {
          printf("Variable '%s' is %g\n", var_name.t.symbol, CD_retrieve(vars, var_name.t.symbol));
          goto loop_end;
        }
        else
        {
          fprintf(stderr, "Unknown variable '%s'\n", var_name.t.symbol);
          goto loop_end;
        }
      }
    }

    tree = Parse(tokens, errmsg, sizeof(errmsg));

    if (tree == NULL)
    {
      fprintf(stderr, "%s\n", errmsg);
      goto loop_end;
    }

    if (isnan(ET_evaluate(tree, vars, errmsg, sizeof(errmsg))))
    {
      fprintf(stderr, "%s\n", errmsg);
      goto loop_end;
    }

    ET_tree2string(tree, expr_buf, sizeof(expr_buf));

    printf("%s  ==> %g\n", expr_buf, ET_evaluate(tree, vars, errmsg, sizeof(errmsg))); // TODO: correct the evaluate function

  loop_end:
    free(input);
    input = NULL;
    CL_free(tokens);
    tokens = NULL;
    ET_free(tree);
    tree = NULL;
  }

  return 0;
}
