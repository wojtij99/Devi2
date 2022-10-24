#include <crow.h>
#include <mysql/mysql.h>
#include <iostream>

#define DEBUG_MODE true

#define SQL_HOST "127.0.0.1"
#define SQL_USER "root"
#define SQL_PASS ""
#define SQL_NAME "testAPI"
#define SQL_PORT 3306

#define API_PORT 3001

int main()
{
    std::cout << "Starting..." << std::endl;
    MYSQL sql;

    mysql_init(&sql);

    if(!mysql_real_connect(&sql, SQL_HOST, SQL_USER, SQL_PASS, SQL_NAME, SQL_PORT, NULL, 0))
    {
        printf("Błąd połączenia z bazą MySQL: %d, %s\n", mysql_errno(&sql), mysql_error(&sql));
        return -1;
    }

    crow::SimpleApp app;

    #if !DEBUG_MODE
        //app.loglevel(crow::LogLevel::Warning);
    #endif

    CROW_ROUTE(app, "/")
    ([&](){
        std::stringstream result;
        result << "WELCOME!" << std::endl << "Connected to DB on " << SQL_HOST << ":" << SQL_PORT;

        return  result.str();
    });

    app.port(API_PORT).multithreaded().run();
    mysql_close(&sql);
    return 0;
}