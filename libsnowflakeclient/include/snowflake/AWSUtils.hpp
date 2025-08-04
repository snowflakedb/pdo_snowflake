#ifndef SNOWFLAKECLIENT_AWSUTILS_HPP
#define SNOWFLAKECLIENT_AWSUTILS_HPP

#include <string>
#include <memory>
#include <boost/optional.hpp>
#include <aws/core/auth/AWSCredentials.h>

namespace Snowflake {
  namespace Client {
    namespace AwsUtils {
      /*
       * Shared initialization of aws sdk to avoid multiple initialization.
       * To initialize aws sdk, call initAwsSdk() and hold the returned shared_ptr as long as you use AWS SDK.
       * shutdown = true to release resources hold for aws sdk.
       */
      class AwsSdkInitialized;
      std::shared_ptr<AwsSdkInitialized> initAwsSdk(bool shutdown = false);

      std::string getDomainSuffixForRegionalUrl(const std::string &regionName);

      class ISdkWrapper {
      public:
        virtual boost::optional<std::string> getEC2Region() = 0;
        virtual Aws::Auth::AWSCredentials getCredentials() = 0;
        virtual ~ISdkWrapper() = default;
        static ISdkWrapper* getInstance();
      };
    }
  }
}

#endif //SNOWFLAKECLIENT_AWSUTILS_HPP
