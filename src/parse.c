/*******************************************************************************
** parse.c (parsing functions)
*******************************************************************************/

#include <stdio.h>    /* testing */
#include <stdlib.h>
#include <string.h>

#include "channel.h"
#include "datatree.h"
#include "global.h"
#include "parse.h"
#include "token.h"
#include "tuning.h"

#define PARSE_EAT_TOKEN(just_eat_it)                                           \
  if (t.token == just_eat_it)                                                  \
    tokenizer_advance(&t);                                                     \
  else                                                                         \
    goto houston;

/*******************************************************************************
** parse_file_to_data_tree()
*******************************************************************************/
data_tree_node* parse_file_to_data_tree(char* filename)
{
  tokenizer         t;
  data_tree_node*   root;
  data_tree_node*   current;
  data_tree_node**  stack;
  int               stack_size;
  int               stack_top;
  int               parse_state;

  /* initialize tokenizer and open file */
  tokenizer_init(&t);

  if (tokenizer_open_file(&t, filename))
    return NULL;

  /* setup stack */
  stack = malloc(DATA_TREE_STACK_INITIAL_SIZE * sizeof(data_tree_node*));
  stack_size = DATA_TREE_STACK_INITIAL_SIZE;
  stack_top = -1;

  /* initial parsing; create root node and push onto stack */
  root = data_tree_node_create();

  PARSE_EAT_TOKEN(TOKEN_LESS_THAN)

  if ((t.token == TOKEN_IDENTIFIER) && (!strcmp(t.sb, "opl2wave")))
  {
    root->type = DATA_TREE_NODE_TYPE_FIELD_OPL2WAVE;
    current = root;
    DATA_TREE_PUSH_NODE(stack, root)
    tokenizer_advance(&t);
  }
  else
    goto cleanup;

  parse_state = PARSE_STATE_ATTRIBUTE_SUBFIELD_OR_VALUE;

  /* begin parsing subfields */
  while (stack_top >= 0)
  {
    /* attribute */
    if ((t.token == TOKEN_AT_SYMBOL) && 
        (parse_state == PARSE_STATE_ATTRIBUTE_SUBFIELD_OR_VALUE))
    {
      DATA_TREE_CREATE_NEW_NODE(stack, current)
      tokenizer_advance(&t);

      if (t.token != TOKEN_IDENTIFIER)
        goto houston;

      if (!strcmp(t.sb, "export_length"))
        current->type = DATA_TREE_NODE_TYPE_ATTRIBUTE_EXPORT_LENGTH;
      else if (!strcmp(t.sb, "export_sampling"))
        current->type = DATA_TREE_NODE_TYPE_ATTRIBUTE_EXPORT_SAMPLING;
      else if (!strcmp(t.sb, "export_bitres"))
        current->type = DATA_TREE_NODE_TYPE_ATTRIBUTE_EXPORT_BITRES;
      else if (!strcmp(t.sb, "downsampling_m"))
        current->type = DATA_TREE_NODE_TYPE_ATTRIBUTE_DOWNSAMPLING_M;
      else if (!strcmp(t.sb, "tuning_system"))
        current->type = DATA_TREE_NODE_TYPE_ATTRIBUTE_TUNING_SYSTEM;
      else if (!strcmp(t.sb, "tuning_fork"))
        current->type = DATA_TREE_NODE_TYPE_ATTRIBUTE_TUNING_FORK;
      else
        goto houston;

      tokenizer_advance(&t);

      PARSE_EAT_TOKEN(TOKEN_EQUAL_SIGN)

      if (t.token == TOKEN_NUMBER_INTEGER)
      {
        current->child = data_tree_node_create();
        current->child->type = DATA_TREE_NODE_TYPE_VALUE_INTEGER;
        current->child->value = strdup(t.sb);
      }
      else if (t.token == TOKEN_NUMBER_FLOAT)
      {
        current->child = data_tree_node_create();
        current->child->type = DATA_TREE_NODE_TYPE_VALUE_FLOAT;
        current->child->value = strdup(t.sb);
      }
      else if (t.token == TOKEN_STRING)
      {
        current->child = data_tree_node_create();
        current->child->type = DATA_TREE_NODE_TYPE_VALUE_STRING;
        current->child->value = strdup(t.sb);
      }
      else
        goto houston;

      tokenizer_advance(&t);
    }
    /* subfield */
    else if ( (t.token == TOKEN_LESS_THAN) &&
              ( (parse_state == PARSE_STATE_ATTRIBUTE_SUBFIELD_OR_VALUE) ||
                (parse_state == PARSE_STATE_SUBFIELD_OR_END_OF_FIELD)))
    {
      DATA_TREE_CREATE_NEW_NODE(stack, current)
      tokenizer_advance(&t);

      if (t.token != TOKEN_IDENTIFIER)
        goto houston;

      /* channel fields */
      if (!strcmp(t.sb, "op1"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_OP_1;
      else if (!strcmp(t.sb, "op2"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_OP_2;
      else if (!strcmp(t.sb, "waveform"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_WAVEFORM;
      else if (!strcmp(t.sb, "multiple"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_MULTIPLE;
      else if (!strcmp(t.sb, "detune_coarse"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_DETUNE_COARSE;
      else if (!strcmp(t.sb, "detune_fine"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_DETUNE_FINE;
      else if (!strcmp(t.sb, "pms"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_PMS;
      else if (!strcmp(t.sb, "ams"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_AMS;
      else if (!strcmp(t.sb, "algorithm"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_ALGORITHM;
      else if (!strcmp(t.sb, "feedback"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_FEEDBACK;
      /* envelope fields */
      else if (!strcmp(t.sb, "ar"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_AR;
      else if (!strcmp(t.sb, "dr"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_DR;
      else if (!strcmp(t.sb, "sr"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_SR;
      else if (!strcmp(t.sb, "rr"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_RR;
      else if (!strcmp(t.sb, "tl"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_TL;
      else if (!strcmp(t.sb, "sl"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_SL;
      else if (!strcmp(t.sb, "rks"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_RKS;
      else if (!strcmp(t.sb, "lks"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_LKS;
      /* filter fields */
      else if (!strcmp(t.sb, "filter_cutoff"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_FILTER_CUTOFF;
      /* note fields */
      else if (!strcmp(t.sb, "note"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_NOTE;
      else if (!strcmp(t.sb, "note_length"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_NOTE_LENGTH;
      else if (!strcmp(t.sb, "pmd"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_PMD;
      else if (!strcmp(t.sb, "amd"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_AMD;
      else
        goto houston;

      DATA_TREE_PUSH_NODE(stack, current)
      tokenizer_advance(&t);
      parse_state = PARSE_STATE_ATTRIBUTE_SUBFIELD_OR_VALUE;
    }
    /* integer */
    else if ( (t.token == TOKEN_NUMBER_INTEGER) &&
              (parse_state == PARSE_STATE_ATTRIBUTE_SUBFIELD_OR_VALUE))
    {
      DATA_TREE_CREATE_NEW_NODE(stack, current)
      current->type = DATA_TREE_NODE_TYPE_VALUE_INTEGER;
      current->value = strdup(t.sb);
      tokenizer_advance(&t);
      parse_state = PARSE_STATE_END_OF_FIELD;
    }
    /* float */
    else if ( (t.token == TOKEN_NUMBER_FLOAT) &&
              (parse_state == PARSE_STATE_ATTRIBUTE_SUBFIELD_OR_VALUE))

    {
      DATA_TREE_CREATE_NEW_NODE(stack, current)
      current->type = DATA_TREE_NODE_TYPE_VALUE_FLOAT;
      current->value = strdup(t.sb);
      tokenizer_advance(&t);
      parse_state = PARSE_STATE_END_OF_FIELD;
    }
    /* string */
    else if ( (t.token == TOKEN_STRING) &&
              (parse_state == PARSE_STATE_ATTRIBUTE_SUBFIELD_OR_VALUE))

    {
      DATA_TREE_CREATE_NEW_NODE(stack, current)
      current->type = DATA_TREE_NODE_TYPE_VALUE_STRING;
      current->value = strdup(t.sb);
      tokenizer_advance(&t);
      parse_state = PARSE_STATE_END_OF_FIELD;
    }
    /* end of field */
    else if ( (t.token == TOKEN_GREATER_THAN) &&
              ( (parse_state == PARSE_STATE_END_OF_FIELD) ||
                (parse_state == PARSE_STATE_SUBFIELD_OR_END_OF_FIELD)))
    {
      current = stack[stack_top];
      DATA_TREE_POP_NODE(stack)
      tokenizer_advance(&t);
      parse_state = PARSE_STATE_SUBFIELD_OR_END_OF_FIELD;
    }
    /* error */
    else
      goto houston;
  }

  /* read eof and cleanup */
  PARSE_EAT_TOKEN(TOKEN_EOF)

  goto cleanup;

  /* error handling */
houston:
  if (root != NULL)
  {
    data_tree_node_destroy_tree(root);
    root = NULL;
  }

  printf("Failed text file parsing on line number %d.\n", t.ln);

  /* cleanup */
cleanup:
  if (stack != NULL)
  {
    free(stack);
    stack = NULL;
  }

  tokenizer_close_file(&t);
  tokenizer_deinit(&t);

  return root;
}

/*******************************************************************************
** parse_data_tree_semantic_analysis()
*******************************************************************************/
short int parse_data_tree_semantic_analysis(int current_type, int parent_type)
{
  if ((current_type == DATA_TREE_NODE_TYPE_FIELD_OPL2WAVE) &&
      (parent_type != DATA_TREE_NODE_TYPE_NONE))
    return 1;
  /* channel fields */
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_OP_1) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OPL2WAVE))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_OP_2) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OPL2WAVE))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_WAVEFORM) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OP_1)      &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OP_2))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_MULTIPLE) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OP_1)      &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OP_2))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_DETUNE_COARSE) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OP_1)           &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OP_2))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_DETUNE_FINE) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OP_1)         &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OP_2))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_AMS) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OP_1) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OP_2))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_PMS) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OP_1) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OP_2))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_ALGORITHM) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OPL2WAVE))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_FEEDBACK) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OPL2WAVE))
    return 1;
  /* envelope fields */
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_AR)  &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OP_1) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OP_2))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_DR)  &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OP_1) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OP_2))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_SR)  &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OP_1) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OP_2))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_RR)  &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OP_1) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OP_2))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_TL)  &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OP_1) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OP_2))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_SL)  &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OP_1) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OP_2))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_RKS) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OP_1) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OP_2))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_LKS) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OP_1) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OP_2))
    return 1;
  /* filter fields */
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_FILTER_CUTOFF) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OPL2WAVE))
    return 1;
  /* note fields */
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_NOTE) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OPL2WAVE))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_NOTE_LENGTH) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OPL2WAVE))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_PMD) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OPL2WAVE))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_AMD) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OPL2WAVE))
    return 1;
  /* attributes */
  else if ( (current_type == DATA_TREE_NODE_TYPE_ATTRIBUTE_EXPORT_LENGTH) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OPL2WAVE))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_ATTRIBUTE_EXPORT_SAMPLING) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OPL2WAVE))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_ATTRIBUTE_EXPORT_BITRES) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OPL2WAVE))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_ATTRIBUTE_DOWNSAMPLING_M) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OPL2WAVE))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_ATTRIBUTE_TUNING_SYSTEM) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OPL2WAVE))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_ATTRIBUTE_TUNING_FORK) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OPL2WAVE))
    return 1;
  /* values */
  else if ( (current_type == DATA_TREE_NODE_TYPE_VALUE_INTEGER)             &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_MULTIPLE)             &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_DETUNE_COARSE)        &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_DETUNE_FINE)          &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_PMS)                  &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_AMS)                  &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_ALGORITHM)            &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_FEEDBACK)             &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_AR)                   &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_DR)                   &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_SR)                   &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_RR)                   &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_TL)                   &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_SL)                   &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_RKS)                  &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_LKS)                  &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_PMD)                  &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_AMD)                  &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_FILTER_CUTOFF)        &&
            (parent_type != DATA_TREE_NODE_TYPE_ATTRIBUTE_EXPORT_SAMPLING)  &&
            (parent_type != DATA_TREE_NODE_TYPE_ATTRIBUTE_EXPORT_BITRES)    &&
            (parent_type != DATA_TREE_NODE_TYPE_ATTRIBUTE_DOWNSAMPLING_M))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_VALUE_FLOAT)       &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_NOTE_LENGTH)  &&
            (parent_type != DATA_TREE_NODE_TYPE_ATTRIBUTE_EXPORT_LENGTH))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_VALUE_STRING)            &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_WAVEFORM)           &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_NOTE)               &&
            (parent_type != DATA_TREE_NODE_TYPE_ATTRIBUTE_TUNING_SYSTEM)  &&
            (parent_type != DATA_TREE_NODE_TYPE_ATTRIBUTE_TUNING_FORK))
    return 1;

  return 0;
}

