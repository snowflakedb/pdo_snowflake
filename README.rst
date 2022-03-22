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

To build the Snowflake PHP PDO Driver, the following software must be installed:

- On Windows: Visual Studio
- On Linux:

  - gcc 5.2 or higher
  - cmake 2.8 or higher

- On macOS:

  - clang
  - cmake 2.8 or higher

To install and use the Snowflake PHP PDO Driver, you must have the following software installed:

- PHP 8.1, 8.0, 7.4 or 7.3 (Note: support for PHP 7.2 is deprecated)
- the :code:`php-pdo` extension
- the :code:`php-json` extension

Note: Some of the examples in the instructions refer to the ``php-fpm`` extension. This extension is not required.
The driver also works with regular PHP CGI.

To build the driver, you must install the PHP development package for your operating system.

If you are using PHP with an application server or web server (e.g. Apache or nginx), configure the server to handle
requests for PHP pages. See the `PHP documentation <https://www.php.net/manual/en/install.php>`_ for details.

Building the PHP PDO Driver
================================================================================

The following sections explain how to build the PHP PDO Driver on Linux, macOS, and Windows.

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

   If you built PHP from the source code, run these commands from the directory containing the PHP source code.

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

.. |win-vs-version| replace:: VS16 8.0.16 

**Note**: Snowflake supports only thread-safe versions of PHP.

To build the PHP driver for Windows:

#. Download and install the PHP SDK:

    #. Download PHP 8.0.16 binaries from `<https://windows.php.net/downloads/releases/php-8.0.16-Win32-vs16-x64.zip>`_.

    #. Unzip the file to <path to PHP SDK>, such as :code:`C:\php-sdk`.

#. Clone the :code:`pdo_snowflake` repository:

   .. code-block:: batch

       git clone https://github.com/snowflakedb/pdo_snowflake.git
       cd pdo_snowflake

#. Run the script to download the PHP SDK:

   .. code-block:: batch

       .\scripts\setup_php_sdk.bat <arch> <build> <visual studio version> <path to PHP SDK>

   where:

   - :code:`<arch>` is your CPU architecture (:code:`x64` or :code:`x86`).
   - :code:`<build>` is the type of binary that you want to build (:code:`Release` or :code:`Debug`).
   - :code:`<visual studio version>` is the version of Visual Studio that you are using (:code:`VS14` or :code:`VS15`).
   - :code:`<path to PHP SDK>` is the path to the directory where the PHP SDK should be downloaded.
     **Do not create this directory.** The script creates this directory for you when downloading the PHP SDK.

   For example:

   .. parsed-literal::

       .\\scripts\\setup_php_sdk.bat x64 Release |win-vs-version| C:\\php-sdk

#. Download and install the PHP binaries, or build PHP yourself.

   If you want to build PHP yourself, run the script to download the PHP source and build PHP:

   .. code-block:: batch

       .\scripts\run_setup_php.bat <arch> <build> <visual studio version> <full PHP version> <path to PHP SDK>

   For :code:`<arch>`, :code:`<build>`, :code:`<visual studio version>`, and :code:`<path to PHP SDK>`, specify the same values
   that you used in the previous step.

   For :code:`<full PHP version>`, specify the full version number of PHP that you want to install (e.g. :code:`7.2.24`).

   For example:

   .. parsed-literal::

       .\\scripts\\run_setup_php.bat x64 Release |win-vs-version| C:\\php-sdk

#. Run the script to build the driver:

   .. code-block:: batch

       .\scripts\run_build_pdo_snowflake.bat <arch> <build> <visual studio version> <full PHP version> <path to PHP SDK>

   For example:

   .. parsed-literal::

       .\\scripts\\run_build_pdo_snowflake.bat x64 Release |win-vs-version| C:\\php-sdk

#.  Copy :code:`php_pdo_snowflake.dll` from the directory where you built the driver to the PHP extension 
    directory (the same directory that contains the :code:`php_pdo.dll file`). Usually, the PHP extension 
    directory is the :code:`ext` subdirectory in the directory where PHP is installed.

#. Run the following command to verify that the driver can be loaded into memory successfully:

   .. code-block:: batch

       C:\php\php.exe -dextension=ext\php_pdo_snowflake.dll -m

Installing the PHP PDO Driver
================================================================================

The following sections explain how to install the PHP PDO Driver on Linux, macOS, and Windows.

Installing the Driver on Linux and macOS
----------------------------------------

#. Copy :code:`pdo_snowflake.so` from the directory where you built the driver to the PHP extension directory (the same directory
   that contains the :code:`pdo.so` file).

   To find the PHP extension directory, run:

   .. code-block:: bash

       $PHP_HOME/bin/php -i | grep '^extension_dir'

#. Copy :code:`cacert.pem` from the :code:`libsnowflakeclient` subdirectory in the repository to the directory containing the
   PHP configuration files (e.g. :code:`/etc/php/7.2/fpm/conf.d` for PHP-FPM version 7.2 on Ubuntu).

