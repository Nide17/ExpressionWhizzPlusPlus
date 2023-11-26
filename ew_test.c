/*
 * token_test.c
 *
 * Tests for tokenization functions
 *
 * Author: Howdy Pierce <howdy@sleepymoose.net>
 * Contributor: Niyomwungeri Parmenide Ishimwe <parmenin@andrew.cmu.edu>
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h> // strlen
#include <ctype.h>  // isblank
#include <math.h>   // fabs
#include <stdbool.h>

#include "clist.h"
#include "token.h"
#include "tokenize.h"
#include "expr_tree.h"
#include "parse.h"
#include "cdict.h"

// If value is not true; prints a failure message and returns 0.
#define test_assert(value)                                         \
  {                                                                \
    if (!(value))                                                  \
    {                                                              \
      printf("FAIL %s[%d]: %s\n", __FUNCTION__, __LINE__, #value); \
      goto test_error;                                             \
    }                                                              \
  }

Token tokens[] = {{TOK_VALUE, {2}}, {TOK_PLUS}, {TOK_MINUS}, {TOK_MULTIPLY}, {TOK_DIVIDE}, {TOK_POWER}, {TOK_OPEN_PAREN}, {TOK_CLOSE_PAREN}, {TOK_END}, {TOK_DIVIDE}, {TOK_POWER}};

const int num_tokens = sizeof(tokens) / sizeof(tokens[0]);

/*
 * Returns true if tok1 and tok2 compare equally, false otherwise
 */
static bool test_tok_eq(Token tok1, Token tok2)
{
  if (tok1.type != tok2.type)
    return false;

  if (tok1.type == TOK_VALUE && fabs(tok1.t.value - tok2.t.value) > 0.0001)
    return false;

  return true;
}

struct inspect_data
{
  int num_calls;
  int num_correct;
};

void inspect_element(int pos, CListElementType element, void *data)
{
  struct inspect_data *id = (struct inspect_data *)data;

  id->num_calls++;
  if (test_tok_eq(element, tokens[pos]))
    id->num_correct++;
}

/*
 * Tests the CL_new, CL_free, CL_append, CL_length, CL_nth, CL_foreach
 * and CL_pop functions, using struct token type. This should give you
 * some assurance that your CList is working correctly.
 *
 * Returns: 1 if all tests pass, 0 otherwise
 */
int test_cl_token()
{
  struct inspect_data id = {0, 0};
  CList list = CL_new();

  for (int i = 0; i < num_tokens; i++)
  {
    CL_append(list, tokens[i]);
    test_assert(CL_length(list) == i + 1);
    test_assert(test_tok_eq(CL_nth(list, i), tokens[i]));
  }

  // test foreach
  CL_foreach(list, inspect_element, &id);
  test_assert(id.num_calls == num_tokens);
  test_assert(id.num_correct == num_tokens);

  // pop everything off, make sure that works
  for (int i = 0; i < num_tokens; i++)
    test_assert(test_tok_eq(CL_pop(list), tokens[i]));

  test_assert(CL_length(list) == 0);

  CL_free(list);
  return 1;

test_error:
  CL_free(list);
  return 0;
}

/*
 * Exactly like strcmp, but ignores spaces.  Therefore the following
 * strings compare alike: "ab", " ab", "  a  b  ", "a b"
 *
 * Parameters:
 *   s1, s2    The strings to be compared
 *
 * Returns: -1, 0, or 1 depending on whether s1 sorts before, equal
 * to, or after s2.
 */
static int strcmp_sp(const char *s1, const char *s2)
{
  // advance past leading spaces
  while (isblank(*s1))
    s1++;
  while (isblank(*s2))
    s2++;

  while (*s1 != '\0')
  {
    if (isblank(*s1))
      s1++;
    else if (isblank(*s2))
      s2++;
    else if (*s1 != *s2)
      return (*s1 - *s2);
    else
    {
      s1++;
      s2++;
    }
  }

  while (isblank(*s2))
    s2++;

  return (*s1 - *s2);
}

