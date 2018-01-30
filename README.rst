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

Basic Usage
----------------------------------------------------------------------

Download the PHP source code to build PHP PDO driver for Snowflake.

.. code-block:: bash

    # Go to http://php.net/releases/
    # Download php source code and copy to $WORKSPACE, for example
    cd $WORKSPACE

Set PHP version to the environment variable. For example, set ``SF_PHP_VERSION`` to ``7.1.6``
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
    export PHP_HOME=$WORKSPACE/install-php-$SF_PHP_VERSION
    ./scripts/build_pdo_snowflake.sh
    REPORT_EXIT_STATUS=1 NO_INTERACTION=true make test

Profile
----------------------------------------------------------------------

You can use ``callgrind`` to profile PHP PDO programs. For example, run ``tests/selectnum.phpt`` testcase using ``valgrind`` along with ``callgrind`` option.

.. code-block:: bash

    valgrind --tool=callgrind $PHP_HOME/bin/php -dextension=modules/pdo_snowflake.so tests/selectnum.phpt
    callgrind_annotate callgrind.out.*

Check memory leak by valgrind
----------------------------------------------------------------------

Use ``valgrind`` to check memeory leak. Both C API and PHP PDO can run along with ``valgrind``. For example, run ``tests/selectnum.phpt`` testcase using ``valgrind`` by the following command.

.. code-block:: bash

    valgrind --leak-check=full $PHP_HOME/bin/php -dextension=modules/pdo_snowflake.so tests/selectnum.phpt

and verify no error in the output:

.. code-block:: bash

     ERROR SUMMARY: 0 errors from 0 contexts ...

Test Framework
----------------------------------------------------------------------

The PHP PDO Snowflake driver uses phpt test framework. Refer the following documents to write tests.

- https://qa.php.net/write-test.php
- https://qa.php.net/phpt_details.php

Check if the PDO Snowflake module can be loaded
----------------------------------------------------------------------

Run the following command to check if PHP PDO Driver for Snowflake is successfully loaded in memory.

.. code-block:: bash

    $PHP_HOME/bin/php -dextension=modules/pdo_snowflake.so -m | grep pdo_snowflake

Trouble Shootings
================================================================================

Cannot load module 'pdo_snowflake' because required module 'pdo' is not loaded
----------------------------------------------------------------------

In some environments, e.g., Ubuntu 16, when you run ``make test``, the following error message shows up and no test won't run.

.. code-block:: bash

    PHP Warning:  Cannot load module 'pdo_snowflake' because required module 'pdo' is not loaded in Unknown on line 0

Ensure the php has PDO:

.. code-block:: bash

    $ php -i | grep -i "pdo support"
    PDO support => enabled

If not installed, install the package. WIP

Locate ``pdo.so`` under ``/usr/lib`` and specify it in ``phpt`` files, e.g.,

.. code-block:: bash

    --INI--
    extension=/usr/lib/php/20151012/pdo.so
    pdo_snowflake.cacert=libsnowflakeclient/cacert.pem
