#include "php.h"
#include "snowflake_paramstore.h"

PARAM_TYPE STDCALL _pdo_sf_get_param_style(const int paramno)
{
  PARAM_TYPE retval = INVALID_PARAM_TYPE;

  if (paramno < 0)
  {
    retval = NAMED;
  }
  else
  {
    retval = POSITIONAL;
  }

  return retval;
}

void STDCALL pdo_sf_param_store_init(PARAM_TYPE ptype, void **ps)
{
  PARAM_STORE *pstore = (PARAM_STORE *)ecalloc(1, sizeof(PARAM_STORE));
  switch(ptype)
  {
    case NAMED:
      pstore->param_style = NAMED;
      pstore->param_type.tree_map = pdo_sf_treemap_init();
      break;
    case POSITIONAL:
      pstore->param_style = POSITIONAL;
      pstore->param_type.array_list = pdo_sf_array_list_init();
      break;
    default:
      pstore->param_style = INVALID_PARAM_TYPE;
      pstore->param_type.tree_map = NULL;
      pstore->param_type.array_list = NULL;
      break;
  }
  *ps = (void *)pstore;
}

void STDCALL pdo_sf_param_store_deallocate(void *ps)
{
  PARAM_STORE *pstore = (PARAM_STORE *)ps;
  if (pstore->param_style == POSITIONAL)
  {
    pdo_sf_array_list_deallocate(pstore->param_type.array_list);
  }
  else if (pstore->param_style == NAMED)
  {
    pdo_sf_treemap_deallocate(pstore->param_type.tree_map);
  }
  efree(pstore);
}

int STDCALL pdo_sf_param_store_set(void *ps,
                                           void *item,
                                           size_t idx,
                                           char *name)
{
  PARAM_STORE *pstore = (PARAM_STORE *)ps;
  int retval = 1;

  if (pstore->param_style == POSITIONAL)
  {
    /*
     * Update sf_array_list_set to return status
     * code;
     */
    pdo_sf_array_list_set(pstore->param_type.array_list, item, idx);
  }
  else if (pstore->param_style == NAMED)
  {
    retval = pdo_sf_treemap_set(pstore->param_type.tree_map, item, name);
  }
  return retval;
}

void *STDCALL pdo_sf_param_store_get(void *ps, size_t index, char *key)
{
  PARAM_STORE * pstore = (PARAM_STORE *)ps;
  if (pstore->param_style == POSITIONAL)
  {
    if (index < 1)
    {
      //PDO_LOG_ERR("pdo_sf_param_store_get: Invalid index for POSITIONAL Params\n");
      return NULL;
    }
    return pdo_sf_array_list_get(pstore->param_type.array_list, index);
  }
  else if (pstore->param_style == NAMED)
  {
    if (!key)
    {
      //PDO_LOG_ERR("pdo_sf_param_store_get: Key NULL for named params \n");
      return NULL;
    }
    return pdo_sf_treemap_get(pstore->param_type.tree_map, key);
  }
  return NULL;
}