/*
 * Tests the ET_node, ET_value, ET_tree2string, ET_depth, and
 * ET_evaluate functions. This should provide some assurance that your
 * ExprTree module is working correctly.
 *
 * Returns: 1 if all tests pass, 0 otherwise
 */
int test_expr_tree()
{
  ExprTree tree = NULL;
  char buffer[1024];
  double result;
  size_t len;
  int depth;
  const double value = 0.125;
  char errmsg[128];
  CDict vars = CD_new();

  CD_store(vars, "x", 0.8);
  CD_store(vars, "y", 0.2);

  // x + y
  tree = ET_node(OP_ADD, ET_symbol("x"), ET_symbol("y"));
  len = ET_tree2string(tree, buffer, sizeof(buffer));
  result = ET_evaluate(tree, vars, errmsg, sizeof(errmsg));
  test_assert(result == 1);
  depth = ET_depth(tree);
  ET_free(tree);

  // t = x
  tree = ET_node(OP_ASSIGN, ET_symbol("t"), ET_symbol("x"));
  len = ET_tree2string(tree, buffer, sizeof(buffer));
  result = ET_evaluate(tree, vars, errmsg, sizeof(errmsg));
  test_assert(result == 0.8);
  depth = ET_depth(tree);
  ET_free(tree);

  // g = 25
  tree = ET_node(OP_ASSIGN, ET_symbol("g"), ET_value(25));
  len = ET_tree2string(tree, buffer, sizeof(buffer));
  result = ET_evaluate(tree, vars, errmsg, sizeof(errmsg));
  test_assert(result == 25);
  depth = ET_depth(tree);
  ET_free(tree);

  // x = x - y
  tree = ET_node(OP_ASSIGN, ET_symbol("x"), ET_node(OP_SUB, ET_symbol("x"), ET_symbol("y")));
  len = ET_tree2string(tree, buffer, sizeof(buffer));
  result = ET_evaluate(tree, vars, errmsg, sizeof(errmsg));
  test_assert(fabs(result - 0.6) < 0.0001);
  depth = ET_depth(tree);
  ET_free(tree);

  // x
  tree = ET_symbol("x");
  len = ET_tree2string(tree, buffer, sizeof(buffer));
  result = ET_evaluate(tree, vars, errmsg, sizeof(errmsg));
  test_assert(fabs(result - 0.6) < 0.0001);
  depth = ET_depth(tree);
  ET_free(tree);

  // pi->undefined
  tree = ET_symbol("pi");
  len = ET_tree2string(tree, buffer, sizeof(buffer));
  result = ET_evaluate(tree, vars, errmsg, sizeof(errmsg));
  test_assert(isnan(result));
  test_assert(strcasecmp(errmsg, "Undefined variable: pi") == 0);
  depth = ET_depth(tree);
  ET_free(tree);

  // -(0.125) (using unary negation)
  tree = ET_node(UNARY_NEGATE, ET_value(value), NULL);
  len = ET_tree2string(tree, buffer, sizeof(buffer));
  result = ET_evaluate(tree, vars, errmsg, sizeof(errmsg));
  depth = ET_depth(tree);

  test_assert(result == -value);
  test_assert(strcmp_sp(buffer, "(-0.125)") == 0);
  test_assert(strlen(buffer) == len);
  test_assert(depth == 2);

  // double unary negation: (- (-0.125)))
  tree = ET_node(UNARY_NEGATE, tree, NULL);
  len = ET_tree2string(tree, buffer, sizeof(buffer));
  result = ET_evaluate(tree, vars, errmsg, sizeof(errmsg));
  depth = ET_depth(tree);

  test_assert(result == value);
  test_assert(strcmp_sp(buffer, "(-(-0.125))") == 0);
  test_assert(strlen(buffer) == len);
  test_assert(depth == 3);

  ET_free(tree);

  // 6.5 * (4 + 3)
  tree = ET_node(OP_ADD, ET_value(4), ET_value(3));
  tree = ET_node(OP_MUL, ET_value(6.5), tree);
  len = ET_tree2string(tree, buffer, sizeof(buffer));
  result = ET_evaluate(tree, vars, errmsg, sizeof(errmsg));
  depth = ET_depth(tree);

  test_assert(result == 45.5);
  test_assert(strcmp_sp(buffer, "(6.5 * (4 + 3))") == 0);
  test_assert(strlen(buffer) == len);
  test_assert(depth == 3);

  ET_free(tree);
  CD_free(vars);
  return 1;

test_error:
  ET_free(tree);
  CD_free(vars);
  return 0;
}

