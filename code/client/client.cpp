#include "client.hpp"
#include "../tools/tools.hpp"
#include "../tools/sin.hpp"
#include "../tools/sql.hpp"
#include "../tools/sha1.hpp"
#include "../tools/permissions.hpp"

void devi::Client(crow::App<crow::CORSHandler>& app)
{
    CROW_ROUTE(app, "/client/addAccount")
    .methods(crow::HTTPMethod::PUT)
    ([&](const crow::request& req){
        auto body = crow::json::load(req.body);
        if(!body) 
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid body\"}");

        std::string name, sin, pass;

        try
        {
            name        = parseStr(body["name"].s());
            pass        = parseStr(body["pass"].s());
            sin         = parseStr(body["sin"].s());
        }
        catch(const std::runtime_error& e)
        {
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid body\"}");
        }

        if(name.find('\'') != std::string::npos) return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Invalid character (') in name\"}");
        if(pass.find('\'') != std::string::npos) return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Invalid character (') in pass\"}");

        if(!checkSIN(sin, req))
            return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Wrong SIN\"}");

        SHA1 checksum;
        checksum.update(pass);
        std::string hashPass = checksum.final();

        MYSQL sql;
        if(!devi::sql_start(&sql, "db_" + SINs[sin].db)) return crow::response(crow::SERVICE_UNAVAILABLE, "{\"response\":\"Can't connect to DB\"}");

        MYSQL_RES* sql_response;
        MYSQL_ROW sql_row;

        if(!exec_NOquery(&sql, {"INSERT INTO `system_users` VALUES(NULL, '", name, "', '", hashPass, "', '{\"global\": [\"NONE\"]}');"})) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't insert user to DB\"}");
        mysql_close(&sql);
        return crow::response(crow::OK);    
    });

    CROW_ROUTE(app, "/client/addPermissions")
    .methods(crow::HTTPMethod::POST)
    ([&](const crow::request& req){
        auto body = crow::json::load(req.body);
        if(!body) 
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid body\"}");

        std::string name, sin, structN, typeN;

        try
        {
            name        = parseStr(body["name"].s());
            structN     = parseStr(body["struct"].s());
            typeN       = parseStr(body["type"].s());
            sin         = parseStr(body["sin"].s());
        }
        catch(const std::runtime_error& e)
        {
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid body\"}");
        }

        if(name.find('\'') != std::string::npos) return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid character (') in name\"}");
        if(structN.find('\'') != std::string::npos) return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid character (') in structN\"}");
        if(typeN.find('\'') != std::string::npos) return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid character (') in typeN\"}");

        if(!checkSIN(sin, req))
            return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Wrong SIN\"}");

        std::map<std::string, permStruct_e> struct_m{
            {"tables", permStruct_e::Table},
            {"sheets", permStruct_e::Sheet},
            {"global", permStruct_e::Global},
            {"pages", permStruct_e::Page}};
        std::map<std::string, permType_e> type_m{
            {"NONE", permType_e::NONE},
            {"READ", permType_e::READ},
            {"UPADTE", permType_e::UPADTE},
            {"DELETE", permType_e::DELETE},
            {"MODIFY", permType_e::MODIFY},
            {"ACCOUNT", permType_e::ACCOUNT},
            {"ALL", permType_e::ALL}
        };

        try
        {
            setPermissions(sin, struct_m[structN] , name, type_m[typeN]);
        }
        catch(int e)
        {
            switch (e)
            {
            case 1: return crow::response(crow::BAD_REQUEST, "{\"response\":\"Can't connect to DB.\"}");
            case 2: return crow::response(crow::BAD_REQUEST, "{\"response\":\"Can't update permission.\"}");
            case 3: return crow::response(crow::BAD_REQUEST, "{\"response\":\"You chose invalid struct.\"}");
            case 4: return crow::response(crow::BAD_REQUEST, "{\"response\":\"You chose invalid type.\"}");
            case 5: return crow::response(crow::BAD_REQUEST, "{\"response\":\"You chose invalid name.\"}");
            }
        }
        
        
        return crow::response(crow::OK);    
    });

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

    CROW_ROUTE(app, "/client/permissions")
    .methods(crow::HTTPMethod::POST)
    ([&](const crow::request& req){
        return "";
    });
}
