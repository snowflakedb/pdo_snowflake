********************************************************************************
PHP PDO driver for Snowflake
********************************************************************************

.. image:: https://codecov.io/gh/snowflakedb/pdo_snowflake/branch/master/graph/badge.svg
    :target: https://codecov.io/gh/snowflakedb/pdo_snowflake

.. image:: http://img.shields.io/:license-Apache%202-brightgreen.svg
    :target: http://www.apache.org/licenses/LICENSE-2.0.txt

Snowflake provides a driver that uses the 
`PHP Data Objects (PDO) extension <https://www.php.net/manual/en/book.pdo.php>`_.
to connect to the Snowflake database.

.. contents::

Prerequisites
================================================================================

Operating system
For a list of the operating systems supported by Snowflake clients, see `Operating system support <https://docs.snowflake.com/en/release-notes/requirements#label-client-operating-system-support>`_.

To build the Snowflake PHP PDO Driver, the following software must be installed:

- On Windows: Visual Studio 2019
- On Linux:

  - gcc 8.3 or higher. **Note**: on certain OS (e.g. Centos 7) the preinstalled gcc/libstdc++ version is below the required minimum. For Centos 7, this is 4.8.5, which is below the requirement. Building and using the PHP PDO driver might be unsuccessful on such OS's until the prerequisite is fulfilled, i.e. libraries upgraded to at least the minimum version.
  - cmake 2.8 or higher

- On macOS:

  - clang
  - cmake 2.8 or higher

To install and use the Snowflake PHP PDO Driver, you must have the following software installed:

- PHP 8.4, 8.3, 8.2 or 8.1 (Note: support for PHP 8.0 or lower is deprecated)
- the :code:`php-pdo` extension
- the :code:`php-json` extension

To build the driver, you must install the PHP development package for your operating system.

If you are using PHP with an application server or web server (e.g. Apache or nginx), configure the server to handle
requests for PHP pages. See the `PHP documentation <https://www.php.net/manual/en/install.php>`_ for details.

Building the PHP PDO Driver
================================================================================

The following sections explain how to build the PHP PDO Driver on Linux, macOS, and Windows.

:Note: Snowflake PHP PDO driver does not yet support ARM/AARCH64 architecture on Linux.
While this feature is implemented, you can consider using the Snowflake ODBC driver https://developers.snowflake.com/odbc/ for PHP which supports multiple architectures.

Building the Driver on Linux and macOS
--------------------------------------

#. Download and install the PHP binaries, or build and install PHP from the source code.

   If you need to build PHP from the source code, see
   `Building PHP source code <https://github.com/php/php-src/blob/master/README.md#building-php-source-code>`_.

#. Set the :code:`PHP_HOME` environment variable to the path to the :code:`bin` directory containing the :code:`phpize`
   executable.

   For example, if the :code:`phpize` executable is in :code:`/usr/bin`, run the following command:

   .. code-block:: bash

       export PHP_HOME=/usr

#. Clone the :code:`pdo_snowflake` repository, and run the script to build the driver:

   .. code-block:: bash

       git clone https://github.com/snowflakedb/pdo_snowflake.git
       cd pdo_snowflake
       ./scripts/build_pdo_snowflake.sh

#. Run the following command to verify that the driver can be loaded into memory successfully:

   .. code-block:: bash

       $PHP_HOME/bin/php -dextension=modules/pdo_snowflake.so -m | grep pdo_snowflake

   :code:`pdo_snowflake` should appear in the output from the command.

Building the Driver on Windows
------------------------------

.. |win-vs-version| replace:: VS16 
.. |win-php-version| replace:: 8.1.28


**Note**: Snowflake supports only thread-safe versions of PHP.

You must install Microsoft Visual Studio 2019 (VS16) or 2022 (VS17) with the C++ development installer option.

To build the PHP driver for Windows:

#. Download and install PHP:

   #. Download the PHP version binaries from `<https://windows.php.net/downloads/releases/>`_,
      such as `<https://windows.php.net/downloads/releases/php-8.1.28-Win32-vs16-x64.zip>`_.

      .. note::

       The Snowflake PHP driver does not support x86 architecture or Windows NTS, so don't download packages that
       include ``nts`` or ``x86`` in the package name.

   #. Unzip the file to the desired directory, such as :code:`C:\php`.

