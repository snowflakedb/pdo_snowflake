#ifndef SNOWFLAKE_CURLDESC_HPP
#define	SNOWFLAKE_CURLDESC_HPP

#include "snowflake/BaseClasses.hpp"
#include "snowflake/SFURL.hpp"
#include <curl/curl.h>

namespace Snowflake
{
namespace Client
{
/**
 * Class CurlDesc
 */
class CurlDesc: private DoNotCopy
{
public:

  /**
   * Constructor
   *
   * @param (IN/NULL) shareDesc
   *   shared descriptor, use null if not used
   */
  CurlDesc(CURLSH *shareDesc);

  /**
   * Destructor, no-op
   */
  virtual ~CurlDesc();

  /**
   * Get underlying curl descriptor for this request
   */
  CURL *getCurl()
  {
    return(m_curl);
  }

  std::string getUrlStr()
  {
    return m_url.toString();
  }

  /**
   * Reset the descriptor and make it ready to be reused
   *
   * @param cleanup
   *   if true, descriptor should be re-created
   */
  virtual void reset(bool cleanup = false);

  // returns SSL version actually being used.
  // for logging purpose also could be used in test
  std::string getNegotiatedSSLVersion()
  {
      return m_negotiatedSSLVersion;
  }

  /**
   * Callback for CURLOPT_PREREQFUNCTION, to update negotiated SSL version
   *
   * @param clientp
   *   pointer to CurlDesc instance, passed through CURLOPT_PREREQDATA
   *
   * @return always CURL_PREREQFUNC_OK
   */
  static int prereqCallback(void* clientp,
                            char*, char*, int, int);

protected:

  /** shared descriptor to use. Can be null if we should not use shared desc */
  CURLSH *m_shareCurl;

  /** Curl easy open descriptor */
  CURL *m_curl;

  /** url set at prepare time */
  SFURL m_url;

  std::string m_negotiatedSSLVersion;

  void updateNegotiatedSSLVersion();

};
}
}

#endif	/* SNOWFLAKE_CURLDESC_HPP */

