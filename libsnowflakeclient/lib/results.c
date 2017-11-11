/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */

#include <string.h>
#include "results.h"
#include "connection.h"
#include "snowflake_memory.h"
#include "log.h"

SNOWFLAKE_TYPE string_to_snowflake_type(const char *string) {
    if (strcmp(string, "fixed") == 0) {
        return SF_TYPE_FIXED;
    } else if (strcmp(string, "real") == 0) {
        return SF_TYPE_REAL;
    } else if (strcmp(string, "text") == 0) {
        return SF_TYPE_TEXT;
    } else if (strcmp(string, "date") == 0) {
        return SF_TYPE_DATE;
    } else if (strcmp(string, "timestamp_ltz") == 0) {
        return SF_TYPE_TIMESTAMP_LTZ;
    } else if (strcmp(string, "timestamp_ntz") == 0) {
        return SF_TYPE_TIMESTAMP_NTZ;
    } else if (strcmp(string, "timestamp_tz") == 0) {
        return SF_TYPE_TIMESTAMP_TZ;
    } else if (strcmp(string, "variant") == 0) {
        return SF_TYPE_VARIANT;
    } else if (strcmp(string, "object") == 0) {
        return SF_TYPE_OBJECT;
    } else if (strcmp(string, "array") == 0) {
        return SF_TYPE_ARRAY;
    } else if (strcmp(string, "binary") == 0) {
        return SF_TYPE_BINARY;
    } else if (strcmp(string, "time") == 0) {
        return SF_TYPE_TIME;
    } else if (strcmp(string, "boolean") == 0) {
        return SF_TYPE_BOOLEAN;
    } else {
        // TODO default type case
    }
}

SNOWFLAKE_C_TYPE snowflake_to_c_type(SNOWFLAKE_TYPE type, int64 precision, int64 scale) {
    if (type == SF_TYPE_FIXED) {
        if (scale > 0 || precision >= 19) {
            return SF_C_TYPE_FLOAT64;
        } else {
            return SF_C_TYPE_INT64;
        }
    } else if (type == SF_TYPE_TIMESTAMP_LTZ ||
            type == SF_TYPE_TIMESTAMP_NTZ ||
            type == SF_TYPE_TIMESTAMP_TZ) {
        return SF_C_TYPE_TIMESTAMP;
    } else if (type == SF_TYPE_BOOLEAN) {
        return SF_C_TYPE_INT8;
    } else if (type == SF_TYPE_TEXT ||
            type == SF_TYPE_VARIANT ||
            type == SF_TYPE_OBJECT ||
            type == SF_TYPE_ARRAY) {
        return SF_C_TYPE_STRING;
    } else {
        // TODO better default case
        return 0;
    }
}

SNOWFLAKE_COLUMN_DESC ** set_description(const cJSON *rowtype) {
    int i;
    cJSON *blob;
    cJSON *column;
    SNOWFLAKE_COLUMN_DESC **desc = NULL;
    size_t array_size = (size_t) cJSON_GetArraySize(rowtype);
    if (rowtype == NULL || array_size == 0) {
        return desc;
    }
    desc = (SNOWFLAKE_COLUMN_DESC **) sf_calloc(array_size, sizeof(SNOWFLAKE_COLUMN_DESC *));
    for (i = 0; i < array_size; i++) {
        column = cJSON_GetArrayItem(rowtype, i);
        desc[i] = (SNOWFLAKE_COLUMN_DESC *) sf_calloc(1, sizeof(SNOWFLAKE_COLUMN_DESC));
        if(!json_copy_string(&desc[i]->name, column, "name")) {
            desc[i]->name = NULL;
        }
        if (!json_copy_int(&desc[i]->byte_size, column, "byteLength")) {
            desc[i]->byte_size = 0;
        }
        if (!json_copy_int(&desc[i]->internal_size, column, "length")) {
            desc[i]->internal_size = 0;
        }
        if (!json_copy_int(&desc[i]->precision, column, "precision")) {
            desc[i]->precision = 0;
        }
        if (!json_copy_int(&desc[i]->scale, column, "scale")) {
            desc[i]->scale = 0;
        }
        if (!json_copy_bool(&desc[i]->null_ok, column, "nullable")) {
            desc[i]->null_ok = SF_BOOLEAN_FALSE;
        }
        // Get type
        blob = cJSON_GetObjectItem(column, "type");
        if (cJSON_IsString(blob)) {
            desc[i]->type = string_to_snowflake_type(blob->valuestring);
        } else {
            // TODO Replace with default type
            desc[i]->type = SF_TYPE_FIXED;
        }
        desc[i]->c_type = snowflake_to_c_type(desc[i]->type, desc[i]->precision, desc[i]->scale);
        log_debug("Found type and ctype; %i: %i", desc[i]->type, desc[i]->c_type);

    }

    return desc;
}
