#include "admin.hpp"

void devi::Admin(crow::SimpleApp& app, MYSQL& sql)
{
    CROW_ROUTE(app, "/admin/addNewCompany")
    .methods(crow::HTTPMethod::POST)
    ([&](const crow::request& req){
        auto body = crow::json::load(req.body);
        if(!body) 
            return crow::response(crow::BAD_REQUEST, "Invalid body");

        std::string name, email, key, user, pass;

        try
        {
            name    = body["name"].s();
            email   = body["email"].s();
            key     = body["key"].s();
            user    = body["user"].s();
            pass    = body["pass"].s();
        }
        catch(const std::runtime_error& e)
        {
            return crow::response(crow::BAD_REQUEST, "Invalid body");
        }

        MYSQL_RES* sql_response;
        MYSQL_ROW sql_row;
        std::stringstream command;
        command << "SELECT id FROM users WHERE `key` = '" << key << "';";

        mysql_query(&sql, command.str().c_str());
        sql_response = mysql_store_result(&sql);

        if ((sql_row = mysql_fetch_row(sql_response)) == NULL)
            return crow::response(crow::UNAUTHORIZED, "Invalid key");

        if(email.find("@") == std::string::npos || email.substr(email.find("@")).find(".") == std::string::npos)
            return crow::response(crow::BAD_REQUEST, "Invalid email");

        command.str(std::string());
        command << "CREATE DATABASE db_" << name << " DEFAULT CHARACTER SET utf8 COLLATE utf8_unicode_ci;";

        if(mysql_query(&sql, command.str().c_str()) != 0) 
            return crow::response(crow::CONFLICT);
        
        command.str(std::string());
        command << "CREATE USER '" << name << "_" << user << "'@'localhost' IDENTIFIED BY '" << pass << "';";

        if(mysql_query(&sql, command.str().c_str()) != 0) 
            return crow::response(crow::CONFLICT);

        // ! Change DB
        command.str(std::string());
        command << "CREATE TABLE system_users(ID INT NOT NULL AUTO_INCREMENT, name TEXT NOT NULL, pass TEXT NOT NULL, PRIMARY KEY(ID))";;

        if(mysql_query(&sql, command.str().c_str()) != 0) 
            return crow::response(crow::CONFLICT);

        return crow::response(crow::OK);
    });
}