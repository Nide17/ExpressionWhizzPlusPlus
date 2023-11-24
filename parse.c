/*
 * parse.c
 *
 * Code that implements a recursive descent parser for arithmetic
 * expressions
 *
 * Author: Niyomwungeri Parmenide Ishimwe <parmenin@andrew.cmu.edu>
 */

#include <stdio.h>

#include "parse.h"
#include "tokenize.h"

/*
 * Forward declarations for the functions (rules) to produce the
 * ExpressionWhizz grammar.  See the assignment writeup for the grammar.
 * Each function has the same signature, so we will document all of
 * them here.
 *
 * Parameters:
 *   tokens     List of tokens remaining to be parsed
 *   errmsg     Return space for an error message, filled in in case of error
 *   errmsg_sz  The size of errmsg
 *
 * Returns: The parsed ExprTree on success. If a parsing error is
 *   encountered, copies an error message into errmsg and returns
 *   NULL.
 */
static ExprTree assignment(CList tokens, char *errmsg, size_t errmsg_sz);     // symbol = assignment | additive
static ExprTree additive(CList tokens, char *errmsg, size_t errmsg_sz);       // multiplicative { ( + | – ) multiplicative }
static ExprTree multiplicative(CList tokens, char *errmsg, size_t errmsg_sz); // exponential { ( * | / ) exponential }
static ExprTree exponential(CList tokens, char *errmsg, size_t errmsg_sz);    // primary [ ^ exponential ]
static ExprTree primary(CList tokens, char *errmsg, size_t errmsg_sz);        // constant | symbol | ( assignment ) | – primary


ExprTree Parse(CList tokens, char *errmsg, size_t errmsg_sz)
{
  if (tokens == NULL || CL_length(tokens) == 0 || TOK_next_type(tokens) == TOK_END)
    return NULL;

  ExprTree ret = assignment(tokens, errmsg, errmsg_sz);

  if (ret == NULL)
    return NULL;

  if (TOK_next_type(tokens) != TOK_END)
  {
    snprintf(errmsg, errmsg_sz, "Syntax error on token %s", TT_to_str(TOK_next_type(tokens)));
    ET_free(ret);
    return NULL;
  }

  return ret;
}

static ExprTree assignment(CList tokens, char *errmsg, size_t errmsg_sz)
{
  ExprTree expr = additive(tokens, errmsg, errmsg_sz);

  if (expr == NULL)
    return NULL;

  while (TOK_next_type(tokens) == TOK_EQUAL)
  {
    TOK_consume(tokens);
    ExprTree right = assignment(tokens, errmsg, errmsg_sz);

    if (right == NULL)
    {
      ET_free(expr);
      return NULL;
    }

    ExprTree temp_tree = ET_node(OP_ASSIGN, expr, right);

    if (temp_tree == NULL)
    {
      ET_free(expr);
      ET_free(right);
      return NULL;
    }

    expr = temp_tree;
  }

  return expr;
}

static ExprTree additive(CList tokens, char *errmsg, size_t errmsg_sz)
{
  ExprTree expr = multiplicative(tokens, errmsg, errmsg_sz);

  if (expr == NULL)
    return NULL;

  // WHILE THERE ARE STILL TOKENS TO BE PARSED
  while (TOK_next_type(tokens) == TOK_PLUS || TOK_next_type(tokens) == TOK_MINUS)
  {
    TokenType op = TOK_next_type(tokens);
    TOK_consume(tokens);

    ExprTree right = multiplicative(tokens, errmsg, errmsg_sz);

    if (right == NULL)
    {
      ET_free(expr);
      return NULL;
    }

    // CREATE A NEW NODE WITH THE OPERATOR AND THE LEFT AND RIGHT EXPRESSIONS
    ExprTree temp_tree = ET_node((op == TOK_PLUS) ? OP_ADD : OP_SUB, expr, right);

    if (temp_tree == NULL)
    {
      ET_free(expr);
      ET_free(right);
      return NULL;
    }

    expr = temp_tree;
  }

  return expr;
}