/*******************************************************************************
** parse_data_tree_lookup_note()
*******************************************************************************/
char parse_data_tree_lookup_note(char* name)
{
  char note;

  if (name == NULL)
    return -1;

  /* make sure this string is the proper size */
  if ((name[0] == '\0') || (name[1] == '\0') || 
      (name[2] == '\0') || (name[3] != '\0'))
  {
    return -1;
  }

  /* check if this is a specific noise divider */
  if ((name[0] == '0') && (name[1] == 'x'))
  {
    if (name[2] == '0')
      note = 0;
    else if (name[2] == '1')
      note = 1;
    else if (name[2] == '2')
      note = 2;
    else if (name[2] == '3')
      note = 3;
    else if (name[2] == '4')
      note = 4;
    else if (name[2] == '5')
      note = 5;
    else if (name[2] == '6')
      note = 6;
    else if (name[2] == '7')
      note = 7;
    else if (name[2] == '8')
      note = 8;
    else if (name[2] == '9')
      note = 9;
    else if (name[2] == 'a')
      note = 10;
    else if (name[2] == 'b')
      note = 11;
    else if (name[2] == 'c')
      note = 12;
    else if (name[2] == 'd')
      note = 13;
    else if (name[2] == 'e')
      note = 14;
    else if (name[2] == 'f')
      note = 15;
    else
      return -1;

    return note;
  }

  /* lookup note */
  if ((name[0] == 'c') && (name[1] == '_'))
    note = 12;
  else if ((name[0] == 'c') && (name[1] == '#'))
    note = 13;
  else if ((name[0] == 'd') && (name[1] == '_'))
    note = 14;
  else if ((name[0] == 'd') && (name[1] == '#'))
    note = 15;
  else if ((name[0] == 'e') && (name[1] == '_'))
    note = 16;
  else if ((name[0] == 'f') && (name[1] == '_'))
    note = 17;
  else if ((name[0] == 'f') && (name[1] == '#'))
    note = 18;
  else if ((name[0] == 'g') && (name[1] == '_'))
    note = 19;
  else if ((name[0] == 'g') && (name[1] == '#'))
    note = 20;
  else if ((name[0] == 'a') && (name[1] == '_'))
    note = 21;
  else if ((name[0] == 'a') && (name[1] == '#'))
    note = 22;
  else if ((name[0] == 'b') && (name[1] == '_'))
    note = 23;
  else
    return -1;

  /* lookup octave */
  if (name[2] == '0')
    note += 0;
  else if (name[2] == '1')
    note += 12;
  else if (name[2] == '2')
    note += 24;
  else if (name[2] == '3')
    note += 36;
  else if (name[2] == '4')
    note += 48;
  else if (name[2] == '5')
    note += 60;
  else if (name[2] == '6')
    note += 72;
  else if (name[2] == '7')
    note += 84;
  else if (name[2] == '8')
    note += 96;
  else
    return -1;

  /* make sure note is in the range A0 - C8 */
  if ((note < 21) || (note > 108))
  {
    printf("Invalid Note specified.\n");
    return -1;
  }

  return note;
}

