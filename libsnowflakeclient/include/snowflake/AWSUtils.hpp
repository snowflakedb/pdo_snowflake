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
        // Calls STS:AssumeRole with the given credentials and target role
        // ARN. Returns temporary credentials on success, or boost::none on
        // failure. Used by the WIF role-assumption (impersonation) chain.
        virtual boost::optional<Aws::Auth::AWSCredentials> assumeRole(
            const Aws::Auth::AWSCredentials& currentCreds,
            const std::string& roleArn) = 0;
        // Calls AWS STS GetWebIdentityToken for outbound identity federation
        // (SNOW-2919437). Returns the signed JWT on success, or boost::none on
        // any failure (HTTP error, signing failure, malformed response).
        virtual boost::optional<std::string> getWebIdentityToken(
            const Aws::Auth::AWSCredentials& creds,
            const std::string& region,
            const std::string& audience,
            const std::string& signingAlgorithm) = 0;
        virtual ~ISdkWrapper() = default;
        static ISdkWrapper* getInstance();
      };
    }
  }
}

#endif //SNOWFLAKECLIENT_AWSUTILS_HPP
