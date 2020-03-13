********************************************************************************
PHP PDO driver for Snowflake
********************************************************************************

.. image:: https://travis-ci.org/snowflakedb/pdo_snowflake.svg?branch=master
    :target: https://travis-ci.org/snowflakedb/pdo_snowflake

.. image:: https://codecov.io/gh/snowflakedb/pdo_snowflake/branch/master/graph/badge.svg
    :target: https://codecov.io/gh/snowflakedb/pdo_snowflake

.. image:: http://img.shields.io/:license-Apache%202-brightgreen.svg
    :target: http://www.apache.org/licenses/LICENSE-2.0.txt

*Private Preview. Linux Only. No PHP 5 support. PHP 7.2 only.*

Configuring Environment
================================================================================

PHP Versions and Extensions
----------------------------------------------------------------------

PHP 7.2 is supported. The following extensions are required:

    * pdo
    * json
    
If you're not building PHP from source, you will also need to install the corresponding PHP development package for your OS/environment

Application Server (Optional)
----------------------------------------------------------------------

If the PHP is used along with an application server, e.g., nginx, apache, which is the common use case, install them and configure the PHP handler accordingly so that the PHP file can run on the web.
For example, Nginx enables the PHP handler by adding the following config in :code:`/etc/nginx/sites-available/default`:

.. code-block:: text

        server {
            ... the standard settings ...

            # PHP handler
            location ~ \.php$ {
                fastcgi_split_path_info ^(.+\.php)(/.+)$;
                fastcgi_pass unix:/var/run/php/php7.2-fpm.sock;
                fastcgi_index index.php;
                include fastcgi_params;
            }
        }

