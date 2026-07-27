#ifndef SNOWFLAKECLIENT_IAUTH_HPP
#define SNOWFLAKECLIENT_IAUTH_HPP

#include <string>
#include <snowflake/SFURL.hpp>
#include "../../lib/snowflake_cpp_util.h"
#include "snowflake/IBase64.hpp"
#include  "../../cpp/lib/AuthenticationWebBrowserRunner.hpp"
#include  "../../cpp/lib/AuthenticationChallengeProvider.hpp"
#include "authenticator.h" 
#include <future>

#define SOCKET_BUFFER_SIZE 20000

namespace Snowflake::Client
{
    namespace IAuth
    {
        struct AuthorizationCodeRequest;
        struct AuthorizationCodeResponse;
        struct AccessTokenRequest;
        struct AccessTokenResponse;
        struct RefreshAccessTokenRequest;

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
        class IAuthWebServer
        {
        public:
#ifdef _WIN32
            SOCKET m_socket_descriptor; // socket
            SOCKET m_socket_desc_web_client; // socket (client)
#else
            int m_socket_descriptor; // socket
            int m_socket_desc_web_client; // socket (client)
#endif
            IAuthWebServer(){}
            virtual ~IAuthWebServer(){}

            virtual void start();
            virtual void stop();
            virtual int getPort();
            virtual void startAccept();
            virtual std::string getToken();
            void setTimeout(int timeout);
            virtual bool receive();
            virtual void respond(std::string httpError, std::string message);
            void fail(std::string httpError, std::string message, std::string failureResponse);

            virtual int start(std::string host, int port, std::string path) = 0;
            virtual void startAccept(std::string state) = 0;

            virtual bool isConsentCacheIdToken() = 0;
            virtual void parseAndRespondGetRequest(char** rest_mesg) = 0;
            virtual bool parseAndRespondOptionsRequest(std::string response) = 0;
            virtual void parseAndRespondPostRequest(std::string response) = 0;

            std::vector<std::string> splitString(const std::string& s, char delimiter);

        protected:
            int m_port = 0; // port to listen, 0 for random port to be used
            int m_real_port = 0; // actual port used when randomly picked
            int m_timeout = SF_BROWSER_RESPONSE_TIMEOUT;

            std::string m_host = "127.0.0.1";
            std::string m_path;
            std::string m_token;
            const char* m_className;

            const char* HTTP_OK = "200 OK";
            const char* HTTP_BAD_REQUEST = "400 Bad Request";
        };

        class OAuthTokenListenerWebServer : public IAuthWebServer
        {
        private:

            std::string m_state;
            std::string m_redirectUri;

        public:
            OAuthTokenListenerWebServer();
            int start(std::string host, int port, std::string path) override;
            void startAccept(std::string state) override;
            bool isConsentCacheIdToken() override;
        private:
            bool parseAndRespondOptionsRequest(std::string response) override;
            void parseAndRespondPostRequest(std::string response) override;
            void parseAndRespondGetRequest(char** rest_mesg) override;
            void respondUnsupportedRequest(std::string method);
            constexpr static const char* successMessage = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"/>\n"
                "<title>Authorization Code Granted for Snowflake</title></head>\n"
                "<body><h4>Your identity was confirmed</h4>"
                "Access to Snowflake has been granted.\n"
                "You can close this window now and go back where you started from.\n"
                "</body></html>";
            constexpr static const char* failureMessage = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"/>\n"
                "<title>Authorization Code Failed for Snowflake</title></head>\n"
                "<body><h4>Could not validate your identity</h4>"
                "Access to Snowflake could not have been granted.\n"
                "You can close this window now and try again.\n"
                "</body></html>";
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
            IAuthenticatorOKTA(IDPAuthenticator* idp = nullptr);

            virtual ~IAuthenticatorOKTA() {};
            virtual void authenticate();
            virtual void updateDataMap(jsonObject_t& dataMap);

            std::unique_ptr<IDPAuthenticator> m_idp;
            /**
             * Extract post back url from samel response. Input is in HTML format.
            */
            std::string extractPostBackUrlFromSamlResponse(std::string html);

        protected:
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
            IAuthenticatorExternalBrowser(IAuthWebServer* authWebServer = nullptr, IDPAuthenticator* idp = nullptr, IAuthenticationWebBrowserRunner* webBrowserRunner = nullptr);

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
            std::unique_ptr<IAuthWebServer> m_authWebServer;
            IAuthenticationWebBrowserRunner* m_webBrowserRunner;
            std::unique_ptr<IDPAuthenticator> m_idp;
            std::string m_proofKey;
            std::string m_token;
            std::string m_user;
            bool m_consentCacheIdToken;
            bool m_disable_console_login;
            std::string m_origin;
            int64 m_browser_response_timeout;
        };

