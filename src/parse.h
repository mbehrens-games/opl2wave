/*******************************************************************************
** parse.h (parsing functions)
*******************************************************************************/

#ifndef PARSE_H
#define PARSE_H

#include "datatree.h"

enum
{
  PARSE_STATE_ATTRIBUTE_SUBFIELD_OR_VALUE,
  PARSE_STATE_SUBFIELD_OR_END_OF_FIELD,
  PARSE_STATE_END_OF_FIELD
};

/* function declarations */
data_tree_node* parse_file_to_data_tree(char* filename);
short int       parse_data_tree_to_globals(data_tree_node* root);

#endif
