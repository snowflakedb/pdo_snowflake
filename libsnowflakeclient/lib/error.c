/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */

#include <string.h>
#include "error.h"
#include "memory.h"

/*
 * Shared message buffer for emergency use.
 */
pthread_mutex_t mutex_shared_msg = PTHREAD_MUTEX_INITIALIZER;
static char _shared_msg[8192];

void STDCALL set_snowflake_error(SF_ERROR *error,
                                 SF_STATUS error_code,
                                 const char *msg,
                                 const char *sqlstate,
                                 const char *sfqid,
                                 const char *file,
                                 int line) {
    size_t msglen = strlen(msg);
    // NULL error passed in. Should never happen
    if (!error) {
        return;
    }

    error->error_code = error_code;
    strncpy(error->sfqid, sfqid, SF_UUID4_LEN);
    // Null terminate
    if (error->sfqid[SF_UUID4_LEN - 1] != '\0') {
        error->sfqid[SF_UUID4_LEN - 1] = '\0';
    }

    if (sqlstate != NULL) {
        /* set SQLState if specified */
        strncpy(error->sqlstate, sqlstate, sizeof(error->sqlstate));
        error->sqlstate[sizeof(error->sqlstate) - 1] = '\0';
    }

    if (error->msg != NULL && !error->is_shared_msg) {
        /* free message buffer first if already exists */
        SF_FREE(error->msg);
    }

    /* allocate new memory */
    error->msg = SF_CALLOC(msglen + 1, sizeof(char));
    if (error->msg != NULL) {
        strncpy(error->msg, msg, msglen);
        error->msg[msglen] = '\0';
        error->is_shared_msg = SF_BOOLEAN_FALSE;
    } else {
        /* if failed to allocate a memory to store error */
        pthread_mutex_lock(&mutex_shared_msg);
        size_t len =
          msglen > sizeof(_shared_msg) ? sizeof(_shared_msg) : msglen;
        memset(_shared_msg, 0, sizeof(_shared_msg));
        strncpy(_shared_msg, msg, len);
        _shared_msg[sizeof(_shared_msg) - 1] = '\0';
        pthread_mutex_unlock(&mutex_shared_msg);

        error->is_shared_msg = SF_BOOLEAN_TRUE;
        error->msg = _shared_msg;
    }

    error->file = (char *) file;
    error->line = line;
}

void STDCALL clear_snowflake_error(SF_ERROR *error) {
    pthread_mutex_lock(&mutex_shared_msg);
    memset(_shared_msg, 0, sizeof(_shared_msg));
    pthread_mutex_unlock(&mutex_shared_msg);
    if (strncmp(error->sqlstate, SF_SQLSTATE_NO_ERROR,
                sizeof(SF_SQLSTATE_NO_ERROR)) && !error->is_shared_msg) {
        /* Error already set and msg is not on shared mem */
        SF_FREE(error->msg);
    }
    strcpy(error->sqlstate, SF_SQLSTATE_NO_ERROR);
    error->error_code = SF_STATUS_SUCCESS;
    error->msg = NULL;
    error->file = NULL;
    error->line = 0;
    error->is_shared_msg = SF_BOOLEAN_FALSE;
    memset(error->sfqid, 0, SF_UUID4_LEN);
}

void STDCALL copy_snowflake_error(SF_ERROR *dst, SF_ERROR *src) {
    if (!dst || !src) {
        return;
    }

    set_snowflake_error(dst, src->error_code, src->msg, src->sqlstate,
                        src->sfqid, src->file, src->line);
}
