********************************************************************************
Build Docker images for tests
********************************************************************************

Install Docker and run the script to build the docker images for tests.

.. code-block:: bash

    ./build.sh

Run Squid proxy for the tests. Assumptions - Proxy has been built already.

.. code-block:: bash
    ./run.sh

Push the image to Docker hub or registory if required.

.. code-block:: bash

    docker login -u <your_user_name>
    docker tag pdo-snowflake-w-proxy:php7.2-ubuntu14.04 snowflakedb/pdo-snowflake-w-proxy:php7.2-ubuntu14.04
    docker push snowflakedb/pdo-snowflake-w-proxy:php7.2-ubuntu14.04
    docker tag pdo-snowflake-w-proxy:php7.2-ubuntu14.04 snowflakedb/pdo-snowflake-w-proxy:php7.2-ubuntu14.04
    docker push snowflakedb/pdo-snowflake-w-proxy:php7.2-ubuntu14.04
    docker tag pdo-snowflake-w-proxy:php7.2-ubuntu16.04 snowflakedb/pdo-snowflake-w-proxy:php7.2-ubuntu16.04
    docker push snowflakedb/pdo-snowflake-w-proxy:php7.2-ubuntu16.04
    docker tag pdo-snowflake-w-proxy:php7.2-ubuntu18.04 snowflakedb/pdo-snowflake-w-proxy:php7.2-ubuntu18.04
    docker push snowflakedb/pdo-snowflake-w-proxy:php7.2-ubuntu18.04