#. Clone the :code:`pdo_snowflake` repository:

   .. code-block:: batch

       git clone https://github.com/snowflakedb/pdo_snowflake.git
       cd pdo_snowflake


   **Choose a target directory where none of the subdirectories contain any spaces or special characters on the path.** E.g. :code:`C:\temp\pdo_snowflake`.
   Without this, one of the setup scripts (`phpsdk-starter.bat`) will fail during step 4. 

#. Run the script to download the PHP SDK:

   .. code-block:: batch

       .\scripts\setup_php_sdk.bat <arch> <build> <visual studio version> <path to PHP SDK>

   where:

   - :code:`<arch>` is your CPU architecture (Currently, the driver only supports :code:`x64`).
   - :code:`<build>` is the type of binary that you want to build (:code:`Release` or :code:`Debug`).
   - :code:`<visual studio version>` is the version of Visual Studio that you are using (Currently, the driver only supports :code:`VS16` and :code:`VS17`).
   - :code:`<path to PHP SDK>` is the path to the directory where the PHP SDK should be downloaded.
     **Do not create this directory.** The script creates this directory for you when downloading the PHP SDK.

   For example:

   .. parsed-literal::

       .\\scripts\\setup_php_sdk.bat x64 Release VS16 C:\\php-sdk

#. Download and build the PHP source code.

   Run the script to download the PHP source and build PHP:

   .. code-block:: batch

       .\scripts\run_setup_php.bat <arch> <build> <visual studio version> <full PHP version> <path to PHP SDK>

   For :code:`<arch>`, :code:`<build>`, :code:`<visual studio version>`, and :code:`<path to PHP SDK>`, specify the same values
   that you used in the previous step.

   For :code:`<full PHP version>`, specify the full version number of the PHP binary you installed (e.g. :code:`8.1.28`).

   For example:

   .. parsed-literal::

       .\\scripts\\run_setup_php.bat x64 Release |win-vs-version| |win-php-version| C:\\php-sdk

#. Run the script to build the driver:

   .. code-block:: batch

       .\scripts\run_build_pdo_snowflake.bat <arch> <build> <visual studio version> <full PHP version> <path to PHP SDK>

   For example:

   .. parsed-literal::

       .\\scripts\\run_build_pdo_snowflake.bat x64 Release |win-vs-version| |win-php-version| C:\\php-sdk

#. Copy :code:`php_pdo_snowflake.dll` from the directory where you built the driver under the path to PHP SDK
   For example:

   .. parsed-literal::

       C:\\php-sdk\\phpmaster\\vs16\\x64\\php-src\\x64\\Release_TS

   to the PHP extension directory. Usually, the PHP extension directory is the :code:`ext` subdirectory in the
   directory where PHP is installed. To find the PHP extension directory, run:

   .. code-block:: bash

       C:\php\php.exe -i | findstr "extension_dir"

#. Run the following command to verify that the driver can be loaded into memory successfully:

   .. code-block:: batch

       C:\php\php.exe -dextension=ext\php_pdo_snowflake.dll -m

   :code:`pdo_snowflake` should appear in the output from the command.

Installing the PHP PDO Driver
================================================================================

The following sections explain how to install the PHP PDO Driver on Linux, macOS, and Windows.

Installing the Driver on Linux and macOS
----------------------------------------

#. Copy :code:`pdo_snowflake.so` from the :code:`modules` subdirectory in the repository to the PHP extension directory.

   To find the PHP extension directory, run:

   .. code-block:: bash

       $PHP_HOME/bin/php -i | grep '^extension_dir'

#. Copy :code:`cacert.pem` from the :code:`libsnowflakeclient` subdirectory in the repository to the PHP configuration directory
   containing the PHP configuration files.

   To find the PHP configuration directory, run:

   .. code-block:: bash

       $PHP_HOME/bin/php -ini

   In the output if the item of :code:`Scan for additional .ini files in` is not :code:`(none)`, use that as the PHP configuration
   directory so we can have separated configuration file for Snowflake, otherwise use :code:`Configuration File (php.ini) Path:`.