/*******************************************************************************
** parse_data_tree_load_integer()
*******************************************************************************/
short int parse_data_tree_load_integer(int val, int parent_type, int grand_type)
{
  int op_num;

  /* multiple */
  if (parent_type == DATA_TREE_NODE_TYPE_FIELD_MULTIPLE)
  {
    /* determine operator */
    if (grand_type == DATA_TREE_NODE_TYPE_FIELD_OP_1)
      op_num = 0;
    else if (grand_type == DATA_TREE_NODE_TYPE_FIELD_OP_2)
      op_num = 1;
    else
      op_num = 0;

    /* set multiple */
    if ((val >= 0) && (val <= 15))
      G_channel.multiple[op_num] = val;
    else
    {
      printf("Invalid Multiple specified for Op %d (+1). Defaulting to 0.\n", op_num);
      G_channel.multiple[op_num] = 1;
    }
  }
  /* detune coarse */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_DETUNE_COARSE)
  {
    /* determine operator */
    if (grand_type == DATA_TREE_NODE_TYPE_FIELD_OP_1)
      op_num = 0;
    else if (grand_type == DATA_TREE_NODE_TYPE_FIELD_OP_2)
      op_num = 1;
    else
      op_num = 0;

    /* set detune coarse */
    if ((val >= 0) && (val <= 3))
      G_channel.detune_coarse[op_num] = val;
    else
    {
      printf("Invalid Coarse Detune specified for Op %d (+1). Defaulting to 0.\n", op_num);
      G_channel.detune_coarse[op_num] = 0;
    }
  }
  /* detune fine */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_DETUNE_FINE)
  {
    /* determine operator */
    if (grand_type == DATA_TREE_NODE_TYPE_FIELD_OP_1)
      op_num = 0;
    else if (grand_type == DATA_TREE_NODE_TYPE_FIELD_OP_2)
      op_num = 1;
    else
      op_num = 0;

    /* set detune fine */
    if ((val >= 0) && (val <= 7))
      G_channel.detune_fine[op_num] = val;
    else
    {
      printf("Invalid Fine Detune specified for Op %d (+1). Defaulting to 0.\n", op_num);
      G_channel.detune_fine[op_num] = 0;
    }
  }
  /* pms */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_PMS)
  {
    /* determine operator */
    if (grand_type == DATA_TREE_NODE_TYPE_FIELD_OP_1)
      op_num = 0;
    else if (grand_type == DATA_TREE_NODE_TYPE_FIELD_OP_2)
      op_num = 1;
    else
      op_num = 0;

    /* set pms */
    if ((val >= 0) && (val <= 1))
      G_channel.pms[op_num] = val;
    else
    {
      printf("Invalid PMS specified for Op %d (+1). Defaulting to 0.\n", op_num);
      G_channel.pms[op_num] = 0;
    }
  }
  /* ams */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_AMS)
  {
    /* determine operator */
    if (grand_type == DATA_TREE_NODE_TYPE_FIELD_OP_1)
      op_num = 0;
    else if (grand_type == DATA_TREE_NODE_TYPE_FIELD_OP_2)
      op_num = 1;
    else
      op_num = 0;

    /* set ams */
    if ((val >= 0) && (val <= 1))
      G_channel.ams[op_num] = val;
    else
    {
      printf("Invalid AMS specified for Op %d (+1). Defaulting to 0.\n", op_num);
      G_channel.ams[op_num] = 0;
    }
  }
  /* algorithm */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_ALGORITHM)
  {
    if ((val >= 0) && (val <= 7))
      G_channel.algorithm = val;
    else
    {
      printf("Invalid Algorithm specified. Defaulting to 0.\n");
      G_channel.algorithm = 0;
    }
  }
  /* feedback */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_FEEDBACK)
  {
    if ((val >= 0) && (val <= 7))
      G_channel.fb = val;
    else
    {
      printf("Invalid Feedback specified. Defaulting to 0.\n");
      G_channel.fb = 0;
    }
  }
  /* ar */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_AR)
  {
    /* determine operator */
    if (grand_type == DATA_TREE_NODE_TYPE_FIELD_OP_1)
      op_num = 0;
    else if (grand_type == DATA_TREE_NODE_TYPE_FIELD_OP_2)
      op_num = 1;
    else
      op_num = 0;

    /* set ar */
    if ((val >= 0) && (val <= 15))
      G_channel.ar[op_num] = (unsigned char) val;
    else
    {
      printf("Invalid AR specified for Op %d (+1). Defaulting to 0.\n", op_num);
      G_channel.ar[op_num] = 0;
    }
  }
  /* dr */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_DR)
  {
    /* determine operator */
    if (grand_type == DATA_TREE_NODE_TYPE_FIELD_OP_1)
      op_num = 0;
    else if (grand_type == DATA_TREE_NODE_TYPE_FIELD_OP_2)
      op_num = 1;
    else
      op_num = 0;

    /* set dr */
    if ((val >= 0) && (val <= 15))
      G_channel.dr[op_num] = (unsigned char) val;
    else
    {
      printf("Invalid DR specified for Op %d (+1). Defaulting to 0.\n", op_num);
      G_channel.dr[op_num] = 0;
    }
  }
  /* sr */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_SR)
  {
    /* determine operator */
    if (grand_type == DATA_TREE_NODE_TYPE_FIELD_OP_1)
      op_num = 0;
    else if (grand_type == DATA_TREE_NODE_TYPE_FIELD_OP_2)
      op_num = 1;
    else
      op_num = 0;

    /* set sr */
    if ((val >= 0) && (val <= 15))
      G_channel.sr[op_num] = (unsigned char) val;
    else
    {
      printf("Invalid SR specified for Op %d (+1). Defaulting to 0.\n", op_num);
      G_channel.sr[op_num] = 0;
    }
  }
  /* rr */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_RR)
  {
    /* determine operator */
    if (grand_type == DATA_TREE_NODE_TYPE_FIELD_OP_1)
      op_num = 0;
    else if (grand_type == DATA_TREE_NODE_TYPE_FIELD_OP_2)
      op_num = 1;
    else
      op_num = 0;

    /* set rr */
    if ((val >= 0) && (val <= 15))
      G_channel.rr[op_num] = (unsigned char) val;
    else
    {
      printf("Invalid RR specified for Op %d (+1). Defaulting to 0.\n", op_num);
      G_channel.rr[op_num] = 0;
    }
  }
  /* tl */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_TL)
  {
    /* determine operator */
    if (grand_type == DATA_TREE_NODE_TYPE_FIELD_OP_1)
      op_num = 0;
    else if (grand_type == DATA_TREE_NODE_TYPE_FIELD_OP_2)
      op_num = 1;
    else
      op_num = 0;

    /* set tl */
    if ((val >= 0) && (val <= 63))
      G_channel.tl[op_num] = (unsigned char) val;
    else
    {
      printf("Invalid TL specified for Op %d (+1). Defaulting to 0.\n", op_num);
      G_channel.tl[op_num] = 0;
    }
  }
  /* sl */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_SL)
  {
    /* determine operator */
    if (grand_type == DATA_TREE_NODE_TYPE_FIELD_OP_1)
      op_num = 0;
    else if (grand_type == DATA_TREE_NODE_TYPE_FIELD_OP_2)
      op_num = 1;
    else
      op_num = 0;

    /* set ar */
    if ((val >= 0) && (val <= 15))
      G_channel.sl[op_num] = (unsigned char) val;
    else
    {
      printf("Invalid SL specified for Op %d (+1). Defaulting to 0.\n", op_num);
      G_channel.sl[op_num] = 0;
    }
  }
  /* rks */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_RKS)
  {
    /* determine operator */
    if (grand_type == DATA_TREE_NODE_TYPE_FIELD_OP_1)
      op_num = 0;
    else if (grand_type == DATA_TREE_NODE_TYPE_FIELD_OP_2)
      op_num = 1;
    else
      op_num = 0;

    /* set rks */
    if ((val >= 0) && (val <= 2))
      G_channel.rks[op_num] = val;
    else
    {
      printf("Invalid RKS specified for Op %d (+1). Defaulting to 0.\n", op_num);
      G_channel.rks[op_num] = 0;
    }
  }
  /* lks */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_LKS)
  {
    /* determine operator */
    if (grand_type == DATA_TREE_NODE_TYPE_FIELD_OP_1)
      op_num = 0;
    else if (grand_type == DATA_TREE_NODE_TYPE_FIELD_OP_2)
      op_num = 1;
    else
      op_num = 0;

    /* set lks */
    if ((val >= 0) && (val <= 3))
      G_channel.lks[op_num] = val;
    else
    {
      printf("Invalid LKS specified for Op %d (+1). Defaulting to 0.\n", op_num);
      G_channel.lks[op_num] = 0;
    }
  }
  /* pmd */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_PMD)
  {
    if ((val >= 0) && (val <= 127))
      G_channel.pmd = (char) val;
    else
    {
      printf("Invalid PMD specified. Defaulting to 0.\n");
      G_channel.pmd = 0;
    }
  }
  /* amd */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_AMD)
  {
    if ((val >= 0) && (val <= 127))
      G_channel.amd = (char) val;
    else
    {
      printf("Invalid AMD specified. Defaulting to 0.\n");
      G_channel.amd = 0;
    }
  }
  /* filter cutoff */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_FILTER_CUTOFF)
  {
    if ((val >= 2500) && (val <= 5000))
      G_filter.cutoff = val;
    else if (val == 0)
      G_filter.cutoff = 0;
    else
    {
      printf("Invalid filter cutoff specified. Defaulting to 0 (no filter).\n");
      G_filter.cutoff = 0;
    }
  }
  /* export sampling rate */
  else if (parent_type == DATA_TREE_NODE_TYPE_ATTRIBUTE_EXPORT_SAMPLING)
  {
    if (val == 8363)
    {
      G_export_sampling = 8363;
      G_export_period = 119574;
    }
    else if (val == 16726)
    {
      G_export_sampling = 16726;
      G_export_period = 59787;
    }
    else if (val == 22050)
    {
      G_export_sampling = 22050;
      G_export_period = 45351;
    }
    else if (val == 44100)
    {
      G_export_sampling = 44100;
      G_export_period = 22676;
    }
    else if (val == 49716)
    {
      G_export_sampling = 49716;
      G_export_period = 18773;
    }
    else
    {
      printf("Invalid export sampling rate specified. Defaulting to 44100 hz.\n");
      G_export_sampling = 44100;
      G_export_period = 22676;
    }
  }
  /* export bit resolution */
  else if (parent_type == DATA_TREE_NODE_TYPE_ATTRIBUTE_EXPORT_BITRES)
  {
    if ((val == 8) || (val == 16))
      G_export_bitres = val;
    else
    {
      printf("Invalid export bitres specified. Defaulting to 16 bit.\n");
      G_export_bitres = 16;
    }
  }
  /* downsampling m */
  else if (parent_type == DATA_TREE_NODE_TYPE_ATTRIBUTE_DOWNSAMPLING_M)
  {
    if ((val == 64) || (val == 128) || (val == 256) || (val == 512))
    {
      G_downsampling_m = val;
      G_downsampling_bound = (G_downsampling_m / 2) + 1;
    }
    else
    {
      printf("Invalid downsampling M specified. Defaulting to 128.\n");
      G_downsampling_m = 128;
      G_downsampling_bound = (G_downsampling_m / 2) + 1;
    }
  }

  return 0;
}

