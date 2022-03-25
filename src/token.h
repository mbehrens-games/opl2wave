/*******************************************************************************
** token.h (tokenizer)
*******************************************************************************/

#ifndef TOKEN_H
#define TOKEN_H

#include <stdio.h>

#define TOKENIZER_MAX_BUFFER_SIZE 81

enum
{
  /* end of file / error */
  TOKEN_EOF,
  TOKEN_ERROR,

  /* punctuation */
  TOKEN_LESS_THAN,
  TOKEN_GREATER_THAN,
  TOKEN_AT_SYMBOL,
  TOKEN_EQUAL_SIGN,

  /* identifiers and constants */
  TOKEN_IDENTIFIER,
  TOKEN_NUMBER_INTEGER,
  TOKEN_NUMBER_FLOAT,
  TOKEN_STRING
};

typedef struct tokenizer
{
  FILE* fp;                               /* file pointer */

  char  nc;                               /* next character */
  int   token;                            /* current token */
  int   ln;                               /* line number */

  char  sb[TOKENIZER_MAX_BUFFER_SIZE];    /* string buffer */
  int   sb_i;
} tokenizer;

/* function declarations */
short int   tokenizer_init(tokenizer* t);
tokenizer*  tokenizer_create();
short int   tokenizer_deinit(tokenizer* t);
short int   tokenizer_destroy(tokenizer* t);

short int   tokenizer_advance(tokenizer* t);

short int   tokenizer_open_file(tokenizer* t, char* filename);
short int   tokenizer_close_file(tokenizer* t);

short int   tokenizer_print_file_tokens(tokenizer* t, char* filename);

#endif