#. In the same directory that contains the PHP configuration files, create a config file named :code:`20-pdo_snowflake.ini` that
   contains the following settings (or in case using :code:`Configuration File (php.ini) Path:`, add following lines to :code:`php.ini`):

   .. code-block:: ini

       extension=pdo_snowflake.so
       pdo_snowflake.cacert=<path to PHP config directory>/cacert.pem
       # pdo_snowflake.logdir=/tmp             # location of log directory
       # pdo_snowflake.loglevel=DEBUG          # log level

   where :code:`<path to PHP config directory>` is the path to the directory where you copied the :code:`cacert.pem` file in the previous step.

#. If you are using PHP with an application server or web server (e.g. Apache or nginx), restart the server.


Installing the Driver on Windows
--------------------------------

#. Copy :code:`php_pdo_snowflake.dll` from the directory where you built the driver under the path to PHP SDK
   For example:

   .. parsed-literal::

       C:\\php-sdk\\phpmaster\\vs16\\x64\\php-src\\x64\\Release_TS

   to the PHP extension directory. Usually, the PHP extension directory is the :code:`ext` subdirectory in the
   directory where PHP is installed. To find the PHP extension directory, run:

   .. code-block:: bash

       C:\php\php.exe -i | findstr "extension_dir"

#. Copy :code:`cacert.pem` from the :code:`libsnowflakeclient` subdirectory in the repository to the directory containing the
   PHP configuration files (e.g. :code:`C:\php` if PHP is installed in that directory).

#. Add the following lines to your :code:`php.ini` file:

   .. code-block:: ini

       extension=php_pdo_snowflake.dll
       pdo_snowflake.cacert=<path to PHP config directory>\cacert.pem
       ; pdo_snowflake.logdir=C:\path\to\logdir                ; location of log directory
       ; pdo_snowflake.loglevel=DEBUG                          ; log level

   where :code:`<path to PHP config directory>` is the path to the directory where you copied the :code:`cacert.pem` file in the previous step.

#. If you are using PHP with an application server or web server (e.g. Apache or nginx), restart the server.

Using the Driver
================================================================================

The next sections explain how to use the driver in a PHP page.

Note
----------------------------------------------------------------------

This driver currently does not support GCP regional endpoints. Please ensure that any workloads using through this driver do not require support for regional endpoints on GCP. If you have questions about this, please contact Snowflake Support.

Connecting to the Snowflake Database
----------------------------------------------------------------------

To connect to the Snowflake database, create a new :code:`PDO` object, as explained in
`the PHP PDO documentation <https://www.php.net/manual/en/pdo.connections.php>`_.
Specify the data source name (:code:`dsn`) parameter as shown below:

.. code-block:: php

    $dbh = new PDO("snowflake:account=<account_name>", "<user>", "<password>");

where:

- :code:`<account_name>` is
  `your Snowflake account name <https://docs.snowflake.com/en/user-guide/connecting.html#your-snowflake-account-name>`_.
- :code:`<user>` is the login name of the user for the connection.
- :code:`<password>` is the password for the specified user.

Dependes on the region where your account being hosted, you might need to use :code:`region` parameter to specify the region
or append the region to the :code:`account` parameter.
You might also need to append :code:`cloud` in :code:`region` parameter in the format of :code:`<region>.<cloud>`, or do the
same when you append it to the :code:`account` parameter.

where:

- :code:`<region>` is the identifier for the cloud region.
- :code:`<cloud>` is the identifier for the cloud platform (aws, azure, or gcp).

.. code-block:: php

    $dbh = new PDO("snowflake:account=testaccount.us-east-2.aws", "user", "password");
    $dbh = new PDO("snowflake:account=testaccount;region=us-east-2.aws", "user", "password");

You can specify the host name for your account directly as shown below instead of using `account` and `region`:

