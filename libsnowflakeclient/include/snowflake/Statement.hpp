/*
 * Copyright (c) 2017-2018 Snowflake Computing, Inc. All rights reserved.
 */

#ifndef SNOWFLAKECLIENT_SNOWFLAKESTATEMENT_HPP
#define SNOWFLAKECLIENT_SNOWFLAKESTATEMENT_HPP

#include <string>
#include "client.h"
#include "Connection.hpp"
#include "Column.hpp"
#include "Param.hpp"

namespace Snowflake {
    namespace Client {
        class Statement {
        public:

            Statement(Connection &connection_);

            ~Statement(void);

            void query(const std::string &command_);

            int64 affectedRows();

            uint64 numRows();

            uint64 numFields();

            const char *sqlState();

            SF_COLUMN_DESC *desc();

            void prepare(const std::string &command_);

            void setAttribute(SF_STMT_ATTRIBUTE type_,
                                                    const void *value);

            void getAttribute(SF_STMT_ATTRIBUTE type_,
                                                    void **value);

            void execute();

            SF_STATUS fetch();

            uint64 numParams();

            Param& param(size_t i);

            // TODO add method parameters to create a SF_BIND_INPUT
            Param& createParam();

            void destroyParams();

            Column& column(size_t i);

            const char *sfqid();

            const std::string errMsg();

        private:
            // C API struct to operate on
            SF_STMT *m_stmt;
            // Pointer to the connection object that the statement struct will to
            // connect to Snowflake.
            Connection *m_connection;
            // Array of pointers to created columns
            Column **m_columns;
            // Array of pointers to created parameters
            Param **m_params;
        };
    }
}


#endif //SNOWFLAKECLIENT_SNOWFLAKESTATEMENT_HPP
