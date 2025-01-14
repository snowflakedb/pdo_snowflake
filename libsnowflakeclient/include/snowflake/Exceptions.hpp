/*
 * Copyright (c) 2018-2024 Snowflake Computing, Inc. All rights reserved.
 */

#ifndef SNOWFLAKECLIENT_EXCEPTIONS_HPP
#define SNOWFLAKECLIENT_EXCEPTIONS_HPP

#include <exception>
#include <string>
#include "client.h"

namespace Snowflake
{
namespace Client
{

class SnowflakeException: public std::exception
{
public:
  // Return error message combine all information
  // sub-classes need to setup m_errmsg.
  virtual const char* what() const noexcept override
  {
    return m_errmsg.c_str();
  }

  // optional properties sub-classes could choose what to override
  // with information available
  virtual int code()
  {
    return 0;
  }

  virtual const char* sqlstate()
  {
    return "";
  }

  // Return the original error message without other information (sqlstate etc.).
  virtual const char* msg()
  {
    return "";
  }

  virtual const char* sfqid()
  {
    return "";
  }

  virtual const char* file()
  {
    return "";
  }

  virtual int line()
  {
    return 0;
  }

protected:
  // update error message
  void setErrorMessage(const std::string& errmsg);

  // update error message with formatted arguments
  void setErrorMessage(const char* fmt, va_list args);

  std::string m_errmsg;
};

class SnowflakeGeneralException: public SnowflakeException
{
public:
  SnowflakeGeneralException(SF_ERROR_STRUCT *error);
  SnowflakeGeneralException(const std::string& message,
                            const char* file, int line,
                            int code = 0,
                            const std::string queryId = "",
                            const std::string sqlState = "");
  SnowflakeGeneralException(const char* file, int line,
                            int code,
                            const std::string queryId,
                            const std::string sqlState,
                            const char* fmt, ...);

  virtual int code() override;

  virtual const char* sqlstate() override;

  virtual const char* msg() override;

  virtual const char* sfqid() override;

  virtual const char* file() override;

  virtual int line() override;

protected:
  std::string m_message;
  std::string m_file;
  int m_line;
  std::string m_queryId;
  std::string m_sqlState;
  int m_code;
};

// macro for throw general exception with SF_ERROR_STRUCT
#define SNOWFLAKE_THROW_S(error)                                    \
{                                                                   \
  throw SnowflakeGeneralException(error);                           \
}

// macro for throw general exception with error message
#define SNOWFLAKE_THROW(errmsg)                                     \
{                                                                   \
  throw SnowflakeGeneralException(errmsg,                           \
                                  __FILE__, __LINE__);              \
}

// macro for throw general exception with more detail information
#define SNOWFLAKE_THROW_DETAIL(errmsg, code, qid, state)            \
{                                                                   \
  throw SnowflakeGeneralException(errmsg,                           \
                                  __FILE__, __LINE__,               \
                                  code, qid, state);                \
}

// macro for throw general exception with formatted arguments
#define SNOWFLAKE_THROW_FORMATTED(fmt, ...)                         \
{                                                                   \
  throw SnowflakeGeneralException(__FILE__, __LINE__,               \
                                  0, "", "",                        \
                                  fmt, __VA_ARGS__);                \
}

// macro for throw general exception with formatted arguments and detail information.
#define SNOWFLAKE_THROW_FORMATTED_DETAIL(code, qid, state, fmt, ...)       \
{                                                                          \
  throw SnowflakeGeneralException(__FILE__, __LINE__,                      \
                                  code, qid, state,                        \
                                  fmt, __VA_ARGS__);                       \
}

} // namespace Client
} // namespace Snowflake

#endif //SNOWFLAKECLIENT_EXCEPTIONS_HPP
