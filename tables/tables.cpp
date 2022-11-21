#include "tables.hpp"
#include <mysql/mysql.h>
#include "../tools/sql.hpp"
#include "../tools/sin.hpp"
#include <map>

void devi::Tables(crow::App<crow::CORSHandler>& app)
{
    CROW_ROUTE(app, "/tables")
    .methods(crow::HTTPMethod::POST)
    ([&](const crow::request& req){
        auto body = crow::json::load(req.body);
        if(!body) 
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid body\"}");

        std::string sin;

        try
        {
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
        std::string response = "";

        mysql_query(&sql, "SHOW TABLES;");
        sql_response = mysql_store_result(&sql);

        while ((sql_row = mysql_fetch_row(sql_response)) != NULL)
        {
            std::string temp = sql_row[0];
            if (temp.rfind("system_", 0) != 0)
                response += temp + ",";
        }

        if(response.length() > 1)
            response[response.length() - 1] = ' ';

        mysql_close(&sql);
        return crow::response(crow::OK, "{\"tables\": \"" + response +"\"}");
    });

    CROW_ROUTE(app, "/tables/add")
    .methods(crow::HTTPMethod::PUT)
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

        if(!checkSIN(sin, req))
            return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Wrong SIN\"}");

        MYSQL sql;
        if(!devi::sql_start(&sql, "db_" + SINs[sin].db)) return crow::response(crow::SERVICE_UNAVAILABLE, "{\"response\":\"Can't connect to DB\"}");

        MYSQL_RES* sql_response;
        MYSQL_ROW sql_row;
        
        if(!exec_NOquery(&sql, {"CREATE TABLE `", name ,"`(ID INT NOT NULL AUTO_INCREMENT, PRIMARY KEY(ID));"})) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table\"}");

        mysql_close(&sql);
        return crow::response(crow::OK);
    });

    CROW_ROUTE(app, "/tables/<string>/addColumn")
    .methods(crow::HTTPMethod::PUT)
    ([&](const crow::request& req, std::string table){
        auto body = crow::json::load(req.body);
        if(!body) 
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid body\"}");

        std::string name, type, sin;

        try
        {
            name    = parseStr(body["name"].s());
            type    = parseStr(body["type"].s());
            sin     = parseStr(body["sin"].s());
        }
        catch(const std::runtime_error& e)
        {
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid body\"}");
        }

        if(!checkSIN(sin, req))
            return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Wrong SIN\"}");

        std::vector<std::string> types = {"INT", "TEXT", "DATETIME", "TIME", "DATE", "FLOAT", "BOOL", "KEY"};
        boost::to_upper(type);
        
        bool inCorrectType = true;
        for(std::string t : types)
            if(t == type)
            {
                inCorrectType = false;
                continue;
            }
        if(type == "KEY") type = "INT";

        if(inCorrectType) 
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Incorrect type\"}");

        MYSQL sql;
        if(!devi::sql_start(&sql, "db_" + SINs[sin].db)) return crow::response(crow::SERVICE_UNAVAILABLE, "{\"response\":\"Can't connect to DB\"}");
        MYSQL_RES* sql_response;
        MYSQL_ROW sql_row;
        
        if(!exec_NOquery(&sql, {"ALTER TABLE `", table ,"` ADD `" , name ,"` ", type , ";"}, false)) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table\"}");

        mysql_close(&sql);
        return crow::response(crow::OK);
    });

    CROW_ROUTE(app, "/tables/<string>/insert")
    .methods(crow::HTTPMethod::PUT)
    ([&](const crow::request& req, std::string table){
        auto body = crow::json::load(req.body);
        if(!body) 
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid body\"}");

        std::string sin;

        try
        {
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

        std::string query = "DESCRIBE `" + table + "`;";
        mysql_query(&sql, query.c_str());
        sql_response = mysql_store_result(&sql);

        std::unordered_map<std::string, std::pair<short, std::string>> values;
        short i = 0;
        while ((sql_row = mysql_fetch_row(sql_response)) != NULL)
        {
            values[sql_row[0]] = std::pair<short, std::string>(i,"");
            i++;
        }

        for(auto b : body)
        {
            if(b.key() == "sin" || b.key() == "ID") continue;
            if(values.find(b.key()) == values.end())
                return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid body\"}");
            values[b.key()] = std::pair<short, std::string>(values[b.key()].first,b.s());
        }

        std::string values_oredred[values.size()];
        for(auto v : values)
        {
            std::string temp;
            if(v.second.second == "")
                temp = "NULL,";
            else
                temp = "'" + v.second.second + "',";

            values_oredred[v.second.first] = temp;
        }
        std::string values_str = "";

        for(auto s: values_oredred) 
            values_str += s;

        values_str[values_str.length() - 1] = ' ';

        std::cout << values_str << std::endl;

        if(!exec_NOquery(&sql, {"INSERT INTO `", table ,"` VALUES(", values_str ,");"}, false)) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't insert data\"}");

        mysql_close(&sql);
        return crow::response(crow::OK);
    });

    CROW_ROUTE(app, "/tables/<string>/update/<int>")
    .methods(crow::HTTPMethod::POST)
    ([&](const crow::request& req, std::string table, int id){
        auto body = crow::json::load(req.body);
        if(!body) 
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid body\"}");

        std::string sin, column, value;

        try
        {
            column  = parseStr(body["column"].s());
            value   = parseStr(body["value"].s());
            sin     = parseStr(body["sin"].s());
        }
        catch(const std::runtime_error& e)
        {
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid body\"}");
        }

        if(column == "ID")
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid column\"}");

        if(!checkSIN(sin, req))
            return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Wrong SIN\"}");

        MYSQL sql;
        if(!devi::sql_start(&sql, "db_" + SINs[sin].db)) return crow::response(crow::SERVICE_UNAVAILABLE, "{\"response\":\"Can't connect to DB\"}");
        MYSQL_RES* sql_response;
        MYSQL_ROW sql_row;
        
        if(!exec_NOquery(&sql, {"UPDATE `", table ,"` SET `", column ,"` = '", value ,"' WHERE `ID` = '", std::to_string(id),"' ;"}, false)) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table\"}");

        mysql_close(&sql);
        return crow::response(crow::OK);
    });

    CROW_ROUTE(app, "/tables/<string>/delete/<int>")
    .methods(crow::HTTPMethod::DELETE)
    ([&](const crow::request& req, std::string table, int id){
        auto body = crow::json::load(req.body);
        if(!body) 
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid body\"}");

        std::string sin;

        try
        {
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
        
        if(!exec_NOquery(&sql, {"DELETE FROM `", table ,"` WHERE `ID` = '", std::to_string(id),"' ;"}, false)) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table\"}");

        mysql_close(&sql);
        return crow::response(crow::OK);
    });

    CROW_ROUTE(app, "/tables/<string>/select/<int>")
    .methods(crow::HTTPMethod::POST)
    ([&](const crow::request& req, std::string table, int id){
        auto body = crow::json::load(req.body);
        if(!body) 
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid body\"}");

        std::string sin;

        try
        {
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

        std::string query = "SELECT * FROM `" + table + "` WHERE `ID` = '" + std::to_string(id) + "' ;";
        mysql_query(&sql, query.c_str());
        sql_response = mysql_store_result(&sql);

        crow::json::wvalue result;

        MYSQL_FIELD* sql_fil = mysql_fetch_fields(sql_response);
        while ((sql_row = mysql_fetch_row(sql_response)) != NULL)
        {
            for (int i = 0; i < mysql_num_fields(sql_response); i++)
            {
                if (sql_row[i] != NULL)
                    result[sql_fil[i].name] = sql_row[i];
                else
                    result[sql_fil[i].name] = "NULL";
            }
        }

        mysql_close(&sql);
        return crow::response(crow::OK, result);
    });

    CROW_ROUTE(app, "/tables/<string>/select/all")
    .methods(crow::HTTPMethod::POST)
    ([&](const crow::request& req, std::string table){
        auto body = crow::json::load(req.body);
        if(!body) 
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid body\"}");

        std::string sin;

        try
        {
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

        std::string query = "SELECT * FROM `" + table + "` ORDER BY `ID` DESC;";
        mysql_query(&sql, query.c_str());
        sql_response = mysql_store_result(&sql);

        crow::json::wvalue result;
        std::vector<std::string> resrow;
        std::vector<std::string> legend;
        std::vector<std::string> types;

        MYSQL_FIELD* sql_fil = mysql_fetch_fields(sql_response);
        for (int i = 0; i < mysql_num_fields(sql_response); i++) 
        {
            legend.emplace_back(sql_fil[i].name);
            //types.emplace_back(sql_fil[i].type);
            switch (sql_fil[i].type)
            {
                case enum_field_types::MYSQL_TYPE_LONG:     types.emplace_back("INT");break;
                case enum_field_types::MYSQL_TYPE_BLOB:     types.emplace_back("TEXT");break;
                case enum_field_types::MYSQL_TYPE_DATE:     types.emplace_back("DATE");break;
                case enum_field_types::MYSQL_TYPE_TIME:     types.emplace_back("TIME");break;
                case enum_field_types::MYSQL_TYPE_DATETIME: types.emplace_back("DATETIME");break;
                case enum_field_types::MYSQL_TYPE_FLOAT:    types.emplace_back("FLOAT");break;
                case enum_field_types::MYSQL_TYPE_TINY:     types.emplace_back("BOOL");break;
            default:
                break;
            }
        }
        
        int i = 0;
        while ((sql_row = mysql_fetch_row(sql_response)) != NULL)
        {
            resrow.clear();
            for (int j = 0; j < mysql_num_fields(sql_response); j++)
            {
                if (sql_row[j] != NULL)
                    resrow.emplace_back(sql_row[j]);
                else
                    resrow.emplace_back("NULL");
            }
            result["row" + std::to_string(mysql_num_rows(sql_response) - i - 1)] = resrow;
            i++;
        }

        result["Types"]  = types;
        result["Legend"] = legend;
        
        mysql_close(&sql);
        return crow::response(crow::OK, result);
    });
}
