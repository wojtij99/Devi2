#include <crow.h>
#include <mysql/mysql.h>
#include <iostream>

#define DEBUG_MODE true

#define SQL_HOST "192.168.1.49"
#define SQL_USER "API_MySQLuser"
#define SQL_PASS "jSmmwERr4i#G^O2vhP3J*pBayW%eFlZ7"
#define SQL_NAME "mainDB"
#define SQL_PORT 3306

#define API_PORT 3001

int main()
{
    MYSQL sql;

    mysql_init(&sql);

    if(!mysql_real_connect(&sql, SQL_HOST, SQL_USER, SQL_PASS, SQL_NAME, SQL_PORT, NULL, 0))
    {
        printf("Błąd połączenia z bazą MySQL: %d, %s\n", mysql_errno(&sql), mysql_error(&sql));
        return -1;
    }

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

    CROW_ROUTE(app, "/admin/addNewCompany")
    .methods(crow::HTTPMethod::POST)
    ([&](const crow::request& req){
        auto body = crow::json::load(req.body);
        if(!body) 
            return crow::response(crow::BAD_REQUEST, "Invalid body");

        std::string name, email, key;

        try
        {
            name    = body['name'].s();
            email   = body['email'].s();
            key     = body['key'].s();
        }
        catch(const std::runtime_error& e)
        {
            return crow::response(crow::BAD_REQUEST, "Invalid body");
        }

        MYSQL_RES* sql_response;
        MYSQL_ROW sql_row;
        std::stringstream command;
        command << "SELECT id FROM users WHERE key = '" << key << "';";

        mysql_query(&sql, command.str().c_str());
        sql_response = mysql_store_result(&sql);

        if ((sql_row = mysql_fetch_row(sql_response)) == NULL)
            return crow::response(crow::UNAUTHORIZED, "Invalid key");

        if(email.find("@") == std::string::npos || email.substr(email.find("@")).find("."))
            return crow::response(crow::BAD_REQUEST, "Invalid email");
        
        return crow::response(crow::OK);
        
    });
    
    app.port(API_PORT).multithreaded().run();
    mysql_close(&sql);
    return 0;
}