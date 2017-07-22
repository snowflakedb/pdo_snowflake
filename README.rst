********************************************************************************
PHP PDO driver for Snowflake
********************************************************************************

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

Clone the this repository and run ``phpize`` followed by ``configure``

.. code-block:: bash

    git clone git@github.com:snowflakedb/pdo_snowflake.git
    cd pdo_snowflake
    $WORKSPACE/install-php-$SF_PHP_VERSION/bin/phpize
    ./configure \
        --with-php-config=$WORKSPACE/install-php-$SF_PHP_VERSION/bin/php-config
        --with-pdo-odbc=unixODBC,/usr
