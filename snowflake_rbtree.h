#ifndef SNOWFLAKE_PDO_RBTREE_H
#define SNOWFLAKE_PDO_RBTREE_H

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(_WIN32)
#define STDCALL
#else
#define STDCALL __stdcall
#endif

#include <stdlib.h>

typedef enum color
{
  RED,
  BLACK
}Color;

typedef struct redblack_node
{
  Color color;
  void *elem;
  char *key;
  struct redblack_node *left;
  struct redblack_node *right;
  struct redblack_node *parent;
}RedBlackNode;

typedef RedBlackNode RedBlackTree;

RedBlackTree * STDCALL pdo_rbtree_init(void);

int STDCALL pdo_rbtree_insert(RedBlackTree **T, void *param, char *key);

void * STDCALL pdo_rbtree_search_node(RedBlackTree *tree, char *key);

void STDCALL pdo_rbtree_deallocate(RedBlackNode *node);

#ifdef __cplusplus
}
#endif

#endif /* SNOWFLAKE_RBTREE_H */
