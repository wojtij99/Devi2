#include "client.hpp"
#include "../tools/sql.hpp"
#include <mysql/mysql.h>

void devi::Client(crow::SimpleApp& app)
{
    CROW_ROUTE(app, "/client/addTable")
    .methods(crow::HTTPMethod::POST)
    ([&](const crow::request& req){
        auto body = crow::json::load(req.body);
        if(!body) 
            return crow::response(crow::BAD_REQUEST, "Invalid body");

        std::string name, email, key, user, pass, db;

        try
        {
            name    = body["name"].s();
            user    = body["user"].s();
            pass    = body["pass"].s();
            db      = body["db"].s();
        }
        catch(const std::runtime_error& e)
        {
            return crow::response(crow::BAD_REQUEST, "Invalid body");
        }

        MYSQL sql;
        if(!devi::sql_start(&sql, "db_" + db, user, pass)) return crow::response(crow::SERVICE_UNAVAILABLE, std::strcat("Can't connect to DB", mysql_error(&sql)));

        MYSQL_RES* sql_response;
        MYSQL_ROW sql_row;
        //CREATE TABLE 
        if(!exec_NOquery(&sql, {"CREATE TABLE `", name ,"`(ID INT NOT NULL AUTO_INCREMENT, PRIMARY KEY(ID));"})) 
            return crow::response(crow::CONFLICT, "Can't create table");

        //GRANT FOR ADMIN USER
        if(!exec_NOquery(&sql, {"GRANT SELECT, INSERT, UPDATE, DELETE ON 'db_",db , "'.'", db, "_", name ,"' TO '", user , "'@'localhost';"})) 
            return crow::response(crow::CONFLICT, "Can't init table");

        return crow::response(crow::OK);
    });
}
