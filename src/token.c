/*******************************************************************************
** token.c (tokenizer)
*******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "token.h"

/* macros */
#define TOKENIZER_RESET_STRING_BUFFER()                                        \
  t->sb[0] = '\0';                                                             \
  t->sb_i = 0;

#define TOKENIZER_APPEND_NEXT_CHAR_TO_STRING_BUFFER()                          \
  if (t->sb_i < TOKENIZER_MAX_BUFFER_SIZE - 1)                                 \
  {                                                                            \
    t->sb[t->sb_i++] = t->nc;                                                  \
    t->sb[t->sb_i] = '\0';                                                     \
  }

#define TOKENIZER_UPDATE_NEXT_CHAR()                                           \
  if (fread(&(t->nc), 1, 1, t->fp) == 0)                                       \
    t->nc = 0;

/*******************************************************************************
** tokenizer_init()
*******************************************************************************/
short int tokenizer_init(tokenizer* t)
{
  if (t == NULL)
    return 1;

  t->fp = NULL;

  t->nc = 0;
  t->token = TOKEN_EOF;
  t->ln = 0;

  TOKENIZER_RESET_STRING_BUFFER()

  return 0;
}

/*******************************************************************************
** tokenizer_create()
*******************************************************************************/
tokenizer* tokenizer_create()
{
  tokenizer* t;

  t = malloc(sizeof(tokenizer));

  tokenizer_init(t);

  return t;
}

/*******************************************************************************
** tokenizer_deinit()
*******************************************************************************/
short int tokenizer_deinit(tokenizer* t)
{
  if (t == NULL)
    return 1;

  /* close open file if necessary */
  if (t->fp != NULL)
    tokenizer_close_file(t);

  t->nc = 0;
  t->token = TOKEN_EOF;
  t->ln = 0;

  TOKENIZER_RESET_STRING_BUFFER()

  return 0;
}

/*******************************************************************************
** tokenizer_destroy()
*******************************************************************************/
short int tokenizer_destroy(tokenizer* t)
{
  if (t == NULL)
    return 1;

  tokenizer_deinit(t);
  free(t);

  return 0;
}

/*******************************************************************************
** tokenizer_advance()
*******************************************************************************/
short int tokenizer_advance(tokenizer* t)
{
  if (t == NULL)
    return 1;

  /* check that stream exists */
  if (t->fp == NULL)
  {
    t->token = TOKEN_ERROR;
    return 0;
  }

  /* ignore whitespace and comments */
  while (isspace(t->nc) || (t->nc == '/'))
  {
    /* advance over whitespace */
    while (isspace(t->nc))
    {
      if (t->nc == '\n')
        t->ln++;

      TOKENIZER_UPDATE_NEXT_CHAR()
    }

    /* advance over comment */
    if (t->nc == '/')
    {
      TOKENIZER_UPDATE_NEXT_CHAR()

      if (t->nc == '*')
      {
        TOKENIZER_UPDATE_NEXT_CHAR()

        /* skip to end of block comment */
        while (!feof(t->fp))
        {
          if (t->nc == '*')
          {
            TOKENIZER_UPDATE_NEXT_CHAR()

            if (t->nc == '/')
            {
              TOKENIZER_UPDATE_NEXT_CHAR()
              break;
            }
          }
          else
          {
            TOKENIZER_UPDATE_NEXT_CHAR()
          }
        }
      }
      else
        t->token = TOKEN_ERROR;
    }
  }

  /* check for specific tokens */
  /* less than */
  if (t->nc == '<')
  {
    t->token = TOKEN_LESS_THAN;
    TOKENIZER_UPDATE_NEXT_CHAR()
  }
  /* greater than */
  else if (t->nc == '>')
  {
    t->token = TOKEN_GREATER_THAN;
    TOKENIZER_UPDATE_NEXT_CHAR()
  }
  /* at symbol */
  else if (t->nc == '@')
  {
    t->token = TOKEN_AT_SYMBOL;
    TOKENIZER_UPDATE_NEXT_CHAR()
  }
  /* equal sign */
  else if (t->nc == '=')
  {
    t->token = TOKEN_EQUAL_SIGN;
    TOKENIZER_UPDATE_NEXT_CHAR()
  }
  /* identifier */
  else if (isalpha(t->nc) || (t->nc == '_'))
  {
    TOKENIZER_RESET_STRING_BUFFER()

    /* store the identifier in the buffer */
    while (isalpha(t->nc) || isdigit(t->nc) || (t->nc == '_'))
    {
      /* convert upper case letters to lower case */
      if (isupper(t->nc))
        t->nc = tolower(t->nc);

      TOKENIZER_APPEND_NEXT_CHAR_TO_STRING_BUFFER()
      TOKENIZER_UPDATE_NEXT_CHAR()
    }

    t->token = TOKEN_IDENTIFIER;
  }
  /* string */
  else if (t->nc == '\"')
  {
    /* advance over double quote */
    TOKENIZER_UPDATE_NEXT_CHAR()

    TOKENIZER_RESET_STRING_BUFFER()

    /* store the string in the buffer */
    while ( isalpha(t->nc) || isdigit(t->nc) || 
            (t->nc == '_') || (t->nc == '#'))
    {
      /* convert upper case letters to lower case */
      if (isupper(t->nc))
        t->nc = tolower(t->nc);

      TOKENIZER_APPEND_NEXT_CHAR_TO_STRING_BUFFER()
      TOKENIZER_UPDATE_NEXT_CHAR()
    }

    /* advance over next double quote */
    if (t->nc == '\"')
    {
      TOKENIZER_UPDATE_NEXT_CHAR()
    }
    else
    {
      t->token = TOKEN_ERROR;
      return 1;
    }

    t->token = TOKEN_STRING;
  }
  /* negative int/float */
  else if (t->nc == '-')
  {
    TOKENIZER_UPDATE_NEXT_CHAR()

    /* negative number */
    if (isdigit(t->nc))
    {
      TOKENIZER_RESET_STRING_BUFFER()

      /* store initial minus sign that was advanced over */
      t->sb[0] = '-';
      t->sb[1] = '\0';
      t->sb_i = 1;

      while (isdigit(t->nc))
      {
        TOKENIZER_APPEND_NEXT_CHAR_TO_STRING_BUFFER()
        TOKENIZER_UPDATE_NEXT_CHAR()
      }

      t->token = TOKEN_NUMBER_INTEGER;

      /* process floating point number if necessary */
      if (t->nc == '.')
      {
        TOKENIZER_APPEND_NEXT_CHAR_TO_STRING_BUFFER()
        TOKENIZER_UPDATE_NEXT_CHAR()

        while (isdigit(t->nc))
        {
          TOKENIZER_APPEND_NEXT_CHAR_TO_STRING_BUFFER()
          TOKENIZER_UPDATE_NEXT_CHAR()
        }

        t->token = TOKEN_NUMBER_FLOAT;
      }
    }
    else
      t->token = TOKEN_ERROR;
  }
  /* positive int/float */
  else if (isdigit(t->nc) || (t->nc == '#'))
  {
    TOKENIZER_RESET_STRING_BUFFER()

    while (isdigit(t->nc))
    {
      TOKENIZER_APPEND_NEXT_CHAR_TO_STRING_BUFFER()
      TOKENIZER_UPDATE_NEXT_CHAR()
    }

    t->token = TOKEN_NUMBER_INTEGER;

    /* process floating point number if necessary */
    if (t->nc == '.')
    {
      TOKENIZER_APPEND_NEXT_CHAR_TO_STRING_BUFFER()
      TOKENIZER_UPDATE_NEXT_CHAR()

      while (isdigit(t->nc))
      {
        TOKENIZER_APPEND_NEXT_CHAR_TO_STRING_BUFFER()
        TOKENIZER_UPDATE_NEXT_CHAR()
      }

      t->token = TOKEN_NUMBER_FLOAT;
    }
  }
  /* eof */
  else if (feof(t->fp))
    t->token = TOKEN_EOF;
  /* error */
  else
    t->token = TOKEN_ERROR;

  return 0;
}

