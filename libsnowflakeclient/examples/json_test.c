/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */


#include <stdio.h>
#include <stdlib.h>
#include <snowflake_client.h>
#include <example_setup.h>

int main() {
    cJSON *json;
    char *raw_json = (char *) calloc(1, 1000000);
    FILE *fp;
    fp = fopen("/home/kwagner/raw_json.txt", "r+");
    fread(raw_json, sizeof(raw_json[0]), 1000000, fp);
    fclose(fp);
    json = cJSON_Parse(raw_json);

    if (raw_json) {
        free(raw_json);
    }
    cJSON_Delete(json);
}

