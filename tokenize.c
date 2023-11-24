/*
 * tokenize.c
 *
 * Functions to tokenize and manipulate lists of tokens
 *
 * Author: Howdy Pierce <howdy@sleepymoose.net>
 * Contributor: Niyomwungeri Parmenide Ishimwe <parmenin@andrew.cmu.edu>
 */
#include <stdio.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "clist.h"
#include "tokenize.h"
#include "token.h"

// Documented in .h file
const char *TT_to_str(TokenType tt)
{
  switch (tt)
  {
  case TOK_VALUE:
    return "VALUE";
  case TOK_PLUS:
    return "PLUS";
  case TOK_MINUS:
    return "MINUS";
  case TOK_MULTIPLY:
    return "MULTIPLY";
  case TOK_DIVIDE:
    return "DIVIDE";
  case TOK_POWER:
    return "POWER";
  case TOK_OPEN_PAREN:
    return "OPEN_PAREN";
  case TOK_CLOSE_PAREN:
    return "CLOSE_PAREN";
  case TOK_EQUAL:
    return "EQUAL";
  case TOK_SYMBOL:
    return "SYMBOL";
  case TOK_END:
    return "(end)";
  }
  __builtin_unreachable();
}

/*
 * Helper function to check if a character is a valid math sign
 *
 * Parameters:
 *   sign   The character to check
 *
 * Returns: true if the character is a valid math sign, false otherwise
 */
bool isValidMathSign(char sign)
{
  if (sign == '+' || sign == '-' || sign == '*' || sign == '/' || sign == '^')
    return true;
  return false;
}

