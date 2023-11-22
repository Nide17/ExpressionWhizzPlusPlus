/*
 * expr_tree.h
 *
 * A dynamically allocated tree to handle arbitrary expressions
 *
 * Author: Howdy Pierce <howdy@sleepymoose.net>
 * Contributor: Niyomwungeri Parmenide Ishimwe <parmenin@andrew.cmu.edu>
 */

#ifndef _EXPR_TREE_H_
#define _EXPR_TREE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "cdict.h"

typedef struct _expr_tree_node *ExprTree;

typedef enum
{
  VALUE,
  SYMBOL,
  UNARY_NEGATE,
  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,
  OP_POWER,
  OP_ASSIGN
} ExprNodeType;

/*
 * Create a value node on the tree. A value node is always a leaf.
 *
 * Parameters:
 *   value    The value for the leaf node
 *
 * Returns:
 *   The new tree, which will consist of a single leaf node
 *
 * It is the responsibility of the caller to call ET_free on a tree
 * that contains this leaf.
 */
ExprTree ET_value(double value);

/*
 * Create an interior node on tree. An interior node always represents
 * an arithmetic operation.
 *
 * Parameters:
 *   op       The operator
 *   left     Left side of the operator
 *   right    Right side of the operator
 *
 * Returns: The new tree, which will consist of an interior node with
 *   two children.
 *
 * It is the responsibility of the caller to call ET_free on a tree
 * that contains this leaf
 */
ExprTree ET_node(ExprNodeType op, ExprTree left, ExprTree right);


/*
 * Create a symbol node on the tree. A symbol node is always a leaf.
 *
 * Parameters:
 *   symbol    The symbol for the leaf node
 *
 * Returns:
 *   The new tree, which will consist of a single leaf node
 *
 * It is the responsibility of the caller to call ET_free on a tree
 * that contains this leaf.
 */
ExprTree ET_symbol(const char *symbol);

/*
 * Destroy an ExprTree, calling free() on all malloc'd memory
 *
 * Parameters:
 *   tree     The tree
 *
 * Returns: None
 */
void ET_free(ExprTree tree);

/*
 * Return the number of nodes in the tree, including both leaf and
 * interior nodes in the count.
 *
 * Parameters:
 *   tree     The tree
 *
 * Returns: The number of nodes
 */
int ET_count(ExprTree tree);

/*
 * Return the maximum depth for the tree. A tree that contains just a
 * single leaf node has a depth of 1.
 *
 * Parameters:
 *   tree     The tree
 *
 * Returns: The maximum depth.
 */
int ET_depth(ExprTree tree);

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
double ET_evaluate(ExprTree tree, CDict vars, char *errmsg, size_t errmsg_sz);

/*
 * Convert an ExprTree into a printable ASCII string stored in buf
 * Uses a recursive approach, converting the child nodes to strings first
 * and then formatting the parent node.
 *
 * Parameters:
 *   tree     The tree
 *   buf      The buffer
 *   buf_sz   Size of buffer, in bytes
 *
 * If it takes more characters to represent tree than are allowed in
 * buf, this function places as many characters as will fit into buf,
 * followed by a '$' character to indicate the result was truncated.
 * Buf will always be terminated with a \0 and in no case will this
 * function write characters beyond the end of buf.
 *
 * Returns: The number of characters written to buf, not counting the
 * \0 terminator.
 */
size_t ET_tree2string(ExprTree tree, char *buf, size_t buf_sz);

#endif /* _EXPR_TREE_H_ */
