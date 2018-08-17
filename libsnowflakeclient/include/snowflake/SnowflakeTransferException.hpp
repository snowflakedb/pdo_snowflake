/*
 * Copyright (c) 2018 Snowflake Computing, Inc. All rights reserved.
 */

#ifndef SNOWFLAKECLIENT_SNOWFLAKETRANSFEREXCEPTION_HPP
#define SNOWFLAKECLIENT_SNOWFLAKETRANSFEREXCEPTION_HPP

#include <exception>

namespace Snowflake
{
namespace Client
{

enum TransferError
{
  INTERNAL_ERROR,
  COMPRESSION_ERROR,
  MKDIR_ERROR,
  UNSUPPORTED_FEATURE,
  COLUMN_INDEX_OUT_OF_RANGE,
  DIR_OPEN_ERROR,
  COMPRESSION_NOT_SUPPORTED
};

class SnowflakeTransferException : public std::exception
{
public:
  SnowflakeTransferException(TransferError transferError, ...);

  int getCode();

  virtual const char* what() const noexcept;

private:
  int m_code;

  char m_msg[1000];
};
}
}




#endif //SNOWFLAKECLIENT_SNOWFLAKETRANSFEREXCEPTION_HPP