#. In the same directory that contains the PHP configuration files, create a config file named :code:`20-pdo_snowflake.ini` that
   contains the following settings:

   .. code-block:: ini

       extension=pdo_snowflake.so
       pdo_snowflake.cacert=<path to PHP config directory>/cacert.pem
       # pdo_snowflake.logdir=/tmp     # location of log directory
       # pdo_snowflake.loglevel=DEBUG  # log level

   where :code:`<path to PHP config directory>` is the path to the directory where you copied the :code:`cacert.pem` file in the
   previous step.

#. If you are using PHP with an application server or web server (e.g. Apache or nginx), restart the server.


Installing the Driver on Windows
--------------------------------

#. Copy :code:`php_pdo_snowflake.dll` from the directory where you built the driver to the PHP extension directory (the same
   directory that contains the :code:`php_pdo.dll` file). Usually, the PHP extension directory is the :code:`ext` subdirectory
   in the directory where PHP is installed.

#. Copy :code:`cacert.pem` from the :code:`libsnowflakeclient` subdirectory in the repository to the directory containing the
   PHP configuration files (e.g. :code:`C:\php` if PHP is installed in that directory).

#. Add the following lines to your :code:`php.ini` file:

   .. code-block:: ini

       extension=php_pdo_snowflake.dll
       pdo_snowflake.cacert=<path to PHP config directory>\cacert.pem
       # pdo_snowflake.logdir=C:\path\to\logdir     # location of log directory
       # pdo_snowflake.loglevel=DEBUG  # log level

   where :code:`<path to PHP config directory>` is the path to the directory where you copied the :code:`cacert.pem` file in the
   previous step.

#. If you are using PHP with an application server or web server (e.g. Apache or nginx), restart the server.

Using the Driver
================================================================================

The next sections explain how to use the driver in a PHP page.

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

For accounts in regions outside of US-West, use :code:`region` parameter to specify the region or append the region to the
:code:`account` parameter.

.. code-block:: php

    $dbh = new PDO("snowflake:account=testaccount.us-east-1", "user", "password");
    $dbh = new PDO("snowflake:account=testaccount;region=us-east-1", "user", "password");

Configuring OCSP Checking
----------------------------------------------------------------------

By default, OCSP (Online Certificate Status Protocol) checking is enabled and is set per PDO connection.

To disable OCSP checking for a PDO connection, set :code:`insecure_mode=true` in the DSN connection string. For example:

.. code-block:: php

    $dbh = new PDO("snowflake:account=testaccount;insecure_mode=true", "user", "password");

Performing a Simple Query
----------------------------------------------------------------------

The following example connects to the Snowflake database and performs a simple query.
Before using this example, set the :code:`$account`, :code:`$user`, and :code:`$password` variables to your account, login name,
and password.

.. code-block:: php

  <$php
    $account = "<account_name>";
    $user = "<user_name>";
    $password = "<password>";

    $dbh = new PDO("snowflake:account=$account", $user, $password);
    $dbh->setAttribute( PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION );
    echo "Connected\n";

    $sth = $dbh->query("select 1234");
    while ($row=$sth->fetch(PDO::FETCH_NUM)) {
        echo "RESULT: " . $row[0] . "\n";
    }
    $dbh = null;
    echo "OK\n";
  $>


Running Tests For the PHP PDO Driver
================================================================================

In order to run the test scripts, you must have jq installed.

Prepare for Test
----------------------------------------------------------------------

Create a parameter file :code:`parameters.json` under :code:`pdo_snowflake` directory:

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

Call :code:`env.sh` script to set the test connection parameters in the environment variables.

.. code-block:: bash

    ./scripts/env.sh && env | grep SNOWFLAKE_TEST > testenv.ini

Proxy
^^^^^^^^^^

PHP PDO Driver for Snowflake supports HTTP and HTTPS proxy connections using environment variables. To use a proxy server configure the following environment variables:

- http_proxy
- https_proxy
- no_proxy

.. code-block:: bash

    export http_proxy="[protocol://][user:password@]machine[:port]"
    export https_proxy="[protocol://][user:password@]machine[:port]"

More info can be found on the `libcurl tutorial`__ page.

.. __: https://curl.haxx.se/libcurl/c/libcurl-tutorial.html#Proxies

Run Tests
----------------------------------------------------------------------

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
    pdo_snowflake_loglevel=DEBUG

Where is the log files?
----------------------------------------------------------------------

The location of log files are specified by the parameters in php.ini:

.. code-block:: bash

    extension=pdo_snowflake.so
    pdo_snowflake.cacert=/etc/php/7.2/fpm/conf.d/cacert.pem
    pdo_snowflake.logdir=/tmp     # location of log directory
    pdo_snowflake.loglevel=DEBUG  # log level

where :code:`pdo_snowflake.loglevel` can be :code:`TRACE`, :code:`DEBUG`, :code:`INFO`, :code:`WARN`, :code:`ERROR` and :code:`FATAL`.