static ExprTree multiplicative(CList tokens, char *errmsg, size_t errmsg_sz)
{
  ExprTree expr = exponential(tokens, errmsg, errmsg_sz);

  if (expr == NULL)
    return NULL;

  // WHILE THERE ARE STILL TOKENS TO BE PARSED
  while (TOK_next_type(tokens) == TOK_MULTIPLY || TOK_next_type(tokens) == TOK_DIVIDE)
  {
    TokenType op = TOK_next_type(tokens);
    TOK_consume(tokens);
    ExprTree right = exponential(tokens, errmsg, errmsg_sz);

    if (right == NULL)
    {
      ET_free(expr);
      return NULL;
    }

    // CREATE A NEW NODE WITH THE OPERATOR AND THE LEFT AND RIGHT EXPRESSIONS
    ExprTree temp_tree = ET_node((op == TOK_MULTIPLY) ? OP_MUL : OP_DIV, expr, right);

    if (temp_tree == NULL)
    {
      ET_free(expr);
      ET_free(right);
      return NULL;
    }

    expr = temp_tree;
  }

  return expr;
}

static ExprTree exponential(CList tokens, char *errmsg, size_t errmsg_sz)
{
  ExprTree ret = primary(tokens, errmsg, errmsg_sz);

  if (ret == NULL)
    return NULL;

  while (TOK_next_type(tokens) == TOK_POWER || TOK_next_type(tokens) == TOK_EQUAL)
  {
    TokenType op = TOK_next_type(tokens);
    TOK_consume(tokens);
    
    ExprTree right = (op == TOK_POWER) ? exponential(tokens, errmsg, errmsg_sz) : assignment(tokens, errmsg, errmsg_sz);

    if (right == NULL)
    {
      ET_free(ret);
      return NULL;
    }

    ExprTree temp_tree = ET_node((op == TOK_POWER) ? OP_POWER : OP_ASSIGN, ret, right);

    if (temp_tree == NULL)
    {
      ET_free(ret);
      ET_free(right);
      return NULL;
    }

    ret = temp_tree;
  }

  return ret;
}

static ExprTree primary(CList tokens, char *errmsg, size_t errmsg_sz)
{
  ExprTree ret = NULL;

  if (TOK_next_type(tokens) == TOK_VALUE)
  {
    ret = ET_value(TOK_next(tokens).t.value);
    TOK_consume(tokens);
  }
  else if (TOK_next_type(tokens) == TOK_OPEN_PAREN)
  {
    TOK_consume(tokens);
    ret = assignment(tokens, errmsg, errmsg_sz);

    if (ret == NULL)
      return NULL;

    if (TOK_next_type(tokens) != TOK_CLOSE_PAREN)
    {
      snprintf(errmsg, errmsg_sz, "Expected ')'");
      ET_free(ret);
      return NULL;
    }

    TOK_consume(tokens);
    return ret;
  }
  else if (TOK_next_type(tokens) == TOK_MINUS)
  {
    TOK_consume(tokens);
    ret = primary(tokens, errmsg, errmsg_sz);

    if (ret == NULL)
      return NULL;

    ExprTree temp_tree = ET_node(UNARY_NEGATE, ret, NULL);

    if (temp_tree == NULL)
    {
      ET_free(ret);
      return NULL;
    }

    ret = temp_tree;
  }
  else if (TOK_next_type(tokens) == TOK_SYMBOL)
  {
    ExprTree temp_tree = ET_symbol(TOK_next(tokens).t.symbol);

    if (temp_tree == NULL)
      return NULL;

    ret = temp_tree;
    TOK_consume(tokens);
  }
  else
  {
    snprintf(errmsg, errmsg_sz, "Unexpected token %s", TT_to_str(TOK_next_type(tokens)));
    ET_free(ret);
    return NULL;
  }

  return ret;
}
