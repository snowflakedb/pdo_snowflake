/*
** Copyright (c) 2017-2019 Snowflake Computing, Inc. All rights reserved.
*/

#include "php.h"
#include "snowflake_treemap.h"

TREE_MAP * STDCALL pdo_sf_treemap_init()
{
  TREE_MAP *tree_map = (TREE_MAP *)ecalloc(TREE_MAP_MAX_SIZE, sizeof(TREE_MAP));
  if (!tree_map)
  {
    //PDO_LOG_ERR("pdo_sf_treemap_init: Memory Allocation failed\n");
  }

  return tree_map;
}

/*
** The hash fxn used by the treemap
** should follow horner's rule 
** to generate index
** @return an index in the treemap
*/
unsigned long STDCALL pdo_sf_treemap_hash_fxn(char *key)
{
  unsigned int iter = 0;
  unsigned long hash = 0;
  size_t len = 0;

  len = strlen(key);

  for (iter = 0; iter < len; iter++)
  {
    hash = (HASH_CONSTANT*hash)+key[iter];
  }
  return hash;
}

/* 
** Retrieves an index from the tree map
** to store a bind param at.
** @return - unsigned long
*/
unsigned long STDCALL pdo_sf_treemap_get_index(char *key)
{
  unsigned long hash = pdo_sf_treemap_hash_fxn(key);

  hash = hash % TREE_MAP_MAX_SIZE;
  return hash;
}

/*
** Inserts the bind params at the right
** index in the tree map. This also takes
** care of collisions
** @return - int
*/

int STDCALL pdo_sf_treemap_insert_node(unsigned long index, TREE_MAP *tree_map, void *param, char *key)
{
  TREE_MAP *idx_cur;

  idx_cur = &tree_map[index];

  if (!idx_cur->tree)
  {
    idx_cur->tree = pdo_rbtree_init();
  }

  return pdo_rbtree_insert(&idx_cur->tree, param, key);
}

/* pdo_sf_treemap_extract_node
** extract a node corresponding to a key
** from the treemap
** @return - int based on whether node
** was successfully extracted.
*/

int STDCALL pdo_sf_treemap_extract_node(TREE_MAP *tree_map, int idx, char *key, void **ret_param)
{
  TREE_MAP *cur_node;
  int retval = 0;
  if (tree_map && (cur_node = &tree_map[idx]))
  {
    *ret_param = pdo_rbtree_search_node(cur_node->tree, key);
    if (!*ret_param)
    {
      //PDO_LOG_DBG("pdo_sf_treemap_extract_node: Node not found");
      retval = 0;
      goto done;
    }
  }
  else
  {
    if (!tree_map)
    {
      //PDO_LOG_DBG("pdo_sf_treemap_extract_node: tree_map passed is NULL");
      goto done;
    }
    else if (!cur_node)
    {
      //PDO_LOG_DBG("pdo_sf_treemap_extract_node: cur_node is NULL\n");
      goto done;
    }
  }
  retval = 1;
  done:

  return retval;
}

int STDCALL pdo_sf_treemap_set(TREE_MAP *tree_map, void *param, char *key)
{

  if (!tree_map || !param || !key)
  {
    /* Handle error 
    ** return or goto done;
    */
    //PDO_LOG_ERR("pdo_sf_treemap_set: Tree Map || Param || key passed is NULL\n");
    return 0;
  }

  return pdo_sf_treemap_insert_node(pdo_sf_treemap_get_index(key), tree_map, param, key);
}

void * STDCALL pdo_sf_treemap_get(TREE_MAP *tree_map, char *key)
{
  void *param = NULL;
  if (!tree_map || !key)
  {
    return NULL;
  }

  pdo_sf_treemap_extract_node(tree_map, pdo_sf_treemap_get_index(key), key, &param);
  if (!param)
  {
    //PDO_LOG_DBG("pdo_sf_treemap_get: param returned is NULL\n");
  }
  return param;
}

void STDCALL pdo_sf_treemap_deallocate(TREE_MAP *tree_map)
{
  int iter = 0;

  if (!tree_map)
  {
    //PDO_LOG_DBG("pdo_sf_treemap_deallocate: treemap is NULL\n");
    return;
  }
  for (iter = 0; iter < TREE_MAP_MAX_SIZE; iter++)
  {
    if (tree_map[iter].tree)
    {
      pdo_rbtree_deallocate(tree_map[iter].tree);
    }
    tree_map[iter].tree = NULL;
  }
  efree(tree_map);
}
