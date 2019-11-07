/*
 * Copyright (c) 2017-2018 Snowflake Computing, Inc. All rights reserved.
 */

#ifndef SNOWFLAKE_PARAMSTORE_H
#define SNOWFLAKE_PARAMSTORE_H

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(_WIN32)
#define STDCALL
#else
#define STDCALL __stdcall
#endif

#include "snowflake_treemap.h"
#include "snowflake_arraylist.h"

typedef enum {
  INVALID_PARAM_TYPE,
  POSITIONAL,
  NAMED
} PARAM_TYPE;

typedef struct param_store
{
  PARAM_TYPE param_style;
  union
  {
    TREE_MAP *tree_map;
    ARRAY_LIST *array_list;
  }param_type;
}PARAM_STORE;


PARAM_TYPE STDCALL _pdo_sf_get_param_style(const int paramno);

void STDCALL pdo_sf_param_store_init(PARAM_TYPE ptype, void **pstore);

void STDCALL pdo_sf_param_store_deallocate(void *ps);

int STDCALL pdo_sf_param_store_set(void *ps,
                                           void *item,
                                           size_t idx,
                                           char *name);

void *STDCALL pdo_sf_param_store_get(void *ps,
                                 size_t index,
                                 char *key);
#ifdef __cplusplus
}
#endif

#endif /* SNOWFLAKE_PARAMSTORE_H */