/*******************************************************************************
** tokenizer_open_file()
*******************************************************************************/
short int tokenizer_open_file(tokenizer* t, char* filename)
{
  if (t == NULL)
    return 1;

  /* close file if one is currently open */
  if (t->fp != NULL)
    tokenizer_close_file(t);

  /* open file */
  t->fp = fopen(filename, "r");

  /* if file did not open, return error */
  if (t->fp == NULL)
    return 1;

  /* reset string buffer and line number */
  TOKENIZER_RESET_STRING_BUFFER()

  t->ln = 1;

  /* get first token */
  TOKENIZER_UPDATE_NEXT_CHAR()

  tokenizer_advance(t);

  return 0;
}

/*******************************************************************************
** tokenizer_close_file()
*******************************************************************************/
short int tokenizer_close_file(tokenizer* t)
{
  if ((t != NULL) && (t->fp != NULL))
  {
    fclose(t->fp);
    t->fp = NULL;
  }

  return 0;
}

/*******************************************************************************
** tokenizer_print_file_tokens()
*******************************************************************************/
short int tokenizer_print_file_tokens(tokenizer* t, char* filename)
{
  if (t == NULL)
    return 1;

  /* open file */
  if (tokenizer_open_file(t, filename))
    return 1;

  /* print token stream */
  while ((t->token != TOKEN_ERROR) && (t->token != TOKEN_EOF))
  {
    if (t->token == TOKEN_LESS_THAN)
      printf("<\n");
    else if (t->token == TOKEN_GREATER_THAN)
      printf(">\n");
    else if (t->token == TOKEN_AT_SYMBOL)
      printf("@\n");
    else if (t->token == TOKEN_EQUAL_SIGN)
      printf("=\n");
    else if (t->token == TOKEN_IDENTIFIER)
      printf("IDENTIFIER: %s\n", t->sb);
    else if (t->token == TOKEN_NUMBER_INTEGER)
      printf("INTEGER: %s\n", t->sb);
    else if (t->token == TOKEN_NUMBER_FLOAT)
      printf("FLOAT: %s\n", t->sb);
    else if (t->token == TOKEN_STRING)
      printf("STRING: %s\n", t->sb);

    tokenizer_advance(t);
  }

  if (t->token == TOKEN_EOF)
    printf("EOF\n");
  else
    printf("ERROR encountered!\n");

  /* close file */
  tokenizer_close_file(t);

  return 0;
}