        class AuthWebServer : public IAuthWebServer
        {
        public:
            AuthWebServer();

            virtual ~AuthWebServer();

            int start(std::string host, int port, std::string path) override;
            void startAccept(std::string state) override {
                SF_UNUSED(state);
            };
            bool isConsentCacheIdToken() override;

        protected:

            bool m_consent_cache_id_token;
            std::string m_origin;


            void respond(std::string queryParameters);
            void respondJson(picojson::value& json);

            std::string unquote(std::string src);
            std::vector<std::pair<std::string, std::string>> splitQuery(std::string query);
        private:
            bool parseAndRespondOptionsRequest(std::string response) override;
            void parseAndRespondPostRequest(std::string response) override;
            void parseAndRespondGetRequest(char** rest_mesg) override;

            constexpr static const char* successMessage = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"/>\n"
                "<title>SAML Response for Snowflake</title></head>\n"
                "<body>\n"
                "Your identity was confirmed and propagated to Snowflake "
                "You can close this window now and go back where you "
                "started from.\n"
                "</body></html>";

            constexpr static const char* failureMessage = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"/>\n"
                "<title>SAML Response for Snowflake</title></head>\n"
                "<body><h4>Could not validate your identity</h4>"
                "Access to Snowflake could not have been granted to libsnowflakeclient.\n"
                "You can close this window now and try again.\n"
                "</body></html>";

        };

        class IAuthenticatorOAuth : public IAuthenticator, public AuthErrorHandler
        {

        public:
            static const std::string S_LOCALHOST;
            static const std::string S_LOCALHOST_URL;
            static const std::string S_OAUTH_DEFAULT_AUTHORIZATION_URL_POSTFIX;
            static const std::string S_OAUTH_DEFAULT_TOKEN_URL_POSTFIX;

            IAuthenticatorOAuth(IAuthWebServer* authWebServer = nullptr,
                IAuthenticationWebBrowserRunner* webBrowserRunner = nullptr);

            virtual ~IAuthenticatorOAuth() {};

            virtual void authenticate() override;
            void updateDataMap(jsonObject_t& dataMap) override;

        protected:
            virtual void resetTokens(std::string accessToken, std::string refreshToken) = 0;
            virtual bool executeRestRequest(SFURL& endPoint, const std::string& body, jsonObject_t& resp) = 0;

            AuthenticatorType m_oauthFlow;
            SFURL m_authEndpoint;
            SFURL m_tokenEndpoint;
            std::string m_clientId;
            std::string m_clientSecret;
            std::string m_authScope;
            SFURL m_redirectUri;
            bool m_redirectUriDynamicDefault;
            bool m_singleUseRefreshTokens;
            int8 m_browserResponseTimeout = 120;
            std::string m_oauth_refresh_token;
            std::string m_token;
            AuthenticationChallengeBaseProvider* m_challengeProvider;
            std::unique_ptr<IAuthWebServer> m_authWebServer;
            IAuthenticationWebBrowserRunner* m_webBrowserRunner;

        private:
            void authorizationCodeFlow();
            void clientCredentialsFlow();
            bool refreshAccessTokenFlow();
            void handleInvalidResponse(const bool success, const std::string& errorMessage);
            AuthorizationCodeResponse executeAuthorizationCodeRequest(AuthorizationCodeRequest& authorizationCodeRequest);
            AccessTokenResponse executeAccessTokenRequest(AccessTokenRequest& request);
            AccessTokenResponse executeRefreshAccessTokenRequest(RefreshAccessTokenRequest& request);
            std::string createAccessTokenRequestBody(AccessTokenRequest& request);
            std::string createRefreshAccessTokenRequestBody(const RefreshAccessTokenRequest& request);
            void startWebBrowser(const std::string& url);
            static std::string oauthWebServerTask(IAuthWebServer* authWebServer, const SFURL& redirectUrl, const std::string& state, const int browserResponseTimeout);
            std::future<std::string> asyncStartOAuthWebserver(const AuthorizationCodeRequest& authorizationCodeRequest) const;
            void refreshDynamicRedirectUri(int portUsed, AuthorizationCodeRequest& authorizationCodeRequest);
            std::string oauthAuthorizationFlow();


            typedef Snowflake::Client::Util::IBase64 Base64;
        };

        class AuthException : public std::exception
        {
            std::string problem;
        public:
            explicit AuthException(std::string message) : problem(message) {}

            const std::string& cause() const { return problem; };
        };
    } // namespace IAuth

        std::string UrlEncode(std::string url);
        std::string maskOAuthSecret(const std::string& secret);
        std::string maskOAuthSecret(SFURL& secret);
} // namespace Snowflake::Client

#endif //SNOWFLAKECLIENT_IAUTH_HPP
