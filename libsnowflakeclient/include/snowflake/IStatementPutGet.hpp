/*
 * Copyright (c) 2018 Snowflake Computing, Inc. All rights reserved.
 */

#ifndef SNOWFLAKECLIENT_ISTATEMENT_HPP
#define SNOWFLAKECLIENT_ISTATEMENT_HPP

#include "PutGetParseResponse.hpp"

namespace Snowflake
{
namespace Client
{
/**
 * Interface that should be implemented by external component to interact
 * with server to get metadata/credentials about put/get command
 */
class IStatementPutGet
{
public:
  /**
   * Send put or get command to gs and get all data back.
   * This command will allocate a PutGetParseResponse object on heap,
   * Caller of this method is responsible for deallocating the memory
   *
   * @param sql put or get command
   * @param putGetParseResponse parse response. Object is owned by caller
   *
   * return true if parse succeed otherwise false
   */
  virtual bool parsePutGetCommand(std::string *sql,
                                  PutGetParseResponse *putGetParseResponse) = 0;
};
}
}

#endif //SNOWFLAKECLIENT_ISTATEMENT_HPP
