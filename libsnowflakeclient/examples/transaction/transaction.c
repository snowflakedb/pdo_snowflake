/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */


#include <stdio.h>
#include <stdlib.h>
#include <snowflake_client.h>


void main()
{
  SNOWFLAKE_ERROR *err;
  /* init */
  SNOWFLAKE *sf = snowflake_init();

  /* connect*/
  snowflake_set_attr(sf, SF_CON_HOST, getenv("SNOWFLAKE_TEST_HOST"));
  snowflake_set_attr(sf, SF_CON_PORT, getenv("SNOWFLAKE_TEST_PORT"));
  snowflake_set_attr(sf, SF_CON_ACCOUNT, getenv("SNOWFLAKE_TEST_ACCOUNT"));
  snowflake_set_attr(sf, SF_CON_USER, getenv("SNOWFLAKE_TEST_USER"));
  snowflake_set_attr(sf, SF_CON_PASSWORD, getenv("SNOWFLAKE_TEST_PASSWORD"));
  snowflake_set_attr(sf, SF_CON_DATABASE, getenv("SNOWFLAKE_TEST_DATABASE"));
  snowflake_set_attr(sf, SF_CON_SCHEMA, getenv("SNOWFLAKE_TEST_SCHEMA"));
  snowflake_set_attr(sf, SF_CON_ROLE, getenv("SNOWFLAKE_TEST_ROLE"));
  snowflake_set_attr(sf, SF_CON_WAREHOUSE, getenv("SNOWFLAKE_TEST_WAREHOUSE"));
  snowflake_set_attr(sf, SF_CON_AUTOCOMMIT, &SF_BOOLEAN_TRUE);
  snowflake_connect(sf);

  /* begin transaction */
  snowflake_trans_begin(sf);

  /* execute a DML */
  SNOWFLAKE_STMT *sfstmt = snowflake_stmt(sf);
  snowflake_prepare(sfstmt, "INSERT INTO testtable(1,?,?)");
  SNOWFLAKE_BIND_INPUT p1, p2;
  p1.idx = 1;
  p1.c_type = SF_C_TYPE_STRING;
  p1.value = (void *) "test1";
  snowflake_bind_param(sfstmt, &p1);
  p1.idx = 2;
  p1.c_type = SF_C_TYPE_TIMESTAMP;
  p1.value = (void *) "test1";
  snowflake_bind_param(sfstmt, &p2);
  if (snowflake_execute(sfstmt) != SF_STATUS_SUCCESS)
  {
    goto err;
  }

  printf("Success. Query ID: %s, Affected Rows: %ld",
         snowflake_sfqid(sfstmt), (long)snowflake_affected_rows(sfstmt));
  snowflake_trans_commit(sf);
  goto end;

  err: /* error */
  snowflake_trans_rollback(sf);
  /* SNOWFLAKE_ERROR structure is included in a SNOWFLAKE_STMT, so you don't
   * need to free the memory. */
  err = snowflake_error(sfstmt);
  printf("Error. Query ID: %s, Message: %s\n", err->sfqid, err->msg);

  end: /* finally */
  snowflake_stmt_close(sfstmt);
  /* disconnect */
  snowflake_close(sf);

  /* term */
  snowflake_term(sf); // purge snowflake context
}
