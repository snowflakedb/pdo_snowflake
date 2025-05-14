#ifndef SNOWFLAKECLIENT_IAUTH_HPP
#define SNOWFLAKECLIENT_IAUTH_HPP

#include <string>
#include <snowflake/SFURL.hpp>
#include "../../lib/snowflake_cpp_util.h"
#include "snowflake/IBase64.hpp"

namespace Snowflake
{
namespace Client
{
namespace IAuth
{
    class AuthErrorHandler
    {
    public:
        AuthErrorHandler() {};
        virtual ~AuthErrorHandler() {};

        bool isError();
        const char* getErrorMessage();

        std::string m_errMsg;
    };

#if defined(WIN32) || defined(_WIN64)
    /**
     * Winsock start and cleanup
     */
    class AuthWinSock : public AuthErrorHandler
    {
    public:
        AuthWinSock();
        ~AuthWinSock();
    };
#endif

    /**
  * Web Server for external Browser authentication
  */
    class IAuthWebServer : public AuthErrorHandler
    {
    public:
        IAuthWebServer(){}

        virtual ~IAuthWebServer()
        {}

        virtual void start() = 0;
        virtual void stop() = 0;
        virtual int getPort() = 0;
        virtual void startAccept() = 0;
        virtual bool receive() = 0;
        virtual std::string getSAMLToken() = 0;
        virtual bool isConsentCacheIdToken() = 0;
        virtual void setTimeout(int timeout) = 0;
    };

    /**
     * Authenticator
     */
    class IAuthenticator
    {
    public:

        IAuthenticator() : m_renewTimeout(0)
        {}

        virtual ~IAuthenticator()
        {}

        virtual void authenticate() = 0;

        virtual void updateDataMap(jsonObject_t& dataMap) = 0;

        // Retrieve authenticator renew timeout, return 0 if not available.
        // When the authenticator renew timeout is available, the connection should
        // renew the authentication (call renewDataMap) for each time the
        // authenticator specific timeout exceeded within the entire login timeout.
        int64 getAuthRenewTimeout()
        {
            return m_renewTimeout;
        }

        // Renew the autentication and update datamap.
        // The default behavior is to call authenticate() and updateDataMap().
        virtual void renewDataMap(jsonObject_t& dataMap);;

    protected:
        int64 m_renewTimeout;
    };

    class IDPAuthenticator : public AuthErrorHandler
    {
    public:
        IDPAuthenticator()
        {};

        virtual ~IDPAuthenticator()
        {};

        bool getIDPInfo(jsonObject_t& dataMap);

        virtual SFURL getServerURLSync();
        /*
         * Get IdpInfo for OKTA and SAML 2.0 application
         */
        virtual bool curlPostCall(SFURL& url, const jsonObject_t& body, jsonObject_t& resp) = 0;
        virtual bool curlGetCall(SFURL& url, jsonObject_t& resp, bool parseJSON, std::string& raw_data, bool& isRetry) = 0;

        std::string tokenURLStr;
        std::string ssoURLStr;
        std::string proofKey;

        //These fields should be definied in the child class.
        std::string m_authenticator;
        std::string m_account;
        std::string m_port;
        std::string m_host;
        std::string m_protocol;
        int8 m_retriedCount;
        int64 m_retryTimeout;
    };

    class IAuthenticatorOKTA : public IAuthenticator, public AuthErrorHandler
    {
    public:
        IAuthenticatorOKTA() {};

        virtual ~IAuthenticatorOKTA() {};

        virtual void authenticate();

        virtual void updateDataMap(jsonObject_t& dataMap);

        IDPAuthenticator* m_idp;

        /**
         * Extract post back url from samel response. Input is in HTML format.
        */
        std::string extractPostBackUrlFromSamlResponse(std::string html);

    protected:
        //These fields should be definied in the child class.
        std::string m_user;
        std::string m_password;
        std::string m_appID;
        std::string m_appVersion;
        bool m_disableSamlUrlCheck;

        std::string oneTimeToken;
        std::string m_samlResponse;
    };

    class IAuthenticatorExternalBrowser : public IAuthenticator, public AuthErrorHandler
    {
    public:
        IAuthenticatorExternalBrowser() {};

        virtual ~IAuthenticatorExternalBrowser() {};

        int getPort(void);

        virtual void authenticate();

        void updateDataMap(jsonObject_t& dataMap);

        /**
         * Start web browser so that the user can type IdP user and password
         * @param ssoUrl SSO URL
         */
        virtual void startWebBrowser(std::string ssoUrl);

        /**
         * Get Login URL for multiple SAML
         * @param out the login URL
         * @param port port number listening to get SAML token
         */
        virtual void getLoginUrl(std::map<std::string, std::string>& out, int port);

        /**
         * Generate the proof key
         * @return The proof key
         */
        virtual std::string generateProofKey();

    private:
        typedef Snowflake::Client::Util::IBase64 Base64;

    protected:
#ifdef _WIN32
        AuthWinSock m_authWinSock;
#endif
        IAuthWebServer* m_authWebServer;
        IDPAuthenticator* m_idp;
        std::string m_proofKey;
        std::string m_token;
        std::string m_user;
        bool m_consentCacheIdToken;
        bool m_disable_console_login;
        std::string m_origin;
        int64 m_browser_response_timeout;

#ifdef __APPLE__
        void openURL(const std::string& url_str);
#endif
    };
} // namespace IAuth
} // namespace Client
} // namespace Snowflake

#endif //SNOWFLAKECLIENT_IAUTH_HPP
