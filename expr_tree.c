/*
 * expr_tree.c
 *
 * A dynamically allocated tree to handle arbitrary arithmetic
 * expressions
 *
 * Author: Howdy Pierce <howdy@sleepymoose.net>
 * Contributor: Niyomwungeri Parmenide Ishimwe <parmenin@andrew.cmu.edu>
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "expr_tree.h"

#define LEFT 0
#define RIGHT 1
#define SYMBOL_MAX_SIZE 31

struct _expr_tree_node
{
  ExprNodeType type;
  union
  {
    struct _expr_tree_node *child[2];
    double value;
    char symbol[SYMBOL_MAX_SIZE + 1];
  } n;
};

/*
 * Convert an ExprNodeType into a printable character
 *
 * Parameters:
 *   ent    The ExprNodeType to convert
 *
 * Returns: A single character representing the ent
 */
static char ExprNodeType_to_char(ExprNodeType ent)
{
  switch (ent)
  {
  case OP_SUB:
  case UNARY_NEGATE:
    return '-';
  case OP_ADD:
    return '+';
  case OP_MUL:
    return '*';
  case OP_DIV:
    return '/';
  case OP_POWER:
    return '^';
  case OP_ASSIGN:
    return '=';
  default:
    assert(0);
  }
}

// Documented in .h file
ExprTree ET_value(double value)
{
  ExprTree tree = malloc(sizeof(struct _expr_tree_node));
  assert(tree != NULL);

  tree->type = VALUE;
  tree->n.value = value;
  return tree;
}

// Documented in .h file
ExprTree ET_symbol(const char *symbol)
{
  if (symbol == NULL)
    return NULL;

  // This function should create a new type of leaf node in the ExprTree, which has the
  // ExprNodeType SYMBOL
  ExprTree tree = malloc(sizeof(struct _expr_tree_node));
  assert(tree != NULL);

  tree->type = SYMBOL;
  snprintf(tree->n.symbol, SYMBOL_MAX_SIZE + 1, "%s", symbol);

  return tree;
}

// Documented in .h file
ExprTree ET_node(ExprNodeType op, ExprTree left, ExprTree right)
{
  if (op == UNARY_NEGATE)
    assert(right == NULL);
  else
    assert(left != NULL && right != NULL);

  ExprTree tree = malloc(sizeof(struct _expr_tree_node));
  assert(tree != NULL);

  tree->type = op;
  tree->n.child[LEFT] = left;
  tree->n.child[RIGHT] = right;

  return tree;
}

// Documented in .h file
void ET_free(ExprTree tree)
{
  if (tree == NULL)
    return;

  if (tree->type != VALUE && tree->type != SYMBOL)
  {
    ET_free(tree->n.child[LEFT]);
    ET_free(tree->n.child[RIGHT]);
  }

  free(tree);
}

// Documented in .h file
int ET_count(ExprTree tree)
{
  if (tree == NULL)
    return 0;

  if (tree->type == VALUE || tree->type == SYMBOL)
    return 1;

  return 1 + ET_count(tree->n.child[LEFT]) + ET_count(tree->n.child[RIGHT]);
}

// Documented in .h file
int ET_depth(ExprTree tree)
{
  if (tree == NULL)
    return 0;

  if (tree->type == VALUE || tree->type == SYMBOL)
    return 1;

  int left = ET_depth(tree->n.child[LEFT]);
  int right = ET_depth(tree->n.child[RIGHT]);

  return 1 + (left > right ? left : right);
}

/*
 * Evaluate an ExprTree and return the resulting value
 *
 * Parameters:
 * tree The tree to compute
 * vars A dictionary containing the variables known so far, which
 * may be modified by this function
 * errmsg Return space for an error message, filled in in case of error
 * errmsg_sz The size of errmsg
 *
 * Returns: The computed value on success. If a syntax error is
 * encountered, copies an error message into errmsg and returns NaN.
 */
double ET_evaluate(ExprTree tree, CDict vars, char *errmsg, size_t errmsg_sz)
{
  if (tree == NULL)
    return 0;

  if (tree->type == VALUE)
    return tree->n.value;

  if (tree->type == SYMBOL)
    return CD_retrieve(vars, tree->n.symbol);

  double left = ET_evaluate(tree->n.child[LEFT], vars, errmsg, errmsg_sz);
  double right = ET_evaluate(tree->n.child[RIGHT], vars, errmsg, errmsg_sz);

  switch (tree->type)
  {
  case OP_ADD:
    return left + right;
  case OP_SUB:
    return left - right;
  case OP_MUL:
    return left * right;
  case OP_DIV:
    return left / right;
  case OP_POWER:
    return pow(left, right);
  case UNARY_NEGATE:
    return -left;
  case OP_ASSIGN:
    if (tree->n.child[LEFT]->type != SYMBOL)
    {
      snprintf(errmsg, errmsg_sz, "Left side of assignment must be a symbol");
      return NAN;
    }

    CD_store(vars, tree->n.child[LEFT]->n.symbol, right);
    return right;

  default:
    assert(0);
  }
}

// Documented in .h file
size_t ET_tree2string(ExprTree tree, char *buf, size_t buf_sz)
{
  if (tree == NULL || buf == NULL || buf_sz == 0)
    return 0;

  size_t length = 0;
  char leftBuffer[buf_sz];
  char rightBuffer[buf_sz];

  // write to buffer if it is a value
  if (tree->type == VALUE)
    length = snprintf(buf, buf_sz, "%g", tree->n.value);

  // write to buffer if it is a symbol
  else if (tree->type == SYMBOL)
  {
    length = snprintf(buf, buf_sz, "%s", tree->n.symbol);
  }
  else
  {
    // process the left child
    size_t leftLength = 0;

    if (tree->n.child[LEFT] != NULL)
      leftLength = ET_tree2string(tree->n.child[LEFT], leftBuffer, buf_sz);

    // print to the buffer if unary negate
    if (tree->type == UNARY_NEGATE)
      length = snprintf(buf, buf_sz, "(-%s)", leftBuffer);

    else
    {
      // process the right child
      size_t rightLength = 0;

      if (tree->n.child[RIGHT] != NULL)
        rightLength = ET_tree2string(tree->n.child[RIGHT], rightBuffer, buf_sz);

      // print to the buffer both children
      if (tree->n.child[LEFT]->type != VALUE && tree->n.child[LEFT]->type != SYMBOL)
      {
        char tempBuffer[buf_sz];
        snprintf(tempBuffer, buf_sz, "%s", leftBuffer);
        strcpy(leftBuffer, tempBuffer);
        leftLength += 2;
      }

      if (tree->n.child[RIGHT]->type != VALUE && tree->n.child[RIGHT]->type != SYMBOL)
      {
        char tempBuffer[buf_sz];
        snprintf(tempBuffer, buf_sz, "%s", rightBuffer);
        strcpy(rightBuffer, tempBuffer);
        rightLength += 2;
      }

      // finally print to the buffer the whole expression
      length = snprintf(buf, buf_sz, "(%s %c %s)", leftBuffer, ExprNodeType_to_char(tree->type), rightBuffer);
    }
  }

  // truncate the string if it is too long for the buffer
  if (length >= buf_sz)
  {
    buf[buf_sz - 2] = '$';
    buf[buf_sz - 1] = '\0';
    return buf_sz - 1;
  }

  buf[length] = '\0';
  return length;
}