.. code-block:: php

    $dbh = new PDO("snowflake:host=<host_name>", "<user>", "<password>");

where:

- :code:`<host_name>` is the host name for your account, usually in the format of :code:`<account_identifier>.snowflakecomputing.com`

where:

- :code:`<account_identifier>` is your account identifier. For information about account identifiers, see `Account identifiers <https://docs.snowflake.com/en/user-guide/admin-account-identifier>`_.

Using Key Pair Authentication
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The PHP PDO driver supports key pair authentication and key rotation.

You must first complete the initial configuration for key pair authentication as shown in 
`Key Pair Authentication & Key Pair Rotation <https://docs.snowflake.com/en/user-guide/key-pair-auth.html#key-pair-authentication-key-pair-rotation>`_.

To connect to the Snowflake database using key pair authentication, create a new :code:`PDO` object, as explained in the
`PHP PDO documentation <https://www.php.net/manual/en/pdo.connections.php>`_.
Specify the data source name (:code:`dsn`) parameter as shown below:

.. code-block:: php

    $dbh = new PDO("account=<account name>;authenticator=SNOWFLAKE_JWT;priv_key_file=<path>/rsa_key.p8;priv_key_file_pwd=<private_key_passphrase>", 
                    "<username>", "");

where:

- :code:`<account_name>` Specifies your
  `Snowflake account name <https://docs.snowflake.com/en/user-guide/connecting.html#your-snowflake-account-name>`_.
- :code:`authenticator = SNOWFLAKE_JWT` Specifies that you want to authenticate the Snowflake connection using key pair authentication with JSON Web Token (JWT).
- :code:`priv_key_file = <path>/rsa_key.p8` Specifies the local path to the private key file you created (i.e. :code:`rsa_key.p8`).
- :code:`priv_key_file_pwd = <private_key_passphrase>` Specifies the passphrase to decrypt the private key file. If you using an unecrypted private key file, omit this parameter.
- :code:`<username>` Specifies the login name of the user for the connection.
- :code:`""` Specifies the password for the specified user. The parameter is required. When using key-pair authentication, specify an empty string.

Using Multi-factor authentication (MFA)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The PHP PDO driver supports Multi-factor authentication.
Guidance can be found in
`Multi-factor authentication (MFA) <https://docs.snowflake.com/en/user-guide/security-mfa>`_.

To connect to the Snowflake database using Duo-generated passcode instead of the push mechanism,
Specify the data source name (:code:`dsn`) parameter as shown below:

.. code-block:: php

    $dbh = new PDO("account=<account name>;passcode=<duo_generated_passcode>",
                    "<username>", "<password>");

where:

- :code:`<account_name>` Specifies your
  `Snowflake account name <https://docs.snowflake.com/en/user-guide/connecting.html#your-snowflake-account-name>`_.
- :code:`passcode = <duo_generated_passcode>` Specifies the Duo-generated passcode.
- :code:`<username>` Specifies the login name of the user for the connection.
- :code:`<password>` Specifies the password for the specified user.

You can also use passcodeinpassword and set passcode and password concatenated, in the form of <password_string><passcode_string>,
Specify the data source name (:code:`dsn`) parameter as shown below:

.. code-block:: php

    $dbh = new PDO("account=<account name>;passcodeinpassword=on",
                    "<username>", "<password><passcode>");

where:

- :code:`<account_name>` Specifies your
  `Snowflake account name <https://docs.snowflake.com/en/user-guide/connecting.html#your-snowflake-account-name>`_.
- :code:`passcodeinpassword=on` Specifies the Duo-generated passcode.
- :code:`<username>` Specifies Duo passcode is embedded in the password.
- :code:`<password><passcode>` Specifies the password and passcode concatenated.

Configuring OCSP Checking
----------------------------------------------------------------------

By default, OCSP (Online Certificate Status Protocol) checking is enabled and is set per PDO connection.

To disable OCSP checking for a PDO connection, set :code:`disableocspchecks=true` in the DSN connection string. For example:

.. code-block:: php

    $dbh = new PDO("snowflake:account=testaccount;disableocspchecks=true", "user", "password");