/*
 * Tests the TOK_next_type and TOK_consume functions
 *
 * Returns: 1 if all tests pass, 0 otherwise
 */
int test_tok_next_consume()
{
  CList list = CL_new();

  for (int i = 0; i < num_tokens; i++)
  {
    CL_append(list, tokens[i]);
    test_assert(CL_length(list) == i + 1);
    test_assert(test_tok_eq(CL_nth(list, i), tokens[i]));
  }

  for (int i = 0; i < num_tokens; i++)
  {
    test_assert(TOK_next_type(list) == tokens[i].type);
    TOK_consume(list);
  }

  test_assert(CL_length(list) == 0);

  test_assert(TOK_next_type(list) == TOK_END);
  TOK_consume(list);
  test_assert(TOK_next_type(list) == TOK_END);
  TOK_consume(list);
  test_assert(TOK_next_type(list) == TOK_END);
  TOK_consume(list);

  CL_free(list);
  return 1;

test_error:
  CL_free(list);
  return 0;
}

/*
 * Tests the TOK_tokenize_input function
 *
 * Returns: 1 if all tests pass, 0 otherwise
 */
int test_tokenize_input()
{
  char errmsg[128];
  CList list = NULL;

  list = TOK_tokenize_input("3", errmsg, sizeof(errmsg));
  test_assert(CL_length(list) == 1);
  test_assert(test_tok_eq(CL_nth(list, 0), (Token){TOK_VALUE, {3}}));
  CL_free(list);

  list = TOK_tokenize_input("3 + 2", errmsg, sizeof(errmsg));
  test_assert(CL_length(list) == 3);
  test_assert(test_tok_eq(CL_nth(list, 0), (Token){TOK_VALUE, {3}}));
  test_assert(test_tok_eq(CL_nth(list, 1), (Token){TOK_PLUS}));
  test_assert(test_tok_eq(CL_nth(list, 2), (Token){TOK_VALUE, {2}}));
  CL_free(list);

  list = TOK_tokenize_input("0x3p+2", errmsg, sizeof(errmsg));
  test_assert(CL_length(list) == 1);
  test_assert(test_tok_eq(CL_nth(list, 0), (Token){TOK_VALUE, {12}}));
  CL_free(list);

  list = TOK_tokenize_input("3pi", errmsg, sizeof(errmsg));
  test_assert(CL_length(list) == 2);
  CL_free(list);

  list = TOK_tokenize_input("make", errmsg, sizeof(errmsg));
  test_assert(CL_length(list) == 1);
  CL_free(list);

  list = TOK_tokenize_input("1258make111", errmsg, sizeof(errmsg));
  test_assert(CL_length(list) == 2);
  CL_free(list);

  // long symbol
  list = TOK_tokenize_input("makeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee", errmsg, sizeof(errmsg));
  test_assert(CL_length(list) == 0);
  test_assert(strcasecmp(errmsg, "Position 32: symbol too long") == 0);
  CL_free(list);

  list = TOK_tokenize_input("(3 + 2)", errmsg, sizeof(errmsg));
  test_assert(CL_length(list) == 5);
  test_assert(test_tok_eq(CL_nth(list, 0), (Token){TOK_OPEN_PAREN}));
  test_assert(test_tok_eq(CL_nth(list, 1), (Token){TOK_VALUE, {3}}));
  test_assert(test_tok_eq(CL_nth(list, 2), (Token){TOK_PLUS}));
  test_assert(test_tok_eq(CL_nth(list, 3), (Token){TOK_VALUE, {2}}));
  test_assert(test_tok_eq(CL_nth(list, 4), (Token){TOK_CLOSE_PAREN}));
  CL_free(list);

  list = TOK_tokenize_input("3 + 2)", errmsg, sizeof(errmsg));
  test_assert(CL_length(list) == 4);
  test_assert(test_tok_eq(CL_nth(list, 0), (Token){TOK_VALUE, {3}}));
  test_assert(test_tok_eq(CL_nth(list, 1), (Token){TOK_PLUS}));
  test_assert(test_tok_eq(CL_nth(list, 2), (Token){TOK_VALUE, {2}}));
  test_assert(test_tok_eq(CL_nth(list, 3), (Token){TOK_CLOSE_PAREN}));
  CL_free(list);

  list = TOK_tokenize_input("3 + (2*", errmsg, sizeof(errmsg));
  test_assert(CL_length(list) == 5);
  test_assert(test_tok_eq(CL_nth(list, 0), (Token){TOK_VALUE, {3}}));
  test_assert(test_tok_eq(CL_nth(list, 1), (Token){TOK_PLUS}));
  test_assert(test_tok_eq(CL_nth(list, 2), (Token){TOK_OPEN_PAREN}));
  test_assert(test_tok_eq(CL_nth(list, 3), (Token){TOK_VALUE, {2}}));
  test_assert(test_tok_eq(CL_nth(list, 4), (Token){TOK_MULTIPLY}));
  CL_free(list);

  //  2 ^ ( 1.5 * 2 ) / ( - 1.7 + ( 6 - 0.3 ) )
  list = TOK_tokenize_input("2^(1.5*2)/(-1.7+(6-0.3))", errmsg, sizeof(errmsg));
  test_assert(CL_length(list) == 18);
  test_assert(test_tok_eq(CL_nth(list, 0), (Token){TOK_VALUE, {2}}));
  test_assert(test_tok_eq(CL_nth(list, 1), (Token){TOK_POWER}));
  test_assert(test_tok_eq(CL_nth(list, 2), (Token){TOK_OPEN_PAREN}));
  test_assert(test_tok_eq(CL_nth(list, 3), (Token){TOK_VALUE, {1.5}}));
  test_assert(test_tok_eq(CL_nth(list, 4), (Token){TOK_MULTIPLY}));
  test_assert(test_tok_eq(CL_nth(list, 5), (Token){TOK_VALUE, {2}}));
  test_assert(test_tok_eq(CL_nth(list, 6), (Token){TOK_CLOSE_PAREN}));
  test_assert(test_tok_eq(CL_nth(list, 7), (Token){TOK_DIVIDE}));
  test_assert(test_tok_eq(CL_nth(list, 8), (Token){TOK_OPEN_PAREN}));
  test_assert(test_tok_eq(CL_nth(list, 9), (Token){TOK_MINUS}));
  test_assert(test_tok_eq(CL_nth(list, 10), (Token){TOK_VALUE, {1.7}}));
  test_assert(test_tok_eq(CL_nth(list, 11), (Token){TOK_PLUS}));
  test_assert(test_tok_eq(CL_nth(list, 12), (Token){TOK_OPEN_PAREN}));
  test_assert(test_tok_eq(CL_nth(list, 13), (Token){TOK_VALUE, {6}}));
  test_assert(test_tok_eq(CL_nth(list, 14), (Token){TOK_MINUS}));
  test_assert(test_tok_eq(CL_nth(list, 15), (Token){TOK_VALUE, {0.3}}));
  test_assert(test_tok_eq(CL_nth(list, 16), (Token){TOK_CLOSE_PAREN}));
  test_assert(test_tok_eq(CL_nth(list, 17), (Token){TOK_CLOSE_PAREN}));
  CL_free(list);


  return 1;


test_error:
  CL_free(list);
  return 0;
}