where PHP 7.2 and the corresponding PHP-FPM (https://php-fpm.org/) package are used, for example.

Restart Nginx and PHP-FPM services.

.. code-block:: bash

    service nginx restart
    service php7.2-fpm restart

Add a PHP test code :code:`test.php` to the home location of an application server.

.. code-block:: php

    <?php phpinfo(); ?>


Ensure it can run and the output includes both :code:`pdo` and :code:`json` extensions.

.. code-block:: bash

    curl http://localhost/test.php | grep -E "(pdo|json)"

Installing PDO driver for Snowflake
================================================================================
Separate instructions exist for Windows, Linux, macOS

Linux
-----

There are two required files for you to copy:

- pdo_snowflake.so
- cacert.pem

Copy :code:`pdo_snowflake.so` to the same location as `pdo.so` where all PHP extentions reside.

Copy :code:`cacert.pem` to the PHP config directory. For example, PHP-FPM version 7.2 on Ubuntu12 has :code:`/etc/php/7.2/fpm/conf.d/` for the extensions.

.. note::

    If you don't have :code:`pdo_snowflake.so`, build it following the instruction below.

.. code-block:: bash

    cp cacert.pem /etc/php/7.2/fpm/conf.d/

Add a config file :code:`/etc/php/7.2/fpm/conf.d/20-pdo_snowflake.ini` including the following contents to the PHP config directory.

.. code-block:: text

    extension=pdo_snowflake.so
    pdo_snowflake.cacert=/etc/php/7.2/fpm/conf.d/cacert.pem
    # pdo_snowflake.logdir=/tmp     # location of log directory
    # pdo_snowflake.loglevel=DEBUG  # log level

Restart Nginx and PHP-FPM services. For example:

.. code-block:: bash

    service nginx restart
    service php7.2-fpm restart

Ensure :code:`phpinfo()` function return the output including :code:`pdo_snowflake`.

.. code-block:: bash

    curl http://localhost/test.php | grep -E "(pdo|json|snowflake)"

.. note::

    We have not finalized what package would be the best for binary distribution. So far I'm trying to get :code:`pecl` account but have not got one yet. Any suggestion is welcome.

Windows
-------

There are two required files for you to copy:

- php_pdo_snowflake.dll
- cacert.pem

Copy :code:`php_pdo_snowflake.dll` to the same location as :code:`php_pdo.dll` where all PHP extensions reside (usually the :code:`ext` folder in your PHP installation).

Copy :code:`cacert.pem` to the PHP config directory. For example, PHP version 7.2 installed at :code:`C:\` on Windows 10 has :code:`C:\php\php.ini` for the extensions.

.. note::

    If you don't have :code:`php_pdo_snowflake.dll`, build it following the instruction below.

Add the following lines to your :code:`php.ini` file:

.. code-block:: text

    extension=php_pdo_snowflake.dll
    pdo_snowflake.cacert=C:\php\cacert.pem
    # pdo_snowflake.logdir=C:\path\to\logdir     # location of log directory
    # pdo_snowflake.loglevel=DEBUG  # log level

Restart your PHP server and then you should see :code:`pdo_snowflake` as a PHP extension

Usage
================================================================================

Limitations
-----------
- Timestamp support on Windows is limited to values between the dates 1/1/1970 and 1/1/2038. Trying to fetch values outside of this range will result in an empty value being returned
- Named placeholders (placeholders in SQL queries of the form :code:`first_name:`) are not supported at this time. Positional placeholders (placeholders in SQL queries of the form :code:`?`) are supported.

Connection String
----------------------------------------------------------------------

Create a database handle with connection parameters:

.. code-block:: php

    $dbh = new PDO("snowflake:account=testaccount", "user", "password");

For non-US-West region, specify :code:`region` parameter or append it to :code:`account` parameter.

.. code-block:: php

    $dbh = new PDO("snowflake:account=testaccount.us-east-1", "user", "password");
    $dbh = new PDO("snowflake:account=testaccount;region=us-east-1", "user", "password");

OCSP Checking
----------------------------------------------------------------------

OCSP (Online Certificate Status Protocol) checking is set per PDO connection and enabled by default. To disable OCSP checking, set :code:`insecure_mode=true` in the DSN connection string. Example shown here:

.. code-block:: php

    $dbh = new PDO("snowflake:account=testaccount;insecure_mode=true", "user", "password");

Query
----------------------------------------------------------------------

Here is an example of fetch a row:

.. code-block:: php

    $account = "<account_name>";
    $user = "<user_name>";
    $password = "<password";

    $dbh = new PDO("snowflake:account=$account", $user, $password);
    $dbh->setAttribute( PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION );
    echo "Connected\n";

    $sth = $dbh->query("select 1234");
    while ($row=$sth->fetch(PDO::FETCH_NUM)) {
        echo "RESULT: " . $row[0] . "\n";
    }
    $dbh = null;
    echo "OK\n";

Build and Test
================================================================================

Build and Install PHP from source on Linux (Optional)
----------------------------------------------------------------------

If PHP is not available, download and build from the PHP source code.

.. code-block:: bash

    # Go to http://php.net/releases/
    # Download php source code and copy to $WORKSPACE, for example
    cd $WORKSPACE

Set PHP version to the environment variable. For example, set :code:`SF_PHP_VERSION` to :code:`7.2.24`
if the downloaded PHP version is 7.2.24.

.. code-block:: bash

    export SF_PHP_VERSION=7.2.24

Extract and build PHP:

.. code-block:: bash

    cd $WORKSPACE
    rm -rf $WORKSPACE/php-$SF_PHP_VERSION-src
    rm -rf $WORKSPACE/install-php-$SF_PHP_VERSION
    tar xvfj php-$SF_PHP_VERSION.tar.bz2
    cd php-$SF_PHP_VERSION
    ./configure \
        --prefix=$WORKSPACE/install-php-$SF_PHP_VERSION \
    make
    make install

Build PDO Driver on Linux
----------------------------------------------------------------------

Set :code:`PHP_HOME` to the base directory of the PHP. For example, if you built PHP, do this:

.. code-block:: bash

    export PHP_HOME=$WORKSPACE/install-php-$SF_PHP_VERSION

or do this if the PHP is already installed in the system.

.. code-block:: bash

    export PHP_HOME=/usr

where :code:`$PHP_HOME/bin` is referred to run :code:`phpize`:

Clone the this repository and run the build script.

.. code-block:: bash

    git clone https://github.com/snowflakedb/pdo_snowflake.git
    cd pdo_snowflake
    ./scripts/build_pdo_snowflake.sh

Run the following command to check if PHP PDO Driver for Snowflake is successfully loaded in memory.

.. code-block:: bash

    $PHP_HOME/bin/php -dextension=modules/pdo_snowflake.so -m | grep pdo_snowflake

.. note::

    As the build requires a special link process, a simple sequence of :code:`phpize` followed by :code:`make` doesn't work. See the build script for the detail.

Build and Install PHP on Windows (Optional)
----------------------------------------------------------------------

A set of scripts has been created in this repo to facilitate setting up PHP on Windows:

- setup_php_sdk.bat <arch[x64,x86]> <build[Release,Debug]> <visual studio version[VS14,VS15]> <path to PHP SDK>
- run_setup_php.bat <arch[x64,x86]> <build[Release,Debug]> <visual studio version[VS14,VS15]> <full PHP version> <path to PHP SDK>

First, we are going to setup the PHP SDK tools:

.. code-block:: batch

    -- Clone and go to top level of repository
    git clone https://github.com/snowflakedb/pdo_snowflake.git
    cd pdo_snowflake
    .\scripts\setup_php_sdk.bat x64 Release VS15 C:\php-sdk

Now we are going to download (including dependencies) and build PHP:

.. code-block:: batch

    .\scripts\run_setup_php.bat x64 Release VS15 7.2.24 C:\php-sdk

Build PDO Driver on Windows
----------------------------------------------------------------------

Run the following command in the top level of this repo to build the PDO driver on Windows:

- run_build_pdo_snowflake.bat <arch[x64,x86]> <build[Release,Debug]> <visual studio version[VS14,VS15]> <full PHP version> <path to PHP SDK>

Example:

.. code-block:: batch

  run_build_pdo_snowflake.bat x64 Release VS15 7.2.24 C:\php-sdk

Run the following command to check if PHP PDO Driver for Snowflake is successfully loaded in memory.

.. code-block:: bash

    C:\php\php.exe -dextension=ext\php_pdo_snowflake.dll -m


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


Trouble Shootings
================================================================================

Cannot load module 'pdo_snowflake' because required module 'pdo' is not loaded
----------------------------------------------------------------------

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
