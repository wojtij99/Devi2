#include <crow.h>
#include <mysql/mysql.h>
#include <iostream>

#include "admin/admin.hpp"
#include "tools/sql.hpp"
#include "settings.h"

int main()
{
    MYSQL sql;

    if(!devi::sql_start(&sql)) return -1;

    crow::SimpleApp app;

    #if !DEBUG_MODE
        app.loglevel(crow::LogLevel::Warning);
    #endif

    CROW_ROUTE(app, "/")
    ([&](){
        std::stringstream result;
        result << "WELCOME!" << std::endl << "Connected to DB on " << SQL_HOST << ":" << SQL_PORT;

        return  result.str();
    });

    devi::Admin(app, sql);
    
    app.port(API_PORT).multithreaded().run();
    mysql_close(&sql);
    return 0;
}