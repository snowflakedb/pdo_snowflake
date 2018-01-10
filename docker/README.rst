********************************************************************************
Docker images
********************************************************************************

This directory includes docker files to for Docker images.

Build Docker Images
======================================================================

Install Docker and run the script to build all docker images.

.. code-block:: bash

    ./build.sh

Testing Code
======================================================================

Before running a container
----------------------------------------------------------------------

Set the Snowflake connection info in ``parameters.json`` and place it in $HOME:

.. code-block:: json

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

Start a docker container
----------------------------------------------------------------------

Mount $HOME to ``/cfg`` and start a container:

.. code-block:: bash

    docker run -v $HOME:/cfg -it pdo-snowflake-ubuntu12:latest

Set up
----------------------------------------------------------------------

Checkout and build code. Update the test parameters

.. code-block:: bash

    eval $(jq -r '.testconnection | to_entries | map("export \(.key)=\(.value|tostring)")|.[]' /cfg/parameters.json)
    git clone https://github.com/snowflakedb/pdo_snowflake.git
    cd pdo_snowflake
    ./scripts/build_pdo_snowflake.sh -r
    PHP_EXT=$(find /usr/lib/php -name "pdo.so") && for f in $(ls tests/\*.phpt); do sed -i "/--INI--/a extension=$PHP_EXT" $f; done
    PHP_EXT=$(find /usr/lib/php -name "json.so") && for f in $(ls tests/\*.phpt); do sed -i "/--INI--/a extension=$PHP_EXT" $f; done

Test
----------------------------------------------------------------------

Run test

.. code-block:: bash

    REPORT_EXIT_STATUS=1 NO_INTERACTION=true make test
