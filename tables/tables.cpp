#include "tables.hpp"
#include <mysql/mysql.h>
#include "../tools/sql.hpp"
#include "../tools/sin.hpp"

void devi::Tables(crow::SimpleApp& app)
{
    CROW_ROUTE(app, "/tables")
    .methods(crow::HTTPMethod::GET)
    ([&](const crow::request& req){
        auto body = crow::json::load(req.body);
        if(!body) 
            return crow::response(crow::BAD_REQUEST, "Invalid body");

        std::string sin;

        try
        {
            sin     = body["sin"].s();
        }
        catch(const std::runtime_error& e)
        {
            return crow::response(crow::BAD_REQUEST, "Invalid body");
        }

        if(checkSIN(sin, req))
            return crow::response(crow::UNAUTHORIZED, "Wrong SIN");

        MYSQL sql;
        if(!devi::sql_start(&sql, "db_" + SINs[sin].db)) return crow::response(crow::SERVICE_UNAVAILABLE, "Can't connect to DB");

        MYSQL_RES* sql_response;
        MYSQL_ROW sql_row;
        std::string response;

        mysql_query(&sql, "SHOW TABLES;");
        sql_response = mysql_store_result(&sql);

        while ((sql_row = mysql_fetch_row(sql_response)) != NULL)
        {
            response += sql_row[0];
        }

        mysql_close(&sql);
        return crow::response(crow::OK);
    });

    CROW_ROUTE(app, "/tables/add")
    .methods(crow::HTTPMethod::POST)
    ([&](const crow::request& req){
        auto body = crow::json::load(req.body);
        if(!body) 
            return crow::response(crow::BAD_REQUEST, "Invalid body");

        std::string name, sin;

        try
        {
            name    = body["name"].s();
            sin     = body["sin"].s();
        }
        catch(const std::runtime_error& e)
        {
            return crow::response(crow::BAD_REQUEST, "Invalid body");
        }

        if(!checkSIN(sin, req))
            return crow::response(crow::UNAUTHORIZED, "Wrong SIN");

        MYSQL sql;
        if(!devi::sql_start(&sql, "db_" + SINs[sin].db)) return crow::response(crow::SERVICE_UNAVAILABLE, "Can't connect to DB");

        MYSQL_RES* sql_response;
        MYSQL_ROW sql_row;
        //CREATE TABLE 
        if(!exec_NOquery(&sql, {"CREATE TABLE `", name ,"`(ID INT NOT NULL AUTO_INCREMENT, PRIMARY KEY(ID));"})) 
            return crow::response(crow::CONFLICT, "Can't create table");

        mysql_close(&sql);
        return crow::response(crow::OK);
    });

    CROW_ROUTE(app, "/tables/<string>/addColumn")
    .methods(crow::HTTPMethod::POST)
    ([&](const crow::request& req, std::string table){
        auto body = crow::json::load(req.body);
        if(!body) 
            return crow::response(crow::BAD_REQUEST, "Invalid body");

        std::string name, type, sin;

        try
        {
            name    = body["name"].s();
            type    = body["type"].s();
            sin     = body["sin"].s();
        }
        catch(const std::runtime_error& e)
        {
            return crow::response(crow::BAD_REQUEST, "Invalid body");
        }

        if(!checkSIN(sin, req))
            return crow::response(crow::UNAUTHORIZED, "Wrong SIN");

        std::vector<std::string> types = {"INT", "TEXT", "DATETIME", "DATE", "FLOAT", "BOOL"};
        boost::to_upper(type);
        
        bool inCorrectType = true;
        for(std::string t : types)
            if(t == type)
            {
                inCorrectType = false;
                continue;
            }

        if(inCorrectType) 
            return crow::response(crow::BAD_REQUEST, "Incorrect type");

        MYSQL sql;
        if(!devi::sql_start(&sql, "db_" + SINs[sin].db)) return crow::response(crow::SERVICE_UNAVAILABLE, "Can't connect to DB");
        MYSQL_RES* sql_response;
        MYSQL_ROW sql_row;
        //CREATE TABLE 
        if(!exec_NOquery(&sql, {"ALTER TABLE `", table ,"` ADD `" , name ,"` ", type , ";"}, false)) 
            return crow::response(crow::CONFLICT, "Can't create table");

        mysql_close(&sql);
        return crow::response(crow::OK);
    });
}
