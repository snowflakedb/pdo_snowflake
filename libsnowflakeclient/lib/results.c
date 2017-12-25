/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */

#include <string.h>
#include "results.h"
#include "connection.h"
#include "snowflake_memory.h"
#include <snowflake_logger.h>

SF_TYPE string_to_snowflake_type(const char *string) {
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
        // Everybody loves a string, so lets return it by default
        return SF_TYPE_TEXT;
    }
}

const char *snowflake_type_to_string(SF_TYPE type) {
    switch (type) {
        case SF_TYPE_FIXED:
            return "FIXED";
        case SF_TYPE_REAL:
            return "REAL";
        case SF_TYPE_TEXT:
            return "TEXT";
        case SF_TYPE_DATE:
            return "DATE";
        case SF_TYPE_TIMESTAMP_LTZ:
            return "TIMESTAMP_LTZ";
        case SF_TYPE_TIMESTAMP_NTZ:
            return "TIMESTAMP_NTZ";
        case SF_TYPE_TIMESTAMP_TZ:
            return "TIMESTAMP_TZ";
        case SF_TYPE_VARIANT:
            return "VARIANT";
        case SF_TYPE_OBJECT:
            return "OBJECT";
        case SF_TYPE_ARRAY:
            return "ARRAY";
        case SF_TYPE_BINARY:
            return "BINARY";
        case SF_TYPE_TIME:
            return "TIME";
        case SF_TYPE_BOOLEAN:
            return "BOOLEAN";
        default:
            return "TEXT";
    }
}

SF_C_TYPE snowflake_to_c_type(SF_TYPE type, int64 precision, int64 scale) {
    if (type == SF_TYPE_FIXED) {
        if (scale > 0 || precision >= 19) {
            return SF_C_TYPE_FLOAT64;
        } else {
            return SF_C_TYPE_INT64;
        }
    } else if (type == SF_TYPE_REAL) {
        return SF_C_TYPE_FLOAT64;
    } else if (type == SF_TYPE_TIMESTAMP_LTZ ||
            type == SF_TYPE_TIMESTAMP_NTZ ||
            type == SF_TYPE_TIMESTAMP_TZ) {
        return SF_C_TYPE_TIMESTAMP;
    } else if (type == SF_TYPE_BOOLEAN) {
        return SF_C_TYPE_BOOLEAN;
    } else if (type == SF_TYPE_TEXT ||
            type == SF_TYPE_VARIANT ||
            type == SF_TYPE_OBJECT ||
            type == SF_TYPE_ARRAY) {
        return SF_C_TYPE_STRING;
    } else {
        // by default return string, since we can do everything with a string
        return SF_C_TYPE_STRING;
    }
}

SF_TYPE c_type_to_snowflake(SF_C_TYPE c_type, SF_TYPE tsmode) {
    switch (c_type) {
        case SF_C_TYPE_INT8:
            return SF_TYPE_FIXED;
        case SF_C_TYPE_UINT8:
            return SF_TYPE_FIXED;
        case SF_C_TYPE_INT64:
            return SF_TYPE_FIXED;
        case SF_C_TYPE_UINT64:
            return SF_TYPE_FIXED;
        case SF_C_TYPE_FLOAT64:
            return SF_TYPE_REAL;
        case SF_C_TYPE_STRING:
            return SF_TYPE_TEXT;
        case SF_C_TYPE_TIMESTAMP:
            return tsmode;
        case SF_C_TYPE_BOOLEAN:
            return SF_TYPE_BOOLEAN;
        default:
            return SF_TYPE_TEXT;
    }
}