By default, OCSP checking uses fail-open approach. For more details see `Fail-Open or Fail-Close behavior <https://docs.snowflake.com/en/user-guide/ocsp#fail-open-or-fail-close-behavior>`_.

To switch to use fail-close approach, set :code:`ocspfailopen=false` in the DSN connection string. For example:

.. code-block:: php

    $dbh = new PDO("snowflake:account=testaccount;ocspfailopen=false", "user", "password");

Proxy
----------------------------------------------------------------------

PHP PDO Driver for Snowflake supports HTTP and HTTPS proxy connections using environment variables. To use a proxy server configure the following environment variables:

- http_proxy
- https_proxy
- no_proxy

.. code-block:: bash

    export http_proxy="[protocol://][user:password@]machine[:port]"
    export https_proxy="[protocol://][user:password@]machine[:port]"

More info can be found on the `libcurl tutorial`__ page.

.. __: https://curl.haxx.se/libcurl/c/libcurl-tutorial.html#Proxies

Since version 1.2.5 of the driver, you can set individual proxy settings which are only valid for the PDO Snowflake driver. Use the:

- proxy
- no_proxy

directives on the connection string. Example:

.. code-block:: php

   $dbh = new PDO("snowflake:account=<account_name>;proxy=my.pro.xy;no_proxy=.mycompany.com", "<username>", "<password>");

Syntax is the same as is documented for the `Snowflake ODBC driver <https://docs.snowflake.com/en/user-guide/odbc-parameters.html#using-connection-parameters>`_


Performing a Simple Query
----------------------------------------------------------------------

The following example connects to the Snowflake database and performs a simple query.
Before using this example, set the :code:`$account`, :code:`$user`, and :code:`$password` variables to your account, login name,
and password.
The warehouse, database, schema parameters are optional, but can be specified to determine the context of the connection in which the query will be run.
In this example, we'll use those too.

.. code-block:: php

  <$php
    $account = "<account_name>";
    $user = "<user_name>";
    $password = "<password>";
    $warehouse = "<warehouse_name>";
    $database = "<database_name>";
    $schema = "<schema_name>";

    $dbh = new PDO("snowflake:account=$account;warehouse=$warehouse;database=$database;schema=$schema", $user, $password);
    $dbh->setAttribute( PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION );
    echo "Connected\n";

    $sth = $dbh->query("select 1234");
    while ($row=$sth->fetch(PDO::FETCH_NUM)) {
        echo "RESULT: " . $row[0] . "\n";
    }
    $dbh = null;
    echo "OK\n";
  $>

**Note**: `PUT` and `GET` queries are not supported in the driver.

Setting timeouts
----------------------------------------------------------------------

The following parameters are exposed in the PHP PDO Driver to affect the behaviour regarding various timeouts:

- :code:`logintimeout` : The timeout for login requests. Defaults to 300 seconds
- :code:`retrytimeout`: The timeout for query requests, large query result chunk download actions, and request retries, . Defaults to 300 seconds; cannot be decreased, only set higher than 300.
- :code:`maxhttpretries`: The maximum number of retry attempts. Defaults to 7; cannot be decreased, only set higher than 7.

Example usage:

.. code-block:: php

   $dbh = new PDO("$dsn;application=phptest;authenticator=snowflake;priv_key_file=tests/p8test.pem;priv_key_file_pwd=password;disablequerycontext=true;includeretryreason=false;logintimeout=250;maxhttpretries=8;retrytimeout=350", $user, $password);


Running Tests For the PHP PDO Driver on Linux and macOS
================================================================================

In order to run the test scripts, you must have :code:`jq` installed.

Prepare Tests
----------------------------------------------------------------------

#. Create a parameter file :code:`parameters.json` under :code:`pdo_snowflake` directory:

   .. code-block:: none

       {
           "testconnection": {
               "SNOWFLAKE_TEST_USER":      "<your_user>",
               "SNOWFLAKE_TEST_PASSWORD":  "<your_password>",
               "SNOWFLAKE_TEST_ACCOUNT":   "<your_account>",
               "SNOWFLAKE_TEST_WAREHOUSE": "<your_warehouse>",
               "SNOWFLAKE_TEST_DATABASE":  "<your_database>",
               "SNOWFLAKE_TEST_SCHEMA":    "<your_schema>",
               "SNOWFLAKE_TEST_ROLE":      "<your_role>"
           }
       }