/*
 * Runs the parser on one test case, and checks that the resultant
 * ExprTree matches the expected results for both depth and evaluated
 * value.
 *
 * Parameters:
 *   exp_value    The value we expect
 *   exp_depth    The expected depth of the ExprTree
 *   token_arr    An array of tokens, which must be terminated with TOK_END
 *
 * Returns: 1 if the test passes, 0 otherwise
 */
int test_parse_once(double exp_value, int exp_depth, const Token token_arr[])
{
  CList tokens = NULL;
  ExprTree tree = NULL;
  CDict vars = CD_new();
  char errmsg[256];

  tokens = CL_new();

  for (int i = 0; token_arr[i].type != TOK_END; i++)
    CL_append(tokens, token_arr[i]);

  tree = Parse(tokens, errmsg, sizeof(errmsg));
  TOK_print(tokens);

  test_assert(ET_depth(tree) == exp_depth);
  test_assert(fabs(ET_evaluate(tree, vars, errmsg, sizeof(errmsg)) - exp_value) < 0.0001);

  CL_free(tokens);
  ET_free(tree);
  CD_free(vars);

  return 1;

test_error:
  CL_free(tokens);
  ET_free(tree);
  CD_free(vars);
  return 0;
}

/*
 * Tests the recursive descent parser
 *
 * Returns: 1 if all tests pass, 0 otherwise
 */
