
#ifndef SNOWFLAKECLIENT_CSPATESTATIONS_HPP
#define SNOWFLAKECLIENT_CSPATESTATIONS_HPP

#include <string>
#include <boost/optional.hpp>
#include "snowflake/client.h"

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

  inline boost::optional<AttestationType> attestationTypeFromString(const std::string& type)
  {
    if (type == "AWS")
      return AttestationType::AWS;
    if (type == "AZURE")
      return AttestationType::AZURE;
    if (type == "GCP")
      return AttestationType::GCP;
    if (type == "OIDC")
      return AttestationType::OIDC;

    return boost::none;
  }

  struct Attestation {
    AttestationType type;
    std::string credential;
    boost::optional<std::string> issuer;
    boost::optional<std::string> subject;

    static Attestation makeOidc(const std::string& token, const std::string& issuer, const std::string& subject) {
      return Attestation{AttestationType::OIDC, token, issuer, subject};
    }

    static Attestation makeAws(const std::string& credential) {
      return Attestation{AttestationType::AWS, credential, boost::none, boost::none};
    }

    static Attestation makeAzure(const std::string& credential, const std::string& issuer, const std::string& subject) {
      return Attestation{AttestationType::AZURE, credential, issuer, subject};
    }

    static Attestation makeGcp(const std::string& credential, const std::string& issuer, const std::string& subject) {
      return Attestation{AttestationType::GCP, credential, issuer, subject};
    }
  };

  class IHttpClient;
  namespace AwsUtils {
    class ISdkWrapper;
  }

  struct AttestationConfig {
    boost::optional<AttestationType> type;
    boost::optional<std::string> token;
    boost::optional<std::string> snowflakeEntraResource;
    boost::optional<std::string> workloadIdentityImpersonationPath;
    IHttpClient* httpClient = NULL;
    AwsUtils::ISdkWrapper* awsSdkWrapper = NULL;

    boost::optional<std::string> wifHost;

    // When true, AWS WIF uses STS:GetWebIdentityToken (JWT) instead of the
    // legacy GetCallerIdentity presigned-URL credential format.
    bool awsUseOutboundToken = false;

    SF_STATUS configureWIFAttestation(SF_CONNECT* conn);

    std::string getWifHost() const {
      return wifHost.value_or("");
    }

    std::string getWifHostForAws() const;

    std::string getWifHostForGcp() const;
  };

  boost::optional<Attestation> createAttestation(AttestationConfig& config);
}

}

#endif //SNOWFLAKECLIENT_CSPATESTATIONS_HPP
