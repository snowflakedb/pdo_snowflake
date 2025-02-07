/*
 * File:   secure_storage.h *
 * Copyright (c) 2025 Snowflake Computing
 */


#ifndef SNOWFLAKECLIENT_SECURE_STORAGE_H
#define SNOWFLAKECLIENT_SECURE_STORAGE_H

#include <stdbool.h>

typedef void* secure_storage_ptr;

typedef enum {
  MFA_TOKEN,
  SSO_TOKEN,
  OAUTH_REFRESH_TOKEN,
  OAUTH_ACCESS_TOKEN
} SecureStorageKeyType;

#ifdef __cplusplus
extern "C" {
#endif

secure_storage_ptr secure_storage_init();
char* secure_storage_get_credential(secure_storage_ptr tc, const char* host, const char* user, SecureStorageKeyType type);
void secure_storage_free_credential(char* cred);
bool secure_storage_save_credential(secure_storage_ptr tc, const char* host, const char* user, SecureStorageKeyType type, const char *cred);
bool secure_storage_remove_credential(secure_storage_ptr tc, const char* host, const char* user, SecureStorageKeyType type);
void secure_storage_term(secure_storage_ptr tc);

#ifdef __cplusplus
};
#endif

#endif //SNOWFLAKECLIENT_SECURE_STORAGE_H
