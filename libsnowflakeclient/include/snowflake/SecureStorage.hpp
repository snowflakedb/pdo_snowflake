/*
 * File:   SecureStorageApple.hpp *
 * Copyright (c) 2013-2020 Snowflake Computing
 */

#ifndef PROJECT_SECURESTORAGE_HPP
#define PROJECT_SECURESTORAGE_HPP

#include <string>

#include "snowflake/secure_storage.h"

namespace Snowflake {

namespace Client {
  enum class SecureStorageStatus
  {
    NotFound,
    Error,
    Success,
    Unsupported
  };

  inline std::string keyTypeToString(SecureStorageKeyType type) {
    switch (type) {
      case SecureStorageKeyType::MFA_TOKEN:
        return "MFA_TOKEN";
      case SecureStorageKeyType::SSO_TOKEN:
        return "SSO_TOKEN";
      case SecureStorageKeyType::OAUTH_REFRESH_TOKEN:
        return "OAUTH_REFRESH_TOKEN";
      case SecureStorageKeyType::OAUTH_ACCESS_TOKEN:
        return "OAUTH_ACCESS_TOKEN";
      default:
        return "UNKNOWN";
    }
  }
  struct SecureStorageKey {
    std::string host;
    std::string user;
    SecureStorageKeyType type;
  };

  /**
   * Class SecureStorage
   */

  class SecureStorage
  {

  public:
    static std::string convertTarget(const SecureStorageKey& key);

    /**
     * storeToken
     *
     * API to secure store credential
     *
     * @param key - credential key
     * @param cred - credential to be secured
     *
     * @return ERROR / SUCCESS
     */
    SecureStorageStatus storeToken(const SecureStorageKey& key,
                                   const std::string& cred);

    /**
     * retrieveToken
     *
     * API to retrieve credential
     *
     * @param key - credential key
     * @param cred - on succcess, retrieved credential will stored here
     * @return NOT_FOUND, ERROR, SUCCESS
     */
    SecureStorageStatus retrieveToken(const SecureStorageKey& key,
                                      std::string& cred);

    /**
     * remove
     *
     * API to remove a credential.
     *
     * @param key - credenetial key
     *
     * @return ERROR / SUCCESS
     */
    SecureStorageStatus removeToken(const SecureStorageKey& key);
  };

}

}

#endif //PROJECT_SECURESTORAGE_H