#. Set the workfolder to :code:`pdo_snowflake` repository. e.g. Call :code:`cd pdo_snowflake`.

#. Call :code:`env.sh` script to set the test connection parameters in the environment variables.

   .. code-block:: bash
   
       /bin/bash -c "source ./scripts/env.sh && env | grep SNOWFLAKE_TEST > testenv.ini"


Run Tests
----------------------------------------------------------------------

To run the tests, do the following:

.. code-block:: bash

    REPORT_EXIT_STATUS=1 NO_INTERACTION=true make test

Profile
----------------------------------------------------------------------

You can use :code:`callgrind` to profile PHP PDO programs. For example, run :code:`tests/selectnum.phpt` testcase using :code:`valgrind` along with :code:`callgrind` option.

.. code-block:: bash

    valgrind --tool=callgrind $PHP_HOME/bin/php -dextension=modules/pdo_snowflake.so tests/selectnum.phpt
    callgrind_annotate callgrind.out.*

Check memory leak by valgrind
----------------------------------------------------------------------

Use :code:`valgrind` to check memeory leak. Both C API and PHP PDO can run along with :code:`valgrind`. For example, run :code:`tests/selectnum.phpt` testcase using :code:`valgrind` by the following command.

.. code-block:: bash

    valgrind --leak-check=full $PHP_HOME/bin/php -dextension=modules/pdo_snowflake.so tests/selectnum.phpt

and verify no error in the output:

.. code-block:: bash

     ERROR SUMMARY: 0 errors from 0 contexts ...

Running Tests For the PHP PDO Driver on Windows
================================================================================

In order to run the test scripts, you must have :code:`jq` installed.

Prepare Tests
----------------------------------------------------------------------

#. Create a parameter file :code:`parameters.json` under :code:`pdo_snowflake` directory:

   .. code-block:: none
   
       {
           "testconnection": {
               "SNOWFLAKE_TEST_USER":      "<your_user>",
               "SNOWFLAKE_TEST_PASSWORD":  "<your_password>",
               "SNOWFLAKE_TEST_ACCOUNT":   "<your_account>",
               "SNOWFLAKE_TEST_WAREHOUSE": "<your_warehouse>",
               "SNOWFLAKE_TEST_DATABASE":  "<your_database>",
               "SNOWFLAKE_TEST_SCHEMA":    "<your_schema>",
               "SNOWFLAKE_TEST_ROLE":      "<your_role>"
           }
       }

#. Set the workfolder to :code:`pdo_snowflake` repository. e.g. Call :code:`cd pdo_snowflake`.

#. Set the :code:`PHP_HOME` environment variable to the php install directory, such as :code:`C:\php`.

#. Install the driver following the instructions above.

#. Call :code:`env.bat` script to set the test connection parameters.

   .. code-block:: batch
   
       .\scripts\env.bat


Run Tests
----------------------------------------------------------------------

To run the tests, do the following:

.. code-block:: bash

    %PHP_HOME%\php.exe <path to PHP SDK>\phpmaster\<visual studio version>\<arch>\php-src\run-tests.php .\tests

where:

- :code:`<arch>` is your CPU architecture (Currently :code:`x64` is the only supported one).
- :code:`<visual studio version>` is the version of Visual Studio that you are using (Currently :code:`VS16` and :code:`VS17` are supported).
- :code:`<path to PHP SDK>` is the path to the directory where the PHP SDK should be downloaded.

Additional Notes
================================================================================

Test Framework
----------------------------------------------------------------------

The PHP PDO Snowflake driver uses phpt test framework. Refer the following documents to write tests.

- https://qa.php.net/write-test.php
- https://qa.php.net/phpt_details.php

Troubleshooting
================================================================================

