#include "php.h"
#include "snowflake_rbtree.h"

RedBlackTree * STDCALL pdo_rbtree_init(void)
{
  RedBlackTree *root = (RedBlackTree *) ecalloc(1, sizeof(RedBlackNode));

  if (!root)
  {
    //PDO_LOG_ERR("pdo_rbtree_init : failed to allocate memory\n");
    return NULL;
  }

  root->key = NULL;
  root->elem = NULL;
  root->left = NULL;
  root->right = NULL;
  root->parent = NULL;
  root->color = BLACK;

  return root;
}

RedBlackNode * STDCALL pdo_rbtree_new_node(void)
{
  return ((RedBlackNode *)ecalloc(1,sizeof(RedBlackNode)));
}

RedBlackNode * STDCALL pdo_rbtree_get_uncle(RedBlackTree *target)
{
  if (!target->parent->parent)
  {
    goto done;
  }

  if (target->parent == target->parent->parent->right)
  {
    return target->parent->parent->left;
  }
  else
  {
    return target->parent->parent->right;
  }

  done:
  return NULL;
}

void STDCALL pdo_rbtree_set_color(RedBlackNode *node, Color color)
{
  if (node)
  {
    node->color = color;
  }
}

Color STDCALL pdo_rbtree_get_color(RedBlackNode *node)
{
  if (!node)
  {
    return BLACK;
  }

  return node->color;
}

void STDCALL pdo_rbtree_rotate_left(RedBlackTree **tree, RedBlackNode *node)
{
  RedBlackNode *rchild = node->right;
  RedBlackNode *parent = node->parent;

  rchild->parent = parent;
  node->right = rchild->left;
  if (rchild->left)
  {
    rchild->left->parent = node;
  }
  rchild->left = node;
  node->parent = rchild;
  if (!parent)
  {
    *tree = rchild;
  }
  else if (node == parent->right)
  {
    parent->right = rchild;
  }
  else
  {
    parent->left = rchild;
  }
}

void STDCALL pdo_rbtree_rotate_right(RedBlackTree **tree, RedBlackNode *node) {
  RedBlackNode *lchild = node->left;
  RedBlackNode *parent = node->parent;

  lchild->parent = parent;
  node->left = lchild->right;
  if (lchild->right)
  {
    lchild->right->parent = node;
  }
  lchild->right = node;
  node->parent = lchild;
  if (!parent)
  {
    *tree = lchild;
  }
  else if (node == parent->right)
  {
    parent->right = lchild;
  }
  else
  {
    parent->left = lchild;
  }
}

int STDCALL pdo_rbtree_fix_tree(RedBlackTree **tree, RedBlackNode *target) {
  RedBlackNode *parent = NULL;
  RedBlackNode *temp_node = NULL;
  RedBlackNode *uncle = NULL;

  if (!target)
  {
    //PDO_LOG_DBG("pdo_rbtree_Fix_tree: tree passed is NULL\n");
    return 0;
  }
  while((target != *tree) && (pdo_rbtree_get_color(target->parent) == RED))
  {
    /* if target's parent is a left child */
    if (target->parent == target->parent->parent->left)
    {
      uncle = pdo_rbtree_get_uncle(target);
      if (pdo_rbtree_get_color(uncle) == RED)
      {
        /* Flip Colors */
        pdo_rbtree_set_color(target->parent, BLACK);
        pdo_rbtree_set_color(uncle, BLACK);
        pdo_rbtree_set_color(target->parent->parent, RED);
        target = target->parent->parent;
        continue;
      }
      else
      {
        if (target == target->parent->right)
        {
          target = target->parent;
          pdo_rbtree_rotate_left(tree, target);
        }
        pdo_rbtree_set_color(target->parent, BLACK);
        pdo_rbtree_set_color(target->parent->parent, RED);
        pdo_rbtree_rotate_right(tree, target->parent->parent);
      }
    }
    else
    {
      uncle = pdo_rbtree_get_uncle(target);
      if (pdo_rbtree_get_color(uncle) == RED)
      {
        /* Flip Color */
        pdo_rbtree_set_color(target->parent, BLACK);
        pdo_rbtree_set_color(uncle, BLACK);
        pdo_rbtree_set_color(target->parent->parent, RED);
        target = target->parent->parent;
        continue;
      }
      else
      {
        if (target == target->parent->left)
        {
          target = target->parent;
          pdo_rbtree_rotate_right(tree, target);
        }
        pdo_rbtree_set_color(target->parent, BLACK);
        pdo_rbtree_set_color(target->parent->parent, RED);
        pdo_rbtree_rotate_left(tree, target->parent->parent);
      }
    }
  }
  (*tree)->color = BLACK;
  return 1;
}
int STDCALL pdo_rbtree_insert(RedBlackTree **T, void *param, char *name)
{
  int cmp = 0;
  RedBlackNode * node;
  int retval = 0;

  /*
   * Tree cannot be NULL
   * Cannot insert NULL params
   * Name cannot be NULL as it is key
   * to node search.
   */
  if (!T || !(*T) || !param || !name)
  {
    return 0;
  }
  node = *T;

  if (!node->elem)
  {
    node->elem = param;
    node->key = name;
    retval = 0;
    goto done;
  }
  while (1)
  {
    cmp = strcmp(name, node->key);
    if (cmp == 0)
    {
      /* Key Exists */
      /* Update*/
      //PDO_LOG_DBG("pdo_rbtree_insert: Duplicate param found, Overwrite\n");
      node->key = name;
      node->elem = param;
      retval = 0;
      goto done;
    }

    else if (cmp > 0)
    {
      if (node->right != NULL)
      {
        node = node->right;
      }
      else
      {
        node->right = pdo_rbtree_new_node();
        if (!node->right)
        {
          retval = 0;
          //PDO_LOG_ERR("pdo_rbtree_insert : Not able to allocate new rbtree node \n");
          goto done;
        }
        node->right->key = name;
        node->right->elem = param;
        node->right->parent = node;
        node->right->color = RED;
        pdo_rbtree_fix_tree(T, node->right);
        retval = 1;
        goto done;
      }
    }

    else
    {
      if (node->left != NULL)
      {
        node = node->left;
      }
      else
      {
        node->left = pdo_rbtree_new_node();
        if (!node->left)
        {
          retval = 0;
          goto done;
        }
        node->left->key = name;
        node->left->elem = param;
        node->left->parent = node;
        node->left->color = RED;
        pdo_rbtree_fix_tree(T, node->left);
        retval = 1;
        goto done;
      }
    }
  }
  done:
  return retval;
}

void * STDCALL pdo_rbtree_search_node(RedBlackTree *tree, char *key)
{
  RedBlackNode *node = tree;
  int cmp = 0;

  if (!tree || !tree->elem || !key)
  {
    return NULL;
  }
  while(1)
  {
    cmp = strcmp(key, node->key);
    if (cmp == 0)
    {
      return node->elem;
    }
    else if(cmp < 0)
    {
      node = node->left;
      if (!node)
      {
        return NULL;
      }
    }
    else
    {
      node = node->right;
      if (!node)
      {
        return NULL;
      }
    }
  }
}

void STDCALL pdo_rbtree_deallocate(RedBlackNode *node)
{
  if (!node)
  {
    return;
  }

  pdo_rbtree_deallocate(node->left);
  pdo_rbtree_deallocate(node->right);
  node->left = NULL;
  node->right = NULL;
  efree(node);
}