int test_parse()
{
  test_assert(test_parse_once(3.5, 1, (Token[]){{TOK_VALUE, {3.5}}, {TOK_END}}));
  test_assert(test_parse_once(3.5, 2, (Token[]){{TOK_VALUE, {3.5}}, {TOK_PLUS}, {TOK_VALUE, {0}}, {TOK_END}}));
  test_assert(test_parse_once(3.5, 3, (Token[]){{TOK_VALUE, {3.5}}, {TOK_PLUS}, {TOK_VALUE, {0}}, {TOK_PLUS}, {TOK_VALUE, {0}}, {TOK_END}}));
  test_assert(test_parse_once(3.5, 4, (Token[]){{TOK_VALUE, {3.5}}, {TOK_PLUS}, {TOK_VALUE, {0}}, {TOK_PLUS}, {TOK_VALUE, {0}}, {TOK_PLUS}, {TOK_VALUE, {0}}, {TOK_END}}));
  test_assert(test_parse_once(3.5, 5, (Token[]){{TOK_VALUE, {3.5}}, {TOK_PLUS}, {TOK_VALUE, {0}}, {TOK_PLUS}, {TOK_VALUE, {0}}, {TOK_PLUS}, {TOK_VALUE, {0}}, {TOK_PLUS}, {TOK_VALUE, {0}}, {TOK_END}}));

  test_assert(test_parse_once(0, 0, (Token[]){{TOK_END}}));
  test_assert(test_parse_once(0, 1, (Token[]){{TOK_VALUE, {0}}, {TOK_END}}));
  test_assert(test_parse_once(0, 2, (Token[]){{TOK_VALUE, {0}}, {TOK_PLUS}, {TOK_VALUE, {0}}, {TOK_END}}));
  test_assert(test_parse_once(0, 3, (Token[]){{TOK_VALUE, {0}}, {TOK_PLUS}, {TOK_VALUE, {0}}, {TOK_PLUS}, {TOK_VALUE, {0}}, {TOK_END}}));
  // test_assert(test_parse_once(3.5, 2, (Token[]){{TOK_VALUE, {3.5}}, {TOK_PLUS}, {TOK_PLUS}, {TOK_VALUE, {0}}, {TOK_END}}));

  return 1;

test_error:
  return 0;
}

