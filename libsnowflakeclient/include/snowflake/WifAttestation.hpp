
#ifndef SNOWFLAKECLIENT_CSPATESTATIONS_HPP
#define SNOWFLAKECLIENT_CSPATESTATIONS_HPP

#include <string>
#include <picojson.h>
#include <curl/curl.h>
#include <boost/url.hpp>
#include "HttpClient.hpp"

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
    std::string issuer = "";
    std::string subject = "";
  };

  extern const std::unique_ptr<IHttpClient> defaultHttpClient;

  struct AttestationConfig {
    boost::optional<AttestationType> type;
    boost::optional<std::string> token;
    boost::optional<std::string> snowflakeEntraResource;
    IHttpClient* httpClient = defaultHttpClient.get();
  };

  boost::optional<Attestation> createAttestation(AttestationConfig& config);
}

}

#endif //SNOWFLAKECLIENT_CSPATESTATIONS_HPP