/*******************************************************************************
** parse_data_tree_load_float()
*******************************************************************************/
short int parse_data_tree_load_float(float val, int parent_type)
{
  /* note length */
  if (parent_type == DATA_TREE_NODE_TYPE_FIELD_NOTE_LENGTH)
  {
    if ((val >= 0.0f) && (val <= 4.0f))
      G_note_length = val;
    else
    {
      printf("Invalid note length specified. Should be in the interval [0, 4]. Defaulting to 0.0.\n");
      G_note_length = 0.0f;
    }
  }
  /* export length */
  else if (parent_type == DATA_TREE_NODE_TYPE_ATTRIBUTE_EXPORT_LENGTH)
  {
    if ((val >= 0.0f) && (val <= 4.0f))
      G_export_length = val;
    else
    {
      printf("Invalid export length specified. Should be in the interval [0, 4]. Defaulting to 0.0.\n");
      G_export_length = 0.0f;
    }
  }

  return 0;
}

/*******************************************************************************
** parse_data_tree_load_string()
*******************************************************************************/
short int parse_data_tree_load_string(char* name, 
                                      int parent_type, int grand_type)
{
  int op_num;

  if (name == NULL)
    return 1;

  /* waveform */
  if (parent_type == DATA_TREE_NODE_TYPE_FIELD_WAVEFORM)
  {
    /* determine operator */
    if (grand_type == DATA_TREE_NODE_TYPE_FIELD_OP_1)
      op_num = 0;
    else if (grand_type == DATA_TREE_NODE_TYPE_FIELD_OP_2)
      op_num = 1;
    else
      op_num = 0;

    if (!strcmp(name, "sine"))
      G_channel.waveform[op_num] = CHANNEL_WAVEFORM_SINE;
    else if (!strcmp(name, "half_rect"))
      G_channel.waveform[op_num] = CHANNEL_WAVEFORM_HALF_RECT;
    else if (!strcmp(name, "full_rect"))
      G_channel.waveform[op_num] = CHANNEL_WAVEFORM_FULL_RECT;
    else if (!strcmp(name, "quarter"))
      G_channel.waveform[op_num] = CHANNEL_WAVEFORM_QUARTER;
    else
    {
      printf("Invalid waveform specified. Defaulting to Sine.\n");
      G_channel.waveform[op_num] = CHANNEL_WAVEFORM_SINE;
    }
  }
  /* note */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_NOTE)
  {
    G_channel.note = parse_data_tree_lookup_note(name);
  }
  /* tuning system */
  else if (parent_type == DATA_TREE_NODE_TYPE_ATTRIBUTE_TUNING_SYSTEM)
  {
    if (!strcmp(name, "equal_temperament"))
      G_tuning_system = TUNING_SYSTEM_12_ET;
    else if (!strcmp(name, "pythagorean"))
      G_tuning_system = TUNING_SYSTEM_PYTHAGOREAN;
    else if (!strcmp(name, "quarter_comma_meantone"))
      G_tuning_system = TUNING_SYSTEM_QC_MEANTONE;
    else if (!strcmp(name, "just_intonation"))
      G_tuning_system = TUNING_SYSTEM_JUST;
    else if (!strcmp(name, "werckmeister_iii"))
      G_tuning_system = TUNING_SYSTEM_WERCKMEISTER_III;
    else if (!strcmp(name, "werckmeister_iv"))
      G_tuning_system = TUNING_SYSTEM_WERCKMEISTER_IV;
    else if (!strcmp(name, "werckmeister_v"))
      G_tuning_system = TUNING_SYSTEM_WERCKMEISTER_V;
    else if (!strcmp(name, "werckmeister_vi"))
      G_tuning_system = TUNING_SYSTEM_WERCKMEISTER_VI;
    else if (!strcmp(name, "renold_i"))
      G_tuning_system = TUNING_SYSTEM_RENOLD_I;
    else
    {
      printf("Invalid tuning system specified. Defaulting to Equal Temperament.\n");
      G_tuning_system = TUNING_SYSTEM_12_ET;
    }
  }
  /* tuning fork */
  else if (parent_type == DATA_TREE_NODE_TYPE_ATTRIBUTE_TUNING_FORK)
  {
    if (!strcmp(name, "a440"))
      G_tuning_fork = TUNING_FORK_A440;
    else if (!strcmp(name, "a432"))
      G_tuning_fork = TUNING_FORK_A432;
    else if (!strcmp(name, "c256"))
      G_tuning_fork = TUNING_FORK_C256;
    else if (!strcmp(name, "amiga"))
      G_tuning_fork = TUNING_FORK_AMIGA;
    else
    {
      printf("Invalid tuning fork specified. Defaulting to A440.\n");
      G_tuning_fork = TUNING_FORK_A440;
    }
  }

  return 0;
}

