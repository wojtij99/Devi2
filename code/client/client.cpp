#include "client.hpp"
#include "../tools/tools.hpp"
#include "../tools/sin.hpp"
#include "../tools/sql.hpp"
#include "../tools/sha1.hpp"

void devi::Client(crow::App<crow::CORSHandler>& app)
{
    CROW_ROUTE(app, "/client/changeName")
    .methods(crow::HTTPMethod::POST)
    ([&](const crow::request& req){
        auto body = crow::json::load(req.body);
        if(!body) 
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid body\"}");

        std::string name, sin;

        try
        {
            name    = parseStr(body["name"].s());
            sin     = parseStr(body["sin"].s());
        }
        catch(const std::runtime_error& e)
        {
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid body\"}");
        }

        if(name.find('\'') != std::string::npos) return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Invalid character (') in name\"}");

        if(!checkSIN(sin, req))
            return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Wrong SIN\"}");

        MYSQL sql;
        if(!devi::sql_start(&sql, "db_" + SINs[sin].db)) return crow::response(crow::SERVICE_UNAVAILABLE, "{\"response\":\"Can't connect to DB\"}");

        MYSQL_RES* sql_response;
        MYSQL_ROW sql_row;

        if(!exec_NOquery(&sql, {"UPDATE `system_users` SET `name` = '", name,"' WHERE `name` = '", SINs[sin].user,"';"}))
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't chane name\"}");

        SINs[sin].expiredate = SINs[sin].creationDT - 1;

        mysql_close(&sql);
        return crow::response(crow::OK);
    });

    CROW_ROUTE(app, "/client/changePass")
    .methods(crow::HTTPMethod::POST)
    ([&](const crow::request& req){
        auto body = crow::json::load(req.body);
        if(!body) 
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid body\"}");

        std::string pass, sin;

        try
        {
            pass    = parseStr(body["pass"].s());
            sin     = parseStr(body["sin"].s());
        }
        catch(const std::runtime_error& e)
        {
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid body\"}");
        }

        if(!checkSIN(sin, req))
            return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Wrong SIN\"}");

        MYSQL sql;
        if(!devi::sql_start(&sql, "db_" + SINs[sin].db)) return crow::response(crow::SERVICE_UNAVAILABLE, "{\"response\":\"Can't connect to DB\"}");

        MYSQL_RES* sql_response;
        MYSQL_ROW sql_row;

        SHA1 checksum;
        checksum.update(pass);
        std::string hashPass = checksum.final();

        if(!exec_NOquery(&sql, {"UPDATE `system_users` SET `pass` = '", hashPass,"' WHERE `name` = '", SINs[sin].user,"';"}))
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't chane pass\"}");

        SINs[sin].expiredate = SINs[sin].creationDT - 1;

        mysql_close(&sql);
        return crow::response(crow::OK);
    });
}
