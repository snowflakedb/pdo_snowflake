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

const char *snowflake_type_to_string(SNOWFLAKE_TYPE type) {
    switch (type) {
        case SF_TYPE_FIXED:
            return "fixed";
        case SF_TYPE_REAL:
            return "real";
        case SF_TYPE_TEXT:
            return "text";
        case SF_TYPE_DATE:
            return "date";
        case SF_TYPE_TIMESTAMP_LTZ:
            return "timestamp_ltz";
        case SF_TYPE_TIMESTAMP_NTZ:
            return "timestamp_ntz";
        case SF_TYPE_TIMESTAMP_TZ:
            return "timestamp_tz";
        case SF_TYPE_VARIANT:
            return "variant";
        case SF_TYPE_OBJECT:
            return "object";
        case SF_TYPE_ARRAY:
            return "array";
        case SF_TYPE_BINARY:
            return "binary";
        case SF_TYPE_TIME:
            return "time";
        case SF_TYPE_BOOLEAN:
            return "boolean";
        default:
            return "text";
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

SNOWFLAKE_TYPE c_type_to_snowflake(SNOWFLAKE_C_TYPE c_type, SNOWFLAKE_TYPE tsmode) {
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
        // TODO better default case
        default:
            return SF_TYPE_TEXT;
    }
}

char *value_to_string(void *value, SNOWFLAKE_C_TYPE c_type) {
    size_t size;
    char *ret;
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
        case SF_C_TYPE_STRING:
            size = strlen((char *) value) + 1;
            ret = (char *) SF_CALLOC(1, size);
            strncpy(ret, (char *) value, size);
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

SNOWFLAKE_COLUMN_DESC ** set_description(const cJSON *rowtype) {
    int i;
    cJSON *blob;
    cJSON *column;
    SNOWFLAKE_COLUMN_DESC **desc = NULL;
    size_t array_size = (size_t) cJSON_GetArraySize(rowtype);
    if (rowtype == NULL || array_size == 0) {
        return desc;
    }
    desc = (SNOWFLAKE_COLUMN_DESC **) SF_CALLOC(array_size, sizeof(SNOWFLAKE_COLUMN_DESC *));
    for (i = 0; i < array_size; i++) {
        column = cJSON_GetArrayItem(rowtype, i);
        desc[i] = (SNOWFLAKE_COLUMN_DESC *) SF_CALLOC(1, sizeof(SNOWFLAKE_COLUMN_DESC));
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
