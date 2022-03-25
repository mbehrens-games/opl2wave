/*******************************************************************************
** datatree.c (data tree node)
*******************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "datatree.h"

/*******************************************************************************
** data_tree_node_init()
*******************************************************************************/
short int data_tree_node_init(data_tree_node* node)
{
  if (node == NULL)
    return 1;

  node->type = DATA_TREE_NODE_TYPE_NONE;
  node->value = NULL;

  node->child = NULL;
  node->sibling = NULL;

  return 0;
}

/*******************************************************************************
** data_tree_node_create()
*******************************************************************************/
data_tree_node* data_tree_node_create()
{
  data_tree_node* node;

  node = malloc(sizeof(data_tree_node));
  data_tree_node_init(node);

  return node;
}

/*******************************************************************************
** data_tree_node_deinit()
*******************************************************************************/
short int data_tree_node_deinit(data_tree_node* node)
{
  if (node == NULL)
    return 1;

  if (node->value != NULL)
  {
    free(node->value);
    node->value = NULL;
  }

  return 0;
}

/*******************************************************************************
** data_tree_node_destroy()
*******************************************************************************/
short int data_tree_node_destroy(data_tree_node* node)
{
  if (node == NULL)
    return 1;

  data_tree_node_deinit(node);
  free(node);

  return 0;
}

/*******************************************************************************
** data_tree_node_destroy_tree()
*******************************************************************************/
short int data_tree_node_destroy_tree(data_tree_node* node)
{
  data_tree_node*   current;
  data_tree_node*   previous;
  data_tree_node**  stack;
  int               stack_size;
  int               stack_top;

  if (node == NULL)
    return 1;

  /* setup stack */
  stack = malloc(DATA_TREE_STACK_INITIAL_SIZE * sizeof(data_tree_node*));
  stack_size = DATA_TREE_STACK_INITIAL_SIZE;
  stack_top = -1;

  /* push root node onto stack and begin traversing the subtree */
  DATA_TREE_PUSH_NODE(stack, node)
  previous = NULL;
  current = node;

  while(stack_top >= 0)
  {
    if (current->child != NULL)
    {
      DATA_TREE_PUSH_NODE(stack, current)
      previous = current;
      current = current->child;
      previous->child = NULL;
    }
    else if (current->sibling != NULL)
    {
      previous = current;
      current = current->sibling;
      data_tree_node_destroy(previous);
    }
    else
    {
      previous = current;
      current = stack[stack_top];
      DATA_TREE_POP_NODE(stack)
      data_tree_node_destroy(previous);
    }
  }

  if (stack != NULL)
  {
    free(stack);
    stack = NULL;
  }

  return 0;
}