Cannot load module 'pdo_snowflake' because required module 'pdo' is not loaded
-------------------------------------------------------------------------------

In some environments, e.g., Ubuntu 16, when you run :code:`make test`, the following error message shows up and no test runs.

.. code-block:: bash

    PHP Warning:  Cannot load module 'pdo_snowflake' because required module 'pdo' is not loaded in Unknown on line 0

Ensure the php has PDO:

.. code-block:: bash

    $ php -i | grep -i "pdo support"
    PDO support => enabled

If not installed, install the package.

Locate :code:`pdo.so` under :code:`/usr/lib` and specify it in :code:`phpt` files, e.g.,

.. code-block:: bash

    --INI--
    extension=/usr/lib/php/20170718/pdo.so
    pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
    pdo_snowflake.logdir=/tmp
    pdo_snowflake.loglevel=DEBUG

Where is the log files?
----------------------------------------------------------------------

The location of log files are specified by the parameters in php.ini:

.. code-block:: bash

    extension=pdo_snowflake.so
    pdo_snowflake.cacert=/etc/php/8.1/conf.d/cacert.pem
    pdo_snowflake.logdir=/tmp            ; location of log directory
    pdo_snowflake.loglevel=DEBUG         ; log level

where :code:`pdo_snowflake.loglevel` can be :code:`TRACE`, :code:`DEBUG`, :code:`INFO`, :code:`WARN`, :code:`ERROR` and :code:`FATAL`.

Use easy logging while debugging your code
----------------------------------------------------------------------

When debugging an application, increasing the log level can provide more granular information about what the application is doing. The Easy Logging feature simplifies debugging by letting you change the log level and the log file destination using a configuration file (i.e. :code:`sf_client_config.json`).

You typically change log levels only when debugging your application.

:code:`sf_client_config.json` is a JOSN configuration file that is used to define the logging parameters: :code:`log_level` and :code:`log_path`, as follows:

   .. code-block:: none

       {
           "common": {
               "log_level" : "INFO",
               "log_path" :  "/some-path/some-directory"
           }
       }

where:
- :code:`log_level` is the desired logging level.
- :code:`log_path` is the location to store the log files.

   .. code-block:: none
   **Note**
    The driver automatically creates a sub-directory in the specified log_path. For example, if you set :code:`log_path` to :code:`/Users/me/logs`, the drivers creates the :code:`/Users/me/logs/` directory and stores the logs there.

Next, navigate to php.ini to set the path for :code:`sf_client_config.json`:

.. code-block:: ini

    extension=php_pdo_snowflake.dll
    pdo_snowflake.cacert=<path to PHP config directory>\cacert.pem
    ; pdo_snowflake.logdir=C:\path\to\logdir                ; location of log directory
    ; pdo_snowflake.loglevel=DEBUG                          ; log level
    ; pdo_snowflake.clientconfigfile=<path to client config file>/sf_client_config.json

where :code:`<path to PHP config directory>` is the path to the directory where you copied the :code:`cacert.pem` file in the previous step.

where :code:`pdo_snowflake.loglevel` can be :code:`TRACE`, :code:`DEBUG`, :code:`INFO`, :code:`WARN`, :code:`ERROR` and :code:`FATAL`.

where :code:`<path to client config file>` is the path to the directory where you created the :code:`sf_client_config file`.

The driver looks for the location of the configuration file in the following order:

#. :code:`pdo_snowflake.clientconfigfile` in php.ini
#. :code:`SF_CLIENT_CONFIG_FILE` environment variable, containing the full path to the configuration file (e.g. :code:`export SF_CLIENT_CONFIG_FILE=/some_path/some-directory/client_config.json`).
#. php directory (e.g. where :code:`php.exe` is located).
#. User’s home directory.

   .. code-block:: none
   **Note**
   To enhance security, the driver requires the logging configuration file on Unix-style systems to limit file permissions to allow only the file owner to modify the files (such as :code:`chmod 0600` or :code:`chmod 0644`).

    .. code-block:: none
   **Note**
   File must be named :code:`sf_client_config.json` for scenario 3 and 4.
