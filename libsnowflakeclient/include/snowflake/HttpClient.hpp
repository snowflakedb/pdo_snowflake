
#ifndef SNOWFLAKECLIENT_HTTPCLIENT_HPP
#define SNOWFLAKECLIENT_HTTPCLIENT_HPP

#include <string>
#include <map>
#include <vector>

#include <boost/url/url.hpp>
#include <boost/optional.hpp>

namespace Snowflake {
  namespace Client {
    struct HttpResponse {
      long code;

      std::string getBody() const {
        return std::string(buffer.begin(), buffer.end());
      }

      std::vector<char> buffer;
    };

    struct HttpRequest {
      enum class Method {
        GET,
        PUT,
        DEL,
        POST,
      } method;

      static const char* methodToString(Method method) {
        switch (method) {
          case Method::GET:
            return "GET";
          case Method::PUT:
            return "PUT";
          case Method::DEL:
            return "DELETE";
          case Method::POST:
            return "POST";
        }
        return "";
      }

      boost::urls::url url;
      std::map <std::string, std::string> headers;
    };

    struct HttpClientConfig {
      long connectTimeoutInSeconds;
    };

    class IHttpClient {
    public:
      virtual boost::optional<HttpResponse> run(HttpRequest req) = 0;
      virtual ~IHttpClient() = default;

      static IHttpClient* createSimple(const HttpClientConfig&);
      static IHttpClient* getInstance();
    };

  }
}
#endif //SNOWFLAKECLIENT_HTTPCLIENT_HPP
