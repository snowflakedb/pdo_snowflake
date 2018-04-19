********************************************************************************
PHP PDO driver for Snowflake
********************************************************************************

.. image:: https://travis-ci.org/snowflakedb/pdo_snowflake.svg?branch=master
    :target: https://travis-ci.org/snowflakedb/pdo_snowflake

.. image:: https://codecov.io/gh/snowflakedb/pdo_snowflake/branch/master/graph/badge.svg
    :target: https://codecov.io/gh/snowflakedb/pdo_snowflake

.. image:: http://img.shields.io/:license-Apache%202-brightgreen.svg
    :target: http://www.apache.org/licenses/LICENSE-2.0.txt

*Private Preview. Linux Only. No PHP 5 support. PHP 7+ only.*

Usage
================================================================================

Connection String
----------------------------------------------------------------------

Create a database handle with connection parameters:

.. code-block:: php

    $dbh = new PDO("snowflake:account=testaccount", "user", "password");

For non-US-West region, specify :code:`region` parameter or append it to :code:`account` parameter.

.. code-block:: php

    $dbh = new PDO("snowflake:account=testaccount.us-east-1", "user", "password");
    $dbh = new PDO("snowflake:account=testaccount;region=us-east-1", "user", "password");

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

Environments
================================================================================

PHP Versions and Extensions
----------------------------------------------------------------------

PHP 7.0+ is supported. The following extensions are required:

    * pdo
    * json

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
                fastcgi_pass unix:/var/run/php/php7.1-fpm.sock;
                fastcgi_index index.php;
                include fastcgi_params;
            }
        }

where PHP 7.1 and the corresponding PHP-FPM (https://php-fpm.org/) package are used, for example.

Restart Nginx and PHP-FPM services.

.. code-block:: bash

    service nginx restart
    service php7.1-fpm restart

Add a PHP test code :code:`test.php` to the home location of an application server.

.. code-block:: php

    <?php phpinfo(); ?>


Ensure it can run and the output includes both :code:`pdo` and :code:`json` extensions.

.. code-block:: bash

    curl http://localhost/test.php | grep -E "(pdo|json)"

Installing PDO driver for Snowflake
================================================================================

Two files require to copy.

- pdo_snowflake.so
- cacert.pem

Copy :code:`pdo_snowflake.so` to the same location as `pdo.so` where all PHP extentions reside.

Copy :code:`cacert.pem` to the PHP config directory. For example, PHP-FPM version 7.1 on Ubuntu12 has :code:`/etc/php/7.1/fpm/conf.d/` for the extensions.

.. code-block:: bash

    cp cacert.pem /etc/php/7.1/fpm/conf.d/

Add a config file :code:`/etc/php/7.1/fpm/conf.d/20-pdo_snowflake.ini` including the following contents to the PHP config directory.

.. code-block:: text

    extension=pdo_snowflake.so
    pdo_snowflake.cacert=/etc/php/7.1/fpm/conf.d/cacert.pem
    # pdo_snowflake.logdir=/tmp     # location of log directory
    # pdo_snowflake.loglevel=DEBUG  # log level

Restart Nginx and PHP-FPM services. For example:

.. code-block:: bash

    service nginx restart
    service php7.1-fpm restart

Ensure :code:`phpinfo()` function return the output including :code:`pdo_snowflake`.

.. code-block:: bash

    curl http://localhost/test.php | grep -E "(pdo|json|snowflake)"

.. note::

    We have not finalized what package would be the best for binary distribution. So far I'm trying to get :code:`pecl` account but have not got one yet. Any suggestion is welcome.


Build and Tests
================================================================================

Build and Install PHP (Optional)
----------------------------------------------------------------------

If PHP is not available, download and build from the PHP source code.

.. code-block:: bash

    # Go to http://php.net/releases/
    # Download php source code and copy to $WORKSPACE, for example
    cd $WORKSPACE

Set PHP version to the environment variable. For example, set :code:`SF_PHP_VERSION` to :code:`7.1.6`
if the downloaded PHP version is 7.1.6.

.. code-block:: bash

    export SF_PHP_VERSION=7.1.6

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

Build
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

    git clone git@github.com:snowflakedb/pdo_snowflake.git
    cd pdo_snowflake
    ./scripts/build_pdo_snowflake.sh

Run the following command to check if PHP PDO Driver for Snowflake is successfully loaded in memory.

.. code-block:: bash

    $PHP_HOME/bin/php -dextension=modules/pdo_snowflake.so -m | grep pdo_snowflake

.. note::

    As the build requires a special link process, a simple sequence of :code:`phpize` followed by :code:`make` doesn't work. See the build script for the detail.

Test
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

Call :code:`env.sh` script to set the test connection parametes in the environment variables.

.. code-block:: bash

    source ./scripts/env.sh

And run the test:

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
    extension=/usr/lib/php/20151012/pdo.so
    pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
    pdo_snowflake.logdir=/tmp
    pdo_snowflake_loglevel=DEBUG

Where is the log files?
----------------------------------------------------------------------

The location of log files are specified by the parameters in php.ini:

.. code-block:: bash

    extension=pdo_snowflake.so
    pdo_snowflake.cacert=/etc/php/7.1/fpm/conf.d/cacert.pem
    pdo_snowflake.logdir=/tmp     # location of log directory
    pdo_snowflake.loglevel=DEBUG  # log level

where :code:`pdo_snowflake.loglevel` can be :code:`TRACE`, :code:`DEBUG`, :code:`INFO`, :code:`WARN`, :code:`ERROR` and :code:`FATAL`.
