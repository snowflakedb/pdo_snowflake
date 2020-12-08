/*
 * Copyright (c) 2018-2019 Snowflake Computing, Inc. All rights reserved.
 */

#ifndef SNOWFLAKECLIENT_SNOWFLAKECONNECTION_HPP
#define SNOWFLAKECLIENT_SNOWFLAKECONNECTION_HPP

#include <string>
#include "client.h"

namespace Snowflake {
    namespace Client {
        class Connection {
            friend class Statement;
        public:

            /* Construct a blank Snowflake Connection */
            Connection(void);

            ~Connection(void);

            void connect();

            void setAttribute(SF_ATTRIBUTE type_,
                                                    const void *value_);

            void getAttribute(SF_ATTRIBUTE type_,
                                                    void **value_);

            void beginTransaction();

            void commitTransaction();

            void rollbackTransaction();

            //TODO Instead of returning error struct, translate error codes into exceptions

            /*
             * Get error message from error struct. Error message is set when there is an exception
             */
            const std::string err_msg();

        private:
            SF_CONNECT *m_connection;
        };
    }
}

#endif //SNOWFLAKECLIENT_SNOWFLAKECONNECTION_HPP