// Documented in .h file
CList TOK_tokenize_input(const char *input, char *errmsg, size_t errmsg_sz)
{
  int i = 0;
  CList tokens = CL_new();

  // The maximum length of a symbol is 31 characters.
  // check if the symbol is invalid
  if (strlen(input) > 31)
  {
    snprintf(errmsg, errmsg_sz, "Position %d: Symbol is too long", i + 1);
    CL_free(tokens);
    return NULL;
  }

  while (input[i] != '\0')
  {
    if (isspace(input[i]))
      i++;

    else if (isdigit(input[i]) || (input[i] == '.' && isdigit(input[i + 1])))
    {
      char *end;
      // convert string to double, starting at address of input[i]
      // and store the address of the first character after the number in end
      // if the number is 1.2e3, end will point to the 'e' & double value will be 1.2
      double value = strtod(&input[i], &end);

      Token tok = {TOK_VALUE, {.value = value}};

      // append the token to the list of tokens
      CL_append(tokens, (CListElementType)tok);

      // advance i to the first character after the number
      i = end - input;
    }
    else if (input[i] == '+')
    {
      if (input[i + 1] == '+' && CL_nth(tokens, CL_length(tokens) - 1).type == TOK_VALUE && isValidMathSign(input[i + 2]))
      {
        Token prev_token = CL_remove(tokens, CL_length(tokens) - 1);
        Token new_token = {TOK_VALUE, {.value = prev_token.t.value + 1}};
        CL_append(tokens, new_token);
        i += 2;
      }
      else
      {
        Token tok = {TOK_PLUS, {.value = 0.0}};
        CL_append(tokens, (CListElementType)tok);
        i++;
      }
    }
    else if (input[i] == '-')
    {
      if (input[i + 1] == '-' && CL_nth(tokens, CL_length(tokens) - 1).type == TOK_VALUE && isValidMathSign(input[i + 2]))
      {
       CListElementType removed = CL_remove(tokens, CL_length(tokens) - 1); 
      Token prev_token = removed;
      Token new_token = {TOK_VALUE, {.value = prev_token.t.value - 1}};
        CL_append(tokens, new_token);
        i += 2;
      }
      else
      {
        Token tok = {TOK_MINUS, {.value = 0.0}};
        CL_append(tokens, (CListElementType)tok);
        i++;
      }
    }
    else if (input[i] == '*')
    {
      Token tok = {TOK_MULTIPLY, {.value = 0.0}};
      CL_append(tokens, (CListElementType)tok);
      i++;
    }
    else if (input[i] == '/')
    {
      Token tok = {TOK_DIVIDE, {.value = 0.0}};
      CL_append(tokens, (CListElementType)tok);
      i++;
    }
    else if (input[i] == '^')
    {
      Token tok = {TOK_POWER, {.value = 0.0}};
      CL_append(tokens, (CListElementType)tok);
      i++;
    }
    else if (input[i] == '(')
    {
      Token tok = {TOK_OPEN_PAREN, {.value = 0.0}};
      CL_append(tokens, (CListElementType)tok);
      i++;
    }
    else if (input[i] == ')')
    {
      Token tok = {TOK_CLOSE_PAREN, {.value = 0.0}};
      CL_append(tokens, (CListElementType)tok);
      i++;
    }
    else if (input[i] == '=')
    {
      Token tok = {TOK_EQUAL, {.value = 0.0}};
      CL_append(tokens, (CListElementType)tok);
      i++;
    }
    else if (isalpha(input[i]))
    {
      char symbol[SYMBOL_MAX_SIZE + 1];
      int j = 0;

      // a symbol, but it follows these rules: A
      // symbol must begin with a alphabetic letter or underscore, and then it can be any combination of letters,
      // underscores, or digits.The maximum length of a symbol is 31 characters.
      // check if the symbol is invalid
      while (j < SYMBOL_MAX_SIZE && (isalpha(input[i]) || isdigit(input[i]) || input[i] == '_'))
      {
        symbol[j] = input[i];
        i++;
        j++;
      }
      symbol[j] = '\0';

      Token tok = {TOK_SYMBOL, {.symbol = {0}}};

      // assign the symbol to the token
      strncpy(tok.t.symbol, symbol, SYMBOL_MAX_SIZE + 1);

      // append the token to the list of tokens
      CL_append(tokens, (CListElementType)tok);

      // advance i to the first character after the symbol
      // i = end - input;
    }
    else
    {
      Token tok = {TOK_END, {.value = 0.0}};
      CL_append(tokens, (CListElementType)tok);
      snprintf(errmsg, errmsg_sz, "Position %d: unexpected character %c", i + 1, input[i]);
      CL_free(tokens);
      return NULL;
    }
  }

  // 6. Return the list of tokens
  return tokens;
}

// Documented in .h file
TokenType TOK_next_type(CList tokens)
{
  if (tokens == NULL)
    return TOK_END;

  if (tokens->head != NULL)
    return tokens->head->element.type;

  return TOK_END;
}

// Documented in .h file
Token TOK_next(CList tokens)
{
  if (tokens == NULL)
  {
    Token tok = {TOK_END, {.value = 0.0}};
    return tok;
  }

  Token nextToken = {TOK_END, {.value = 0.0}};

  if (tokens->head != NULL)
    nextToken = tokens->head->element;

  return nextToken;
}

// Documented in .h file
void TOK_consume(CList tokens)
{
  if (tokens == NULL)
    return;

  if (tokens->head != NULL)
    CL_remove(tokens, 0);
}

// Documented in .h file
void print_element(int pos, CListElementType element, void *cb_data)
{
  CListElementType *data = (CListElementType *)cb_data;

  // Token4
  if (element.type == TOK_VALUE)
    printf("%s: %d %s %f\n", (char *)data, pos, TT_to_str(element.type), element.t.value);
  else if (element.type == TOK_SYMBOL)
    printf("%s: %d %s %s\n", (char *)data, pos, TT_to_str(element.type), element.t.symbol);
  else
    printf("%s: %d %s\n", (char *)data, pos, TT_to_str(element.type));
}

void TOK_print(CList tokens)
{
  if (tokens == NULL)
    return;

  // For debugging: Prints the list of tokens, one per line
  CL_foreach(tokens, print_element, "DEBUG OUTPUT");
}