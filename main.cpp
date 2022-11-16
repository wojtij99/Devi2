#include <crow.h>
#include <crow/middlewares/cors.h>
#include <mysql/mysql.h>
#include <iostream>

#include "admin/admin.hpp"
#include "client/client.hpp"
#include "tables/tables.hpp"
#include "tools/sql.hpp"
#include "settings.h"
#include "tools/sin.hpp"

#include <boost/format.hpp>

int main()
{
    MYSQL sql;

    if(!devi::sql_start(&sql)) return -1;
        mysql_close(&sql);

    crow::App<crow::CORSHandler> app;

    #if !DEBUG_MODE
        app.loglevel(crow::LogLevel::Warning);
    #endif

    auto& cors = app.get_middleware<crow::CORSHandler>();

    cors
      .global()
        .headers("access-control-allow-origin", "*")
        .methods("POST"_method, "GET"_method);

    CROW_ROUTE(app, "/")
    ([&](){
        std::stringstream result;
        result << "WELCOME!" << std::endl << "Connected to DB on " << SQL_HOST << ":" << SQL_PORT << std::endl <<
        "+----------------------------+--------+------------+-------------------+" << std::endl <<
        "|           Route            | Method |   Parans   |    Description    | " << std::endl <<
        "+----------------------------+--------+------------+-------------------+ " << std::endl <<
        "| /                          | GET    |            | return this page  | " << std::endl <<
        "| /tables                    | GET    |            | return all tables | " << std::endl <<
        "| /tables/add                | POST   |            | add new Table     | " << std::endl <<
        "| /tables/<string>/addColumn | POST   | table name | add new Column    | " << std::endl <<
        "| /getSIN                    | GET    |            | return your SIN   | " << std::endl <<
        "+----------------------------+--------+------------+-------------------+ " << std::endl;

        return  result.str();
    });

    devi::Admin(app);
    devi::Client(app);
    devi::SIN(app);
    devi::Tables(app);
    
    app.port(API_PORT).multithreaded().run();
    return 0;
}