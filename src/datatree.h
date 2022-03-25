/*******************************************************************************
** datatree.h (data tree node)
*******************************************************************************/

#ifndef DATA_TREE_H
#define DATA_TREE_H

#define DATA_TREE_STACK_INITIAL_SIZE  10

#define DATA_TREE_PUSH_NODE(stack, node)                                       \
  if (stack##_top >= stack##_size - 1)                                         \
  {                                                                            \
    stack = realloc(stack,                                                     \
                    (stack##_size + DATA_TREE_STACK_INITIAL_SIZE) *            \
                    sizeof(data_tree_node*));                                  \
  }                                                                            \
                                                                               \
  stack[++stack##_top] = node;

#define DATA_TREE_POP_NODE(stack)                                              \
  if (stack##_top >= 0)                                                        \
    stack##_top--;

#define DATA_TREE_CREATE_NEW_NODE(stack, current)                              \
  if (current == stack[stack##_top])                                           \
  {                                                                            \
    current->child = data_tree_node_create();                                  \
    current = current->child;                                                  \
  }                                                                            \
  else                                                                         \
  {                                                                            \
    current->sibling = data_tree_node_create();                                \
    current = current->sibling;                                                \
  }

enum
{
  DATA_TREE_NODE_TYPE_NONE,
  /* root fields */
  DATA_TREE_NODE_TYPE_FIELD_OPL2WAVE,
  /* channel fields */
  DATA_TREE_NODE_TYPE_FIELD_OP_1,
  DATA_TREE_NODE_TYPE_FIELD_OP_2,
  DATA_TREE_NODE_TYPE_FIELD_WAVEFORM,
  DATA_TREE_NODE_TYPE_FIELD_MULTIPLE,
  DATA_TREE_NODE_TYPE_FIELD_DETUNE_COARSE,
  DATA_TREE_NODE_TYPE_FIELD_DETUNE_FINE,
  DATA_TREE_NODE_TYPE_FIELD_PMS,
  DATA_TREE_NODE_TYPE_FIELD_AMS,
  DATA_TREE_NODE_TYPE_FIELD_ALGORITHM,
  DATA_TREE_NODE_TYPE_FIELD_FEEDBACK,
  /* envelope fields */
  DATA_TREE_NODE_TYPE_FIELD_AR,
  DATA_TREE_NODE_TYPE_FIELD_DR,
  DATA_TREE_NODE_TYPE_FIELD_SR,
  DATA_TREE_NODE_TYPE_FIELD_RR,
  DATA_TREE_NODE_TYPE_FIELD_TL,
  DATA_TREE_NODE_TYPE_FIELD_SL,
  DATA_TREE_NODE_TYPE_FIELD_RKS,
  DATA_TREE_NODE_TYPE_FIELD_LKS,
  /* filtering fields */
  DATA_TREE_NODE_TYPE_FIELD_FILTER_CUTOFF,
  /* note fields */
  DATA_TREE_NODE_TYPE_FIELD_NOTE,
  DATA_TREE_NODE_TYPE_FIELD_NOTE_LENGTH,
  DATA_TREE_NODE_TYPE_FIELD_PMD,
  DATA_TREE_NODE_TYPE_FIELD_AMD,
  /* attributes */
  DATA_TREE_NODE_TYPE_ATTRIBUTE_EXPORT_LENGTH,
  DATA_TREE_NODE_TYPE_ATTRIBUTE_EXPORT_SAMPLING,
  DATA_TREE_NODE_TYPE_ATTRIBUTE_EXPORT_BITRES,
  DATA_TREE_NODE_TYPE_ATTRIBUTE_DOWNSAMPLING_M,
  DATA_TREE_NODE_TYPE_ATTRIBUTE_TUNING_SYSTEM,
  DATA_TREE_NODE_TYPE_ATTRIBUTE_TUNING_FORK,
  /* values */
  DATA_TREE_NODE_TYPE_VALUE_INTEGER,
  DATA_TREE_NODE_TYPE_VALUE_FLOAT,
  DATA_TREE_NODE_TYPE_VALUE_STRING
};

typedef struct data_tree_node
{
  int   type;
  char* value;

  struct data_tree_node*  child;
  struct data_tree_node*  sibling;
} data_tree_node;

/* function declarations */
short int       data_tree_node_init(data_tree_node* node);
data_tree_node* data_tree_node_create();
short int       data_tree_node_deinit(data_tree_node* node);
short int       data_tree_node_destroy(data_tree_node* node);
short int       data_tree_node_destroy_tree(data_tree_node* node);

#endif
