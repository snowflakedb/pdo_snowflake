
#ifndef SNOWFLAKECLIENT_CSPATESTATIONS_HPP
#define SNOWFLAKECLIENT_CSPATESTATIONS_HPP

#include <string>
#include <picojson.h>
#include <curl/curl.h>
#include <boost/url.hpp>
#include "HttpClient.hpp"
#include "AWSUtils.hpp"

namespace Snowflake {

namespace Client {

  enum class AttestationType {
    AWS,
    AZURE,
    GCP,
    OIDC
  };

  inline const char* stringFromAttestationType(AttestationType type)
  {
    switch (type) {
      case AttestationType::AWS:
        return "AWS";
      case AttestationType::AZURE:
        return "AZURE";
      case AttestationType::GCP:
        return "GCP";
      case AttestationType::OIDC:
        return "OIDC";
    }

    return "UNKNOWN";
  }

  struct Attestation {
    AttestationType type;
    std::string credential;
    boost::optional<std::string> issuer;
    boost::optional<std::string> subject;
    boost::optional<std::string> arn;

    static Attestation makeOidc(const std::string& token, const std::string& issuer, const std::string& subject) {
      return Attestation{AttestationType::OIDC, token, issuer, subject, boost::none};
    }

    static Attestation makeAws(const std::string& credential, const std::string& arn) {
      return Attestation{AttestationType::AWS, credential, boost::none, boost::none, arn};
    }

    static Attestation makeAzure(const std::string& credential, const std::string& issuer, const std::string& subject) {
      return Attestation{AttestationType::AZURE, credential, issuer, subject, boost::none};
    }

    static Attestation makeGcp(const std::string& credential, const std::string& issuer, const std::string& subject) {
      return Attestation{AttestationType::GCP, credential, issuer, subject, boost::none};
    }
  };

  extern const std::unique_ptr<IHttpClient> defaultHttpClient;

  struct AttestationConfig {
    boost::optional<AttestationType> type;
    boost::optional<std::string> token;
    boost::optional<std::string> snowflakeEntraResource;
    IHttpClient* httpClient = IHttpClient::getInstance();
    AwsUtils::ISdkWrapper* awsSdkWrapper = AwsUtils::ISdkWrapper::getInstance();
  };

  boost::optional<Attestation> createAttestation(AttestationConfig& config);
}

}

#endif //SNOWFLAKECLIENT_CSPATESTATIONS_HPP