char *value_to_string(void *value, size_t len, SF_C_TYPE c_type) {
    size_t size;
    char *ret;
    // TODO turn cases into macro and check to see if ret if null
    switch (c_type) {
        case SF_C_TYPE_INT8:
            size = (size_t) snprintf( NULL, 0, "%d", *(int8 *) value) + 1;
            ret = (char *) SF_CALLOC(1, size);
            snprintf(ret, size, "%d", *(int8 *) value);
            return ret;
        case SF_C_TYPE_UINT8:
            size = (size_t) snprintf( NULL, 0, "%u", *(uint8 *) value) + 1;
            ret = (char *) SF_CALLOC(1, size);
            snprintf(ret, size, "%u", *(uint8 *) value);
            return ret;
        case SF_C_TYPE_INT64:
            size = (size_t) snprintf( NULL, 0, "%lld", *(int64 *) value) + 1;
            ret = (char *) SF_CALLOC(1, size);
            snprintf(ret, size, "%lld", *(int64 *) value);
            return ret;
        case SF_C_TYPE_UINT64:
            size = (size_t) snprintf( NULL, 0, "%llu", *(uint64 *) value) + 1;
            ret = (char *) SF_CALLOC(1, size);
            snprintf(ret, size, "%llu", *(uint64 *) value);
            return ret;
        case SF_C_TYPE_FLOAT64:
            size = (size_t) snprintf( NULL, 0, "%f", *(float64 *) value) + 1;
            ret = (char *) SF_CALLOC(1, size);
            snprintf(ret, size, "%f", *(float64 *) value);
            return ret;
        case SF_C_TYPE_BOOLEAN:
            size = *(sf_bool*)value == SF_BOOLEAN_TRUE ? sizeof(SF_BOOLEAN_INT_TRUE_STR) : sizeof(SF_BOOLEAN_INT_FALSE_STR);
            ret = (char*) SF_CALLOC(1, size + 1);
            strncpy(ret, *(sf_bool*)value == SF_BOOLEAN_TRUE ? SF_BOOLEAN_INT_TRUE_STR : SF_BOOLEAN_INT_FALSE_STR, size + 1);
            return ret;
        case SF_C_TYPE_STRING:
            size = (size_t)len + 1;
            ret = (char *) SF_CALLOC(1, size);
            strncpy(ret, (const char *) value, size);
            return ret;
        case SF_C_TYPE_TIMESTAMP:
            // TODO Add timestamp case
            return "";
        default:
            // TODO better default case
            // Return empty string in default case
            ret = (char *) SF_CALLOC(1, 1);
            ret[0] = '\0';
            return ret;
    }
}

SF_COLUMN_DESC * set_description(const cJSON *rowtype) {
    int i;
    cJSON *blob;
    cJSON *column;
    SF_COLUMN_DESC *desc = NULL;
    size_t array_size = (size_t) cJSON_GetArraySize(rowtype);
    if (rowtype == NULL || array_size == 0) {
        return desc;
    }
    desc = (SF_COLUMN_DESC *) SF_CALLOC(array_size, sizeof(SF_COLUMN_DESC));
    for (i = 0; i < array_size; i++) {
        column = cJSON_GetArrayItem(rowtype, i);
        if(json_copy_string(&desc[i].name, column, "name")) {
            desc[i].name = NULL;
        }
        if (json_copy_int(&desc[i].byte_size, column, "byteLength")) {
            desc[i].byte_size = 0;
        }
        if (json_copy_int(&desc[i].internal_size, column, "length")) {
            desc[i].internal_size = 0;
        }
        if (json_copy_int(&desc[i].precision, column, "precision")) {
            desc[i].precision = 0;
        }
        if (json_copy_int(&desc[i].scale, column, "scale")) {
            desc[i].scale = 0;
        }
        if (json_copy_bool(&desc[i].null_ok, column, "nullable")) {
            desc[i].null_ok = SF_BOOLEAN_FALSE;
        }
        // Get type
        blob = cJSON_GetObjectItem(column, "type");
        if (cJSON_IsString(blob)) {
            desc[i].type = string_to_snowflake_type(blob->valuestring);
        } else {
            // TODO Replace with default type
            desc[i].type = SF_TYPE_FIXED;
        }
        desc[i].c_type = snowflake_to_c_type(desc[i].type, desc[i].precision, desc[i].scale);
        log_debug("Found type and ctype; %i: %i", desc[i].type, desc[i].c_type);

    }

    return desc;
}
