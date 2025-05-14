/*
 * Notice: For Snowflake internal use only.
 *         External application should not use this class.
 */

#pragma once
#ifndef SNOWFLAKECLIENT_BINDUPLOADER_HPP
#define SNOWFLAKECLIENT_BINDUPLOADER_HPP

#include <chrono>
#include <snowflake/Exceptions.hpp>
#include "client.h"

namespace Snowflake
{
namespace Client
{

class BindUploader
{
public:
 /**
  * constructor
  *
  * @param stageDir The unique stage path for bindings uploading, could be a GUID.
  * @param numParams Number of parameters.
  * @param numParamSets Number of parameter sets.
  * @param maxFileSize The max size of single file for bindings uploading.
  *                    Separate into multiple files when exceed.
  * @param compressLevel The compress level, between -1(default) to 9.
  */
  explicit BindUploader(const std::string& stageDir,
                        unsigned int numParams,
                        unsigned int numParamSets,
                        unsigned int maxFileSize,
                        int compressLevel);

 /**
  * Add string value to binding stream and do uploading as needed.
  * @param value The string value to be added.
  * @param type The DB type of the value, for ODBC only when format
  *             conversion for date/time types between stage/regular
  *             binding is needed. (implementing convert/revert functions)
  *
  * @Return true if succeeded, false otherwise.
  */
  virtual bool addStringValue(const std::string& value, SF_DB_TYPE type);

  /**
   * Add NULL value to binding stream and do uploading as needed.
   *
  * @Return true if succeeded, false otherwise.
   */
  bool addNullValue();

  inline std::string getStagePath()
  {
    return m_stagePath;
  }

  inline bool hasBindingUploaded()
  {
    return m_hasBindingUploaded;
  }

  inline std::string getError()
  {
    return m_errorMessage;
  }

protected:
  /**
  * @return The statement for creating temporary stage for bind uploading.
  */
  std::string getCreateStageStmt();

  void setError(const std::string& errMsg)
  {
    m_errorMessage = errMsg;
  }

  /**
  * Check whether the session's temporary stage has been created, and create it
  * if not.
  *
  * @Return true if succeeded, false otherwise.
  */
  virtual bool createStageIfNeeded() = 0;

  /**
  * Execute uploading for single data file.
  *
  * @param sql PUT command for single data file uploading
  * @param uploadStream stream for data file to be uploaded
  * @param dataSize Size of the data to be uploaded.
  *
  * @Return true if succeeded, false otherwise.
  */
  virtual bool executeUploading(const std::string &sql,
                                std::basic_iostream<char>& uploadStream,
                                size_t dataSize) = 0;

  /* date/time format conversions to be overridden by drivers (such as ODBC)
   * that need native date/time type support.
   * Will be called to converting binding format between regular binding and
   * bulk binding.
   * No conversion by default, in such case application/driver should bind
   * data/time data as string.
   */

  std::stringstream m_csvStream;

private:
  /**
  * Upload serialized binds in CSV stream to stage
  *
  * @Return true if succeeded, false otherwise.
  */
  bool putBinds();

  /**
  * Compress data from csv stream to compress stream with gzip
  * @return The data size of compress stream if compress succeeded.
  * @throw when compress failed.
  */
  size_t compressWithGzip();

  /**
  * Build PUT statement string. Handle filesystem differences and escaping backslashes.
  * @param srcFilePath The faked source file path to upload.
  */
  std::string getPutStmt(const std::string& srcFilePath);

  std::stringstream m_compressStream;

  std::string m_stagePath;

  unsigned int m_fileNo;

  unsigned int m_maxFileSize;

  unsigned int m_numParams;

  unsigned int m_numParamSets;

  unsigned int m_curParamIndex;

  unsigned int m_curParamSetIndex;

  size_t m_dataSize;

  std::chrono::steady_clock::time_point m_startTime;

  std::chrono::steady_clock::time_point m_serializeStartTime;

  long long m_compressTime;

  long long m_serializeTime;

  long long m_putTime;

  bool m_hasBindingUploaded;

  int m_compressLevel;

  std::string m_errorMessage;

};

} // namespace Client
} // namespace Snowflake

#endif  // SNOWFLAKECLIENT_BINDUPLOADER_HPP
