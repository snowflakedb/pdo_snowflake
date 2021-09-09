/*
 * Copyright (c) 2018-2019 Snowflake Computing, Inc. All rights reserved.
 */

#ifndef SNOWFLAKECLIENT_ISFLOGGER_HPP
#define SNOWFLAKECLIENT_ISFLOGGER_HPP

#include "snowflake/logger.h"
#include "snowflake/platform.h"

namespace Snowflake
{
namespace Client
{
class ISFLogger
{
public:
  virtual ~ISFLogger() {}

  /**
   * Method used internally in SFLogger.cpp, which call logLineVA
   */
  void logLine(SF_LOG_LEVEL logLevel, const char * fileName, const char * msgFmt,
               ...);

  /**
   * Method implemented by external logger
   */
  virtual void logLineVA(SF_LOG_LEVEL logLevel,
                         const char * ns,
                         const char * className,
                         const char * msgFmt,
                         va_list &args) = 0;
};
}
}

#endif //SNOWFLAKECLIENT_ISFLOGGER_HPP