/*******************************************************************************
** parse_data_tree_to_globals()
*******************************************************************************/
short int parse_data_tree_to_globals(data_tree_node* root)
{
  data_tree_node*   current;
  int               current_type;
  int               parent_type;
  int               grand_type;
  int               great_type;
  data_tree_node**  stack;
  int               stack_size;
  int               stack_top;

  if (root == NULL)
    return 1;

  /* setup stack */
  stack = malloc(DATA_TREE_STACK_INITIAL_SIZE * sizeof(data_tree_node*));
  stack_size = DATA_TREE_STACK_INITIAL_SIZE;
  stack_top = -1;

  /* verify root node */
  if ((root->type != DATA_TREE_NODE_TYPE_FIELD_OPL2WAVE)  ||
      (root->sibling != NULL)                             ||
      (root->child == NULL))
  {
    printf("Invalid root node.\n");
    goto houston;
  }

  /* push root node onto stack and begin traversing the subtree */
  DATA_TREE_PUSH_NODE(stack, root)
  current = root->child;

  while(stack_top >= 0)
  {
    current_type = current->type;
    parent_type = stack[stack_top]->type;

    if (stack_top == 0)
      grand_type = DATA_TREE_NODE_TYPE_NONE;
    else
      grand_type = stack[stack_top - 1]->type;

    if (stack_top <= 1)
      great_type = DATA_TREE_NODE_TYPE_NONE;
    else
      great_type = stack[stack_top - 2]->type;

    /* semantic analysis */
    if (parse_data_tree_semantic_analysis(current_type, parent_type))
    {
      printf("Semantic analysis failed.\n");
      goto houston;
    }

    /* process this node */
    if (current_type == DATA_TREE_NODE_TYPE_VALUE_INTEGER)
    {
      parse_data_tree_load_integer( strtol(current->value, NULL, 10),
                                    parent_type, grand_type);
    }
    else if (current_type == DATA_TREE_NODE_TYPE_VALUE_FLOAT)
    {
      parse_data_tree_load_float( (float) strtod(current->value, NULL),
                                  parent_type);
    }
    else if (current_type == DATA_TREE_NODE_TYPE_VALUE_STRING)
    {
      parse_data_tree_load_string(current->value, parent_type, grand_type);
    }

    /* go to next node */
    if (current->child != NULL)
    {
      DATA_TREE_PUSH_NODE(stack, current)
      current = current->child;
    }
    else if (current->sibling != NULL)
    {
      current = current->sibling;
    }
    else
    {
      current = stack[stack_top];
      DATA_TREE_POP_NODE(stack)

      while ((stack_top >= 0) && (current->sibling == NULL))
      {
        current = stack[stack_top];
        DATA_TREE_POP_NODE(stack)
      }

      if (stack_top == -1)
        current = NULL;
      else
        current = current->sibling;
    }
  }

  goto cleanup;

  /* error handling */
houston:
  channel_deinit(&G_channel);
  filter_deinit(&G_filter);

  /* cleanup */
cleanup:
  if (stack != NULL)
  {
    free(stack);
    stack = NULL;
  }

  return 0;
}