/*
 * Tests the recursive descent parser for proper associativity: + - *
 * / are left-associative, whereas ^ is right-associative
 *
 * Returns: 1 if all tests pass, 0 otherwise
 */
int test_parse_associativity()
{
  test_assert(test_parse_once(5, 3, (Token[]){{TOK_VALUE, {10}}, {TOK_MINUS}, {TOK_VALUE, {2}}, {TOK_MINUS}, {TOK_VALUE, {3}}, {TOK_END}}));
  test_assert(test_parse_once(1, 4, (Token[]){{TOK_VALUE, {10}}, {TOK_MINUS}, {TOK_VALUE, {2}}, {TOK_MINUS}, {TOK_VALUE, {3}}, {TOK_MINUS}, {TOK_VALUE, {4}}, {TOK_END}}));
  test_assert(test_parse_once(-4, 5, (Token[]){{TOK_VALUE, {10}}, {TOK_MINUS}, {TOK_VALUE, {2}}, {TOK_MINUS}, {TOK_VALUE, {3}}, {TOK_MINUS}, {TOK_VALUE, {4}}, {TOK_MINUS}, {TOK_VALUE, {5}}, {TOK_END}}));
  test_assert(test_parse_once(1, 3, (Token[]){{TOK_VALUE, {10}}, {TOK_DIVIDE}, {TOK_VALUE, {2}}, {TOK_DIVIDE}, {TOK_VALUE, {5}}, {TOK_END}}));

  test_assert(test_parse_once(10, 3, (Token[]){{TOK_VALUE, {2}}, {TOK_PLUS}, {TOK_VALUE, {3}}, {TOK_PLUS}, {TOK_VALUE, {5}}, {TOK_END}}));
  test_assert(test_parse_once(10, 4, (Token[]){{TOK_VALUE, {2}}, {TOK_PLUS}, {TOK_VALUE, {3}}, {TOK_PLUS}, {TOK_VALUE, {1}}, {TOK_PLUS}, {TOK_VALUE, {4}}, {TOK_END}}));
  test_assert(test_parse_once(12, 5, (Token[]){{TOK_VALUE, {2}}, {TOK_PLUS}, {TOK_VALUE, {3}}, {TOK_PLUS}, {TOK_VALUE, {1}}, {TOK_PLUS}, {TOK_VALUE, {4}}, {TOK_PLUS}, {TOK_VALUE, {2}}, {TOK_END}}));

  return 1;

test_error:
  return 0;
}

/*
 * Tests the recursive descent parser with erroneous inputs
 *
 * Returns: 1 if all tests pass, 0 otherwise
 */
