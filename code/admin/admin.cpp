#include "admin.hpp"
#include "../tools/sql.hpp"
#include "../tools/sha1.hpp"
#include "../tools/tools.hpp"
#include <mysql/mysql.h>

void devi::Admin(crow::App<crow::CORSHandler> &app)
{
    CROW_ROUTE(app, "/admin/addNewCompany")
        .methods(crow::HTTPMethod::PUT)([&](const crow::request &req)
                                        {
        auto body = crow::json::load(req.body);
        if(!body) 
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid body\"}");

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
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid body\"}");
        }

        if(key.find('\'') != std::string::npos) return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Invalid key\"}");
        if(name.find('\'') != std::string::npos) return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Invalid character (') in name\"}");
        if(email.find('\'') != std::string::npos) return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Invalid character (') in email\"}");
        if(user.find('\'') != std::string::npos) return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Invalid character (') in user\"}");

        SHA1 checksum;
        checksum.update(pass);
        std::string hashPass = checksum.final();

        MYSQL sql;
        if(!devi::sql_start(&sql)) return crow::response(crow::SERVICE_UNAVAILABLE, "{\"response\":\"Can't connect to DB\"}");

        MYSQL_RES* sql_response;
        MYSQL_ROW sql_row;
        std::stringstream command;
        command << "SELECT id FROM users WHERE `key` = '" << key << "';";

        mysql_query(&sql, command.str().c_str());
        sql_response = mysql_store_result(&sql);

        if ((sql_row = mysql_fetch_row(sql_response)) == NULL)
        {
            mysql_close(&sql);
            return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Invalid key\"}");
        }

        if(email.find("@") == std::string::npos || email.substr(email.find("@")).find(".") == std::string::npos)
        {
            mysql_close(&sql);
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid email\"}");
        }

        mysql_query(&sql, "START TRANSACTION;");

        if(!exec_NOquery(&sql, {"CREATE DATABASE `db_", name ,"` DEFAULT CHARACTER SET utf8 COLLATE utf8_unicode_ci;"}, true))
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create DB\"}");

        if(!exec_NOquery(&sql, {"INSERT INTO `companies` VALUES(NULL,'", name, "', '", email, "')"}, true))
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't add Db to register\"}");

        mysql_close(&sql);
        if(!devi::sql_start(&sql, "db_" + name)) return crow::response(crow::SERVICE_UNAVAILABLE, "{\"response\":\"Can't connect to DB\"}");

        if(!exec_NOquery(&sql, {"CREATE TABLE system_users(ID INT NOT NULL AUTO_INCREMENT, name TEXT NOT NULL, pass TEXT NOT NULL, PRIMARY KEY(ID));"}, true)) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't init DB\"}");

        if(!exec_NOquery(&sql, {"INSERT INTO system_users VALUES(NULL, '", user, "', '", hashPass, "');"}, true)) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't insert user to DB\"}");

        mysql_query(&sql, "COMMIT;");
        mysql_close(&sql);
        return crow::response(crow::OK); });
}