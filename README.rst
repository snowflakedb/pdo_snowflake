********************************************************************************
PHP PDO driver for Snowflake
********************************************************************************

.. image:: https://travis-ci.org/snowflakedb/pdo_snowflake.svg?branch=master
    :target: https://travis-ci.org/snowflakedb/pdo_snowflake

.. image:: https://codecov.io/gh/snowflakedb/pdo_snowflake/branch/master/graph/badge.svg
    :target: https://codecov.io/gh/snowflakedb/pdo_snowflake

.. image:: http://img.shields.io/:license-Apache%202-brightgreen.svg
    :target: http://www.apache.org/licenses/LICENSE-2.0.txt

*Under development. No functionality works. Suggestion is welcome at any time.*

Usage
================================================================================

Connection String
----------------------------------------------------------------------

Create a database handle with connection parameters:

.. code-block:: php

    try {
        $dbh = new PDO("snowflake:account=testaccount", "user", "password");
    } catch (PDOException $e) {
        echo 'Connection failed: ' . $e->getMessage();
    }

Build and Tests
================================================================================

Download the PHP source code to build PHP PDO driver for Snowflake.

.. code-block:: bash

    # Go to http://php.net/releases/
    # Download php source code and copy to $WORKSPACE, for example
    cd $WORKSPACE

Set PHP version to the environment variable. For example, set ``SF_PHP_VERSION=`` to ``7.1.6``
if the downloaded PHP version is 7.1.6.

.. code-block:: bash

    export SF_PHP_VERSION=7.1.6

Extract and build PHP

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

Clone the this repository and run ``phpize``, ``configure``, ``make`` and ``make test``:

.. code-block:: bash

    git clone git@github.com:snowflakedb/pdo_snowflake.git
    cd pdo_snowflake
    $WORKSPACE/install-php-$SF_PHP_VERSION/bin/phpize
    ./configure \
        --with-php-config=$WORKSPACE/install-php-$SF_PHP_VERSION/bin/php-config
        --enable-pdo_snowflake
    make
    # Workaround for libtool issue, which cannot keep the order of Link options
    # and --whole-archive is ignored.
    cc -shared \
        .libs/pdo_snowflake.o \
        .libs/snowflake_driver.o \
        .libs/snowflake_stmt.o \
        -L libsnowflakeclient/cmake-build \
        -L libsnowflakeclient/build/linux/openssl/lib \
        -L libsnowflakeclient/build/linux/curl/lib \
        -Wl,--whole-archive \
        -lsnowflakeclient -lcrypto -lssl -lcurl \
        -Wl,--no-whole-archive \
        -Wl,-soname -Wl,pdo_snowflake.so \
        -o .libs/pdo_snowflake.so
    (cd .libs && rm -f pdo_snowflake.la && ln -s ../pdo_snowflake.la pdo_snowflake.la)
    ./libtool --mode=install cp ./pdo_snowflake.la $(pwd)/modules
    make test


Check if the PDO Snowflake module can be loaded
--------------------

Run the following command to check if PHP PDO Driver for Snowflake is successfully loaded in memory.

.. code-block:: bash

    php -dextension=modules/pdo_snowflake.so -m | grep pdo_snowflake
