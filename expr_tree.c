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
ExprTree ET_node(ExprNodeType op, ExprTree left, ExprTree right)
{
  if (op == UNARY_NEGATE)
    assert(right == NULL);
  else
    assert(left != NULL && right != NULL);

  ExprTree tree = malloc(sizeof(struct _expr_tree_node));
  tree->type = op;
  tree->n.child[LEFT] = left;
  tree->n.child[RIGHT] = right;
  return tree;
}

// Documented in .h file
ExprTree ET_symbol(const char *symbol)
{
  // This function should create a new type of leaf node in the ExprTree, which has the
  // ExprNodeType SYMBOL
  ExprTree tree = malloc(sizeof(struct _expr_tree_node));

  tree->type = SYMBOL;
  snprintf(tree->n.symbol, SYMBOL_MAX_SIZE + 1, "%s", symbol);

  return tree;
}

// Documented in .h file
void ET_free(ExprTree tree)
{
  if (tree == NULL)
    return;

  if (tree->type != VALUE)
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

  if (tree->type == VALUE)
    return 1;

  return 1 + ET_count(tree->n.child[LEFT]) + ET_count(tree->n.child[RIGHT]);
}

// Documented in .h file
int ET_depth(ExprTree tree)
{
  if (tree == NULL)
    return 0;

  if (tree->type == VALUE)
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
  case SYMBOL:
    if (CD_contains(vars, tree->n.symbol))
      return CD_retrieve(vars, tree->n.symbol); // check if this is the right way to retrieve the value
    else
    {
      snprintf(errmsg, errmsg_sz, "Unknown variable: %s", tree->n.symbol);
      return NAN;
    }
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
  else
  {
    // process the left child
    size_t leftLength = ET_tree2string(tree->n.child[LEFT], leftBuffer, buf_sz);

    // print to the buffer if unary negate
    if (tree->type == UNARY_NEGATE)
      length = snprintf(buf, buf_sz, "(-%s)", leftBuffer);

    else
    {
      // process the right child
      size_t rightLength = ET_tree2string(tree->n.child[RIGHT], rightBuffer, buf_sz);

      // print to the buffer both children
      if (tree->n.child[LEFT]->type != VALUE)
      {
        char tempBuffer[buf_sz];
        snprintf(tempBuffer, buf_sz, "%s", leftBuffer);
        strcpy(leftBuffer, tempBuffer);
        leftLength += 2;
      }

      if (tree->n.child[RIGHT]->type != VALUE)
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

parmenin @scottyone : ~ / Assignments / 11 $ scottycheck isse - 11 *.c *.h
//                                                                     Extracting files... Good job : Code compiled without errors

//                                                                                                        Error : Your program crashed :

//     AddressSanitizer : DEADLYSIGNAL ==
//     == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == ==
//     =
//         == 2674297 == ERROR : AddressSanitizer : SEGV on unknown address(pc 0x56498c32c2a4 bp 0x7ffe8e3a60e0 sp 0x7ffe8e3a56e0 T0) == 2674297 == The signal is caused by a READ memory access.== 2674297 == Hint : this fault was caused by a dereference of a high value address(see register values below).Dissassemble the provided pc to learn which register was used.
// # 0 0x56498c32c2a4 in ET_tree2string(/ var / local / scottycheck / isse - 11 / parmenin / 1122 - 134413 / a.out + 0xb2a4)
// # 1 0x56498c32c37c in ET_tree2string(/ var / local / scottycheck / isse - 11 / parmenin / 1122 - 134413 / a.out + 0xb37c)
// # 2 0x56498c32c37c in ET_tree2string(/ var / local / scottycheck / isse - 11 / parmenin / 1122 - 134413 / a.out + 0xb37c)
// # 3 0x56498c32c43d in ET_tree2string(/ var / local / scottycheck / isse - 11 / parmenin / 1122 - 134413 / a.out + 0xb43d)
// # 4 0x56498c330eb9 in test_expr_tree(/ var / local / scottycheck / isse - 11 / parmenin / 1122 - 134413 / a.out + 0xfeb9)
// # 5 0x56498c33d95a in main(/ var / local / scottycheck / isse - 11 / parmenin / 1122 - 134413 / a.out + 0x1c95a)
// # 6 0x7fddf4629d8f in __libc_start_call_main../ sysdeps / nptl / libc_start_call_main.h : 58
// # 7 0x7fddf4629e3f in __libc_start_main_impl../ csu / libc - start.c : 392
// # 8 0x56498c327624 in _start(/ var / local / scottycheck / isse - 11 / parmenin / 1122 - 134413 / a.out + 0x6624)

//                                                                                                                                                                                                                    AddressSanitizer can not provide additional info.SUMMARY : AddressSanitizer : SEGV(/ var / local / scottycheck / isse - 11 / parmenin / 1122 - 134413 / a.out + 0xb2a4) in ET_tree2string == 2674297 == ABORTING