int test_parse_errors()
{

  char errmsg[128];
  CList tokens = NULL;
  ExprTree tree = NULL;
  CDict vars = CD_new();

  tokens = TOK_tokenize_input("3 + 2", errmsg, sizeof(errmsg));
  test_assert(CL_length(tokens) == 3);
  tree = Parse(tokens, errmsg, sizeof(errmsg));
  test_assert(tree != NULL);
  ET_free(tree);
  CL_free(tokens);

  tokens = TOK_tokenize_input("2 + 3 * 2", errmsg, sizeof(errmsg));
  test_assert(CL_length(tokens) == 5);
  tree = Parse(tokens, errmsg, sizeof(errmsg));
  test_assert(tree != NULL);
  ET_free(tree);
  CL_free(tokens);

  tokens = TOK_tokenize_input("3 + 2)", errmsg, sizeof(errmsg));
  test_assert(CL_length(tokens) == 4);
  test_assert(Parse(tokens, errmsg, sizeof(errmsg)) == NULL);
  test_assert(strcasecmp(errmsg, "Syntax error on token CLOSE_PAREN") == 0);
  CL_free(tokens);

  tokens = TOK_tokenize_input("2++3", errmsg, sizeof(errmsg));
  test_assert(CL_length(tokens) == 4);
  test_assert(Parse(tokens, errmsg, sizeof(errmsg)) == NULL);
  test_assert(strcasecmp(errmsg, "Unexpected token PLUS") == 0);
  CL_free(tokens);

  tokens = TOK_tokenize_input("3 + (2*", errmsg, sizeof(errmsg));
  test_assert(CL_length(tokens) == 5);
  test_assert(Parse(tokens, errmsg, sizeof(errmsg)) == NULL);
  test_assert(strcasecmp(errmsg, "Unexpected token (end)") == 0);
  CL_free(tokens);

  tokens = TOK_tokenize_input("3 +) 2", errmsg, sizeof(errmsg));
  test_assert(CL_length(tokens) == 4);
  test_assert(Parse(tokens, errmsg, sizeof(errmsg)) == NULL);
  test_assert(strcasecmp(errmsg, "Unexpected token CLOSE_PAREN") == 0);
  CL_free(tokens);

  tokens = TOK_tokenize_input("1 + 2 (", errmsg, sizeof(errmsg));
  test_assert(CL_length(tokens) == 4);
  test_assert(Parse(tokens, errmsg, sizeof(errmsg)) == NULL);
  test_assert(strcasecmp(errmsg, "Syntax error on token OPEN_PAREN") == 0);
  CL_free(tokens);

  // (((33))) + 6
  tokens = TOK_tokenize_input("(((33))) + 6", errmsg, sizeof(errmsg));
  test_assert(CL_length(tokens) == 9);
  tree = Parse(tokens, errmsg, sizeof(errmsg));
  test_assert(tree != NULL);
  ET_free(tree);
  CL_free(tokens);

  // 3e10 / 10^10
  tokens = TOK_tokenize_input("3e10 / 10^10", errmsg, sizeof(errmsg));
  test_assert(CL_length(tokens) == 5);
  tree = Parse(tokens, errmsg, sizeof(errmsg));
  test_assert(tree != NULL);
  ET_free(tree);
  CL_free(tokens);

  // -1^2
  tokens = TOK_tokenize_input("-1^2", errmsg, sizeof(errmsg));
  test_assert(CL_length(tokens) == 4);
  tree = Parse(tokens, errmsg, sizeof(errmsg));
  test_assert(tree != NULL);
  ET_free(tree);
  CL_free(tokens);

  // x = 3
  tokens = TOK_tokenize_input("x = 25", errmsg, sizeof(errmsg));
  CD_store(vars, "x", 25);
  test_assert(CL_length(tokens) == 3);
  tree = Parse(tokens, errmsg, sizeof(errmsg));
  test_assert(tree != NULL);
  ET_tree2string(tree, errmsg, sizeof(errmsg));
  double result = ET_evaluate(tree, vars, errmsg, sizeof(errmsg));
  test_assert(result == 25);
  ET_free(tree);
  CL_free(tokens);

  tokens = TOK_tokenize_input("sine", errmsg, sizeof(errmsg));
  test_assert(CL_length(tokens) == 1);
  tree = Parse(tokens, errmsg, sizeof(errmsg));
  test_assert(tree != NULL);
  CL_free(tokens);
  ET_free(tree);

  tokens = TOK_tokenize_input("((2+3)*5)/(4-1)", errmsg, sizeof(errmsg));
  test_assert(CL_length(tokens) == 15);
  tree = Parse(tokens, errmsg, sizeof(errmsg));
  test_assert(tree != NULL);
  ET_free(tree);
  CL_free(tokens);

  tokens = TOK_tokenize_input("-(-2)^2", errmsg, sizeof(errmsg));
  test_assert(CL_length(tokens) == 7);
  tree = Parse(tokens, errmsg, sizeof(errmsg));
  test_assert(tree != NULL);
  ET_free(tree);
  CL_free(tokens);

  tokens = TOK_tokenize_input("2 + a * 3", errmsg, sizeof(errmsg));
  test_assert(CL_length(tokens) == 5);
  tree = Parse(tokens, errmsg, sizeof(errmsg));
  test_assert(tree != NULL);
  ET_free(tree);
  CL_free(tokens);

  tokens = TOK_tokenize_input("2 + * 3", errmsg, sizeof(errmsg));
  test_assert(CL_length(tokens) == 4);
  test_assert(Parse(tokens, errmsg, sizeof(errmsg)) == NULL);
  test_assert(strcasecmp(errmsg, "Unexpected token MULTIPLY") == 0);
  CL_free(tokens);

  tokens = TOK_tokenize_input("((((2+3)*5)/(4-1)))", errmsg, sizeof(errmsg));
  test_assert(CL_length(tokens) == 19);
  tree = Parse(tokens, errmsg, sizeof(errmsg));
  test_assert(tree != NULL);
  ET_free(tree);
  CL_free(tokens);

  //  2 ^ ( 1.5 * 2 ) / ( - 1.7 + ( 6 - 0.3 ) )
  tokens = TOK_tokenize_input("2^(1.5*2)/(-1.7+(6-0.3))", errmsg, sizeof(errmsg));
  test_assert(CL_length(tokens) == 18);
  tree = Parse(tokens, errmsg, sizeof(errmsg));
  test_assert(tree != NULL);
  ET_free(tree);
  CL_free(tokens);

  tokens = TOK_tokenize_input("3+4*2/(1-5)^2", errmsg, sizeof(errmsg));
  test_assert(CL_length(tokens) == 13);
  tree = Parse(tokens, errmsg, sizeof(errmsg));
  test_assert(tree != NULL);
  ET_free(tree);
  CL_free(tokens);

  tokens = TOK_tokenize_input("1234567890+9876543210*1234567890", errmsg, sizeof(errmsg));
  test_assert(CL_length(tokens) == 5);
  tree = Parse(tokens, errmsg, sizeof(errmsg));
  test_assert(tree != NULL);
  ET_free(tree);
  CL_free(tokens);

  // 2 ^ ( 1.5 * 2 ) / ( - 1.7 + ( 6 - 0.3 ) )
  tokens = TOK_tokenize_input(" 2 ^ ( 1.5 * 2 ) / ( -1.7 + ( 6 - 0.3 ) ) ", errmsg, sizeof(errmsg));
  test_assert(CL_length(tokens) == 18);
  tree = Parse(tokens, errmsg, sizeof(errmsg));
  test_assert(tree != NULL);
  ET_free(tree);
  CL_free(tokens);
  
  //1111111111111111111111111111111
  tokens = TOK_tokenize_input("111111111111111111111111111111111", errmsg, sizeof(errmsg));
  test_assert(CL_length(tokens) == 1);
  tree = Parse(tokens, errmsg, sizeof(errmsg));
  test_assert(tree != NULL);
  ET_free(tree);
  CL_free(tokens);

  tokens = TOK_tokenize_input("-(-2)^3", errmsg, sizeof(errmsg));
  test_assert(CL_length(tokens) == 7);
  tree = Parse(tokens, errmsg, sizeof(errmsg));
  test_assert(tree != NULL);
  CL_free(tokens);
  ET_free(tree);

  CD_free(vars);
  return 1;

test_error:
  CL_free(tokens);
  ET_free(tree);
  CD_free(vars);
  return 0;
}

int main()
{
  int passed = 0;
  int num_tests = 0;

  num_tests++;
  passed += test_cl_token();
  num_tests++;
  passed += test_expr_tree();
  num_tests++;
  passed += test_tok_next_consume();
  num_tests++;
  passed += test_tokenize_input();
  num_tests++;
  passed += test_parse();
  num_tests++;
  passed += test_parse_associativity();
  num_tests++;
  passed += test_parse_errors();

  printf("Passed %d/%d test cases\n", passed, num_tests);
  fflush(stdout);
  return 0;
}
