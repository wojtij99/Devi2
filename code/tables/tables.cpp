#include "tables.hpp"
#include <mysql/mysql.h>
#include "../tools/sql.hpp"
#include "../tools/sin.hpp"
#include "../tools/tools.hpp"
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
            if (!isSystemTable(temp))
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

        if(name.find('`') != std::string::npos)
            return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Invalid character (`) in name\"}");

        if (isSystemTable(name))
            return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Invalid name\"}");

        if(!checkSIN(sin, req))
            return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Wrong SIN\"}");

        MYSQL sql;
        if(!devi::sql_start(&sql, "db_" + SINs[sin].db)) return crow::response(crow::SERVICE_UNAVAILABLE, "{\"response\":\"Can't connect to DB\"}");

        MYSQL_RES* sql_response;
        MYSQL_ROW sql_row;
        mysql_query(&sql, "START TRANSACTION;");

        if(!exec_NOquery(&sql, {"CREATE TABLE `", name ,"`(ID INT UNSIGNED NOT NULL AUTO_INCREMENT, PRIMARY KEY(ID));"}, true)) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table\"}");

        if(!exec_NOquery(&sql, {"CREATE TABLE `log_", name ,"`(ID INT UNSIGNED NOT NULL AUTO_INCREMENT,action TEXT, actionDT DATETIME NOT NULL, dataID INT, PRIMARY KEY(ID));"}, true)) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table\"}");

        if(!exec_NOquery(&sql, {"CREATE TRIGGER ", SINs[sin].db,"_", name, "_log_insert AFTER INSERT ON ", name, " FOR EACH ROW INSERT INTO log_", name, " VALUES(NULL, \"INSERT\", NOW(), NEW.ID); "}, true)) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table\"}");

        if(!exec_NOquery(&sql, {"CREATE TRIGGER ", SINs[sin].db,"_", name, "_log_update AFTER UPDATE ON ", name, " FOR EACH ROW INSERT INTO log_", name, " VALUES(NULL, \"UPDATE\", NOW(), NEW.ID); "}, true)) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table\"}");

        if(!exec_NOquery(&sql, {"CREATE TRIGGER ", SINs[sin].db,"_", name, "_log_delete AFTER DELETE ON ", name, " FOR EACH ROW INSERT INTO log_", name, " VALUES(NULL, \"DELETE\", NOW(), OLD.ID); "}, true)) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table\"}");

        mysql_query(&sql, "COMMIT;");
        mysql_close(&sql);
        return crow::response(crow::OK);
    });

    CROW_ROUTE(app, "/tables/<string>/addColumn")
    .methods(crow::HTTPMethod::PUT)
    ([&](const crow::request& req, std::string table){
        auto body = crow::json::load(req.body);
        if(!body) 
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid body\"}");

        std::string name, type, sin, references;

        table = urlDecode(table);

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

        if(name.find('`') != std::string::npos) return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Invalid character (`) in user\"}");
        if(table.find('`') != std::string::npos) return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Invalid character (`) in table\"}");

        if (isSystemTable(table) || table.rfind("dic_", 0) == 0)
            return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Invalid table\"}");

        if(name == "ID")
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid name\"}");

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
        
        bool iskey = false;
        std::string ref_table;

        if(type == "KEY") 
        {
            iskey = true;
            type = "INT UNSIGNED";

            try
            {
                ref_table = parseStr(body["reference"].s());
                if(ref_table.find('`') != std::string::npos) return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Invalid character (`) in reference\"}");
            }
            catch(const std::runtime_error& e)
            {
                return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid reference\"}");
            }

            if (isSystemTable(ref_table))
                return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Invalid table\"}");
        }

        if(inCorrectType) 
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Incorrect type\"}");

        MYSQL sql;
        if(!devi::sql_start(&sql, "db_" + SINs[sin].db)) return crow::response(crow::SERVICE_UNAVAILABLE, "{\"response\":\"Can't connect to DB\"}");
        MYSQL_RES* sql_response;
        MYSQL_ROW sql_row;
        mysql_query(&sql, "START TRANSACTION;");
        
        if(iskey)
        {
            if(!exec_NOquery(&sql, {"SELECT * FROM `", ref_table,"`"}, true)) 
                return crow::response(crow::CONFLICT, "{\"response\":\"Wrong reference\"}");
            sql_response = mysql_store_result(&sql);
        }
        
        if(!exec_NOquery(&sql, {"ALTER TABLE `", table ,"` ADD `" , name ,"` ", type , ";"}, true)) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't add column\"}");

        if(!exec_NOquery(&sql, {"ALTER TABLE `log_", table ,"` ADD `new_" , name ,"` ", type , ";"}, true)) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't add column\"}");

        if(!exec_NOquery(&sql, {"ALTER TABLE `log_", table ,"` ADD `old_" , name ,"` ", type , ";"}, true)) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't add column\"}");
        
        std::string query = "SELECT * FROM `" + table + "` ORDER BY `ID` ;";
        mysql_query(&sql, query.c_str());
        sql_response = mysql_store_result(&sql);

        std::string stm_insert = "INSERT INTO log_" + table + " VALUES(NULL, \"INSERT\", NOW(), NEW.ID",
                    stm_update = "INSERT INTO log_" + table + " VALUES(NULL, \"UPDATE\", NOW(), NEW.ID",
                    stm_delete = "INSERT INTO log_" + table + " VALUES(NULL, \"DELETE\", NOW(), OLD.ID";
        MYSQL_FIELD* sql_fil = mysql_fetch_fields(sql_response);
        for (int i = 0; i < mysql_num_fields(sql_response); i++) 
        {
            std::string temp = std::string(sql_fil[i].name);
            if(temp == "ID") 
                continue;
            stm_insert += ", NEW.`" + temp + "`, NULL";
            stm_update += ", NEW.`" + temp + "`, OLD.`" + temp + "`";
            stm_delete += ", NULL, OLD.`" + temp + "`";
        }

        if(!exec_NOquery(&sql, {"DROP TRIGGER IF EXISTS ", SINs[sin].db,"_", table, "_log_insert;"}, true)) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table1\"}");
        
        if(!exec_NOquery(&sql, {"DROP TRIGGER IF EXISTS ", SINs[sin].db,"_", table, "_log_update;"}, true)) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table2\"}");
        
        if(!exec_NOquery(&sql, {"DROP TRIGGER IF EXISTS ", SINs[sin].db,"_", table, "_log_delete;"}, true)) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table3\"}");
        
        if(!exec_NOquery(&sql, {"CREATE TRIGGER ", SINs[sin].db,"_", table, "_log_insert AFTER INSERT ON ", table, " FOR EACH ROW ", stm_insert, "); "}, true)) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table4\"}");

        if(!exec_NOquery(&sql, {"CREATE TRIGGER ", SINs[sin].db,"_", table, "_log_update AFTER UPDATE ON ", table, " FOR EACH ROW ", stm_update, "); "}, true)) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table5\"}");

        if(!exec_NOquery(&sql, {"CREATE TRIGGER ", SINs[sin].db,"_", table, "_log_delete AFTER DELETE ON ", table, " FOR EACH ROW ", stm_delete, "); "}, true)) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table6\"}");

        if(iskey)
        {
            if(!exec_NOquery(&sql, {"ALTER TABLE `", table,"` ADD CONSTRAINT `ref_", name,"_", table ,"` FOREIGN KEY (`", name,"`) REFERENCES `", ref_table,"`(`ID`) ON DELETE RESTRICT ON UPDATE RESTRICT;"}, true)) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table7\"}");
        }

        mysql_query(&sql, "COMMIT;");
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
        table = urlDecode(table);
        if(table.find('`') != std::string::npos) return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Invalid character (`) in table\"}");

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

        if (isSystemTable(table))
            return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Invalid table\"}");

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

        std::string sin, value;
        crow::json::rvalue data;
        table = urlDecode(table);
        if(table.find('`') != std::string::npos) return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Invalid character (`) in table\"}");

        try
        {
            sin     = parseStr(body["sin"].s());
            data    = body["data"];
        }
        catch(const std::runtime_error& e)
        {
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid body\"}");
        }

        if(!checkSIN(sin, req))
            return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Wrong SIN\"}");

        if (isSystemTable(table))
            return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Invalid table\"}");

        MYSQL sql;
        if(!devi::sql_start(&sql, "db_" + SINs[sin].db)) return crow::response(crow::SERVICE_UNAVAILABLE, "{\"response\":\"Can't connect to DB\"}");
        MYSQL_RES* sql_response;
        MYSQL_ROW sql_row;

        std::string query = "DESCRIBE `" + table + "`;";
        mysql_query(&sql, query.c_str());
        sql_response = mysql_store_result(&sql);

        std::unordered_map<std::string, std::string> values;
        short i = 0;
        while ((sql_row = mysql_fetch_row(sql_response)) != NULL)
        {
            values[sql_row[0]] = "";
            i++;
        }

        for(auto b : data)
        {
            if(b.key() == "ID") continue;
            if(values.find(b.key()) == values.end())
                return crow::response(crow::BAD_REQUEST, "{\"response\":\"Column `" + (std::string)b.key() + "` does not exist in this table \"}");
            values[b.key()] = b.s();
        }
        
        std::string values_str = "";

        for(auto s: values) 
        {
            if(s.second == "") continue;
            values_str += "`" + s.first + "`='" + s.second + "',";
        }

        values_str[values_str.length() - 1] = ' ';

        if(!exec_NOquery(&sql, {"UPDATE `", table ,"` SET ", values_str ," WHERE `ID` = '", std::to_string(id),"' ;"}, false)) 
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
        table = urlDecode(table);
        if(table.find('`') != std::string::npos) return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Invalid character (`) in table\"}");

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

        if (isSystemTable(table))
            return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Invalid table\"}");

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
#if ENABLE_SELECT_ONE == false
        return crow::response(crow::NOT_IMPLEMENTED);
#elif
        auto body = crow::json::load(req.body);
        if(!body) 
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid body\"}");

        std::string sin;

        table = urlDecode(table);
        if(table.find('`') != std::string::npos) return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Invalid character (`) in table\"}");

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

        if (isSystemTable(table))
            return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Invalid table\"}");

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
#endif
    });

    CROW_ROUTE(app, "/tables/<string>/select/all")
    .methods(crow::HTTPMethod::POST)
    ([&](const crow::request& req, std::string table){
        auto body = crow::json::load(req.body);
        if(!body) 
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid body\"}");

        std::string sin;
        crow::json::rvalue queryJ;
        table = urlDecode(table);
        if(table.find('`') != std::string::npos) return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Invalid character (`) in table\"}");

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

        if (table.rfind("system_", 0) == 0)
            return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Invalid table\"}");

        MYSQL sql;
        if(!devi::sql_start(&sql, "db_" + SINs[sin].db)) return crow::response(crow::SERVICE_UNAVAILABLE, "{\"response\":\"Can't connect to DB\"}");
        MYSQL_RES* sql_response;
        MYSQL_ROW sql_row;

        std::string query = "SELECT * FROM information_schema.key_column_usage WHERE constraint_schema = 'db_" + SINs[sin].db + "' AND table_name = '" + table + "' AND REFERENCED_TABLE_NAME IS NOT NULL;";
        mysql_query(&sql, query.c_str());
        sql_response = mysql_store_result(&sql);

        std::map<std::string, std::string> keys; // column - ref

        while ((sql_row = mysql_fetch_row(sql_response)) != NULL)
            keys[sql_row[6]] = sql_row[10];

        int limit = 0, page = 0;
        std::string orderBy = "ID", orderType = "ASC";

        try
        {
            try {getURL_param(req, "limit", [&](char* v) { (is_number(v)) ? limit = std::atoi(v) : limit = 0; }); } catch(int e) {;}
            try {getURL_param(req, "page", [&](char* v) { (is_number(v)) ? page = std::atoi(v) : page = 0; });} catch(int e) {;}
            try {getURL_param(req, "orderBy", [&](char* v) { (v == "") ? orderBy = v : orderBy = "ID"; });} catch(int e) {;}
            try {getURL_param(req, "orderType", [&](char* v) { (v == "") ? orderType = v : orderType = "ASC"; });} catch(int e) {;}
        }
        catch(const std::runtime_error& e)
        {
            return crow::response(crow::BAD_REQUEST, "Invalid query");
        }

        if(toUpper(orderType) != "ASC" && toUpper(orderType) != "DESC")
            return crow::response(crow::BAD_REQUEST, "Wrong order type");

        std::string limit_str = "";

        if(limit) limit_str = "LIMIT " + std::to_string(page * limit) + " , " + std::to_string(limit);

        std::string where = "";
        try
        {
            queryJ = body["query"];
        }
        catch(std::runtime_error& e)
        {
            where = "1";
        }
        
        if(where != "1")
        {
            std::string query = "DESCRIBE `" + table + "`;";
            mysql_query(&sql, query.c_str());
            sql_response = mysql_store_result(&sql);

            std::unordered_map<std::string, std::string> values;
            while ((sql_row = mysql_fetch_row(sql_response)) != NULL)
            {
                values[sql_row[0]] = "";
            }

            for(auto b : queryJ)
            {
                if(values.find(b.key()) == values.end())
                    return crow::response(crow::BAD_REQUEST, "{\"response\":\"Column `" + (std::string)b.key() + "` does not exist in this table \"}");
                values[b.key()] = b.s();
            }

            for(auto s: values) 
            {
                if(s.second == "") continue;
                where += "`" + s.first + "` LIKE '%" + s.second + "%' AND ";
            }

            where.replace(where.length() - 4, 4, "");
        }

        //std::cout << where << std::endl;

        query = "SELECT * FROM `" + table + "` WHERE " + where + " ORDER BY `" + orderBy + "` " + orderType + " " + limit_str + " ;";
        //std::cout << query_p << std::endl;
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
            //std::cout << sql_fil[i].type << std::endl;
            std::map<std::string, std::string>::iterator keyPoz;

            switch (sql_fil[i].type)
            {
                case enum_field_types::MYSQL_TYPE_LONG:     types.emplace_back(((keyPoz = keys.find(sql_fil[i].name)) != keys.end()) ? "KEY-" + keyPoz->second : "INT");break;
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

    CROW_ROUTE(app, "/tables/<string>/alter")
    .methods(crow::HTTPMethod::POST)
    ([&](const crow::request& req, std::string table){
        auto body = crow::json::load(req.body);
        if(!body) 
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid body\"}");

        std::string sin, name, newName, newType;
        table = urlDecode(table);
        if(table.find('`') != std::string::npos) return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Invalid character (`) in table\"}");

        try
        {
            sin     = parseStr(body["sin"].s());
            name    = parseStr(body["name"].s());
            newName = parseStr(body["newName"].s());
            newType = parseStr(body["newType"].s());
        }
        catch(const std::runtime_error& e)
        {
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid body\"}");
        }

        if(name.find('`') != std::string::npos) return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Invalid character (`) in name\"}");

        if(name == "ID")
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid name\"}");

        if(!checkSIN(sin, req))
            return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Wrong SIN\"}");

        if (isSystemTable(table) || table.rfind("dic_", 0) == 0)
            return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Invalid table\"}");

        std::vector<std::string> types = {"INT", "TEXT", "DATETIME", "TIME", "DATE", "FLOAT", "BOOL", "KEY"};
        boost::to_upper(newType);
        
        bool inCorrectType = true;
        for(std::string t : types)
            if(t == newType)
            {
                inCorrectType = false;
                continue;
            }
        
        bool isKey = false;
        if(newType == "KEY")
        {
            newType = "INT UNSIGNED";
            isKey = true;
        }

        if(inCorrectType) 
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Incorrect type\"}");

        MYSQL sql;
        if(!devi::sql_start(&sql, "db_" + SINs[sin].db)) 
            return crow::response(crow::SERVICE_UNAVAILABLE, "{\"response\":\"Can't connect to DB\"}");
        MYSQL_RES* sql_response;
        MYSQL_ROW sql_row;

        if(!exec_NOquery(&sql, {"ALTER TABLE `", table ,"` CHANGE `", name ,"` `", newName,"` ", newType ," ;"})) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table\"}");

        if(!exec_NOquery(&sql, {"ALTER TABLE `log_", table ,"` CHANGE `new_", name ,"` `new_", newName,"` ", newType ," ;"})) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table\"}");

        if(!exec_NOquery(&sql, {"ALTER TABLE `log_", table ,"` CHANGE `old_", name ,"` `old_", newName,"` ", newType ," ;"})) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table\"}");

        if(isKey)
        {
            std::string query = "SELECT * FROM information_schema.key_column_usage WHERE constraint_schema = 'db_" + SINs[sin].db + "' AND table_name = '" + table + "' AND REFERENCED_TABLE_NAME IS NOT NULL AND COLUMN_NAME = '" + newName + "'";
            mysql_query(&sql, query.c_str());
            sql_response = mysql_store_result(&sql);
            
            std::string ConName, ref_table;
            while ((sql_row = mysql_fetch_row(sql_response)) != NULL)
            {
                ConName = sql_row[2];
                ref_table = sql_row[10];
            }

            if(!exec_NOquery(&sql, {"ALTER TABLE `", table,"` DROP CONSTRAINT `", ConName,"`;"}, true)) 
                return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table7\"}");
            if(!exec_NOquery(&sql, {"ALTER TABLE `", table,"` ADD CONSTRAINT `ref_", newName,"_", table ,"` FOREIGN KEY (`", newName,"`) REFERENCES `", ref_table,"`(`ID`) ON DELETE RESTRICT ON UPDATE RESTRICT;"}, true)) 
                return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table7\"}");
        }

        std::string query = "SELECT * FROM `" + table + "` ORDER BY `ID` ;";
        mysql_query(&sql, query.c_str());
        sql_response = mysql_store_result(&sql);

        std::string stm_insert = "INSERT INTO log_" + table + " VALUES(NULL, \"INSERT\", NOW(), NEW.ID",
                    stm_update = "INSERT INTO log_" + table + " VALUES(NULL, \"UPDATE\", NOW(), NEW.ID",
                    stm_delete = "INSERT INTO log_" + table + " VALUES(NULL, \"DELETE\", NOW(), OLD.ID";
        MYSQL_FIELD* sql_fil = mysql_fetch_fields(sql_response);
        for (int i = 0; i < mysql_num_fields(sql_response); i++) 
        {
            std::string temp = std::string(sql_fil[i].name);
            if(temp == "ID") 
                continue;
            stm_insert += ", NEW.`" + temp + "`, NULL";
            stm_update += ", NEW.`" + temp + "`, OLD.`" + temp + "`";
            stm_delete += ", NULL, OLD.`" + temp + "`";
        }

        if(!exec_NOquery(&sql, {"DROP TRIGGER IF EXISTS ", SINs[sin].db,"_", table, "_log_insert;"}, true)) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table1\"}");
        
        if(!exec_NOquery(&sql, {"DROP TRIGGER IF EXISTS ", SINs[sin].db,"_", table, "_log_update;"}, true)) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table2\"}");
        
        if(!exec_NOquery(&sql, {"DROP TRIGGER IF EXISTS ", SINs[sin].db,"_", table, "_log_delete;"}, true)) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table3\"}");
        
        if(!exec_NOquery(&sql, {"CREATE TRIGGER ", SINs[sin].db,"_", table, "_log_insert AFTER INSERT ON ", table, " FOR EACH ROW ", stm_insert, "); "}, true)) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table4\"}");

        if(!exec_NOquery(&sql, {"CREATE TRIGGER ", SINs[sin].db,"_", table, "_log_update AFTER UPDATE ON ", table, " FOR EACH ROW ", stm_update, "); "}, true)) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table5\"}");

        if(!exec_NOquery(&sql, {"CREATE TRIGGER ", SINs[sin].db,"_", table, "_log_delete AFTER DELETE ON ", table, " FOR EACH ROW ", stm_delete, "); "}, true)) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table6\"}");


        mysql_close(&sql);
        return crow::response(crow::OK);
    });

    CROW_ROUTE(app, "/tables/<string>/rename")
    .methods(crow::HTTPMethod::POST)
    ([&](const crow::request& req, std::string table){
        auto body = crow::json::load(req.body);
        if(!body) 
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid body\"}");

        std::string sin, name;
        table = urlDecode(table);
        if(table.find('`') != std::string::npos) return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Invalid character (`) in table\"}");

        try
        {
            sin     = parseStr(body["sin"].s());
            name    = parseStr(body["name"].s());
        }
        catch(const std::runtime_error& e)
        {
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid body\"}");
        }

        if(name.find('`') != std::string::npos) return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Invalid character (`) in name\"}");
        if(name == "ID")
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid name\"}");

        if(!checkSIN(sin, req))
            return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Wrong SIN\"}");

        if (isSystemTable(table))
            return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Invalid table\"}");

        MYSQL sql;
        if(!devi::sql_start(&sql, "db_" + SINs[sin].db)) 
            return crow::response(crow::SERVICE_UNAVAILABLE, "{\"response\":\"Can't connect to DB\"}");
        MYSQL_RES* sql_response;
        MYSQL_ROW sql_row;

        if(table.rfind("dic_", 0) == 0 && name.rfind("dic_", 0) != 0) name = "dic_" + name;

        if(!exec_NOquery(&sql, {"RENAME TABLE `db_", SINs[sin].db ,"`.`", table ,"` TO `db_", SINs[sin].db ,"`.`", name ,"`;"})) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table\"}");

        if(table.rfind("dic_", 0) == 0)
        {
            mysql_close(&sql);
            return crow::response(crow::OK);
        }

        if(!exec_NOquery(&sql, {"RENAME TABLE `db_", SINs[sin].db ,"`.`log_", table ,"` TO `db_", SINs[sin].db ,"`.`log_", name ,"`;"})) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table\"}");


        std::string query = "SELECT * FROM `" + name + "` ORDER BY `ID` ;";
        mysql_query(&sql, query.c_str());
        sql_response = mysql_store_result(&sql);

        std::string stm_insert = "INSERT INTO log_" + name + " VALUES(NULL, \"INSERT\", NOW(), NEW.ID",
                    stm_update = "INSERT INTO log_" + name + " VALUES(NULL, \"UPDATE\", NOW(), NEW.ID",
                    stm_delete ="INSERT INTO log_" + name + " VALUES(NULL, \"DELETE\", NOW(), OLD.ID";
        MYSQL_FIELD* sql_fil = mysql_fetch_fields(sql_response);
        for (int i = 0; i < mysql_num_fields(sql_response); i++) 
        {
            std::string temp = std::string(sql_fil[i].name);
            if(temp == "ID") 
                continue;
            stm_insert += ", NEW.`" + temp + "`, NULL";
            stm_update += ", NEW.`" + temp + "`, OLD.`" + temp + "`";
            stm_delete += ", NULL, OLD.`" + temp + "`";
        }

        if(!devi::sql_start(&sql, "db_" + SINs[sin].db)) 
            return crow::response(crow::SERVICE_UNAVAILABLE, "{\"response\":\"Can't connect to DB\"}");

        exec_NOquery(&sql, {"START TRANSACTION"});

        if(!exec_NOquery(&sql, {"DROP TRIGGER IF EXISTS ", SINs[sin].db,"_", table, "_log_insert;"}, true)) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table1\"}");
        
        if(!exec_NOquery(&sql, {"DROP TRIGGER IF EXISTS ", SINs[sin].db,"_", table, "_log_update;"}, true)) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table2\"}");
        
        if(!exec_NOquery(&sql, {"DROP TRIGGER IF EXISTS ", SINs[sin].db,"_", table, "_log_delete;"}, true)) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table3\"}");
        
        if(!exec_NOquery(&sql, {"CREATE TRIGGER ", SINs[sin].db,"_", name, "_log_insert AFTER INSERT ON ", name, " FOR EACH ROW ", stm_insert, "); "}, true)) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table4\"}");

        if(!exec_NOquery(&sql, {"CREATE TRIGGER ", SINs[sin].db,"_", name, "_log_update AFTER UPDATE ON ", name, " FOR EACH ROW ", stm_update, "); "}, true)) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table5\"}");

        if(!exec_NOquery(&sql, {"CREATE TRIGGER ", SINs[sin].db,"_", name, "_log_delete AFTER DELETE ON ", name, " FOR EACH ROW ", stm_delete, "); "}, true)) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table6\"}");

        mysql_close(&sql);
        return crow::response(crow::OK);
    });

    CROW_ROUTE(app, "/tables/<string>/dropColumn")
    .methods(crow::HTTPMethod::DELETE)
    ([&](const crow::request& req, std::string table){
        auto body = crow::json::load(req.body);
        if(!body) 
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid body\"}");

        std::string sin, name;
        table = urlDecode(table);
        if(table.find('`') != std::string::npos) return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Invalid character (`) in table\"}");

        try
        {
            sin     = parseStr(body["sin"].s());
            name    = parseStr(body["name"].s());
        }
        catch(const std::runtime_error& e)
        {
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid body\"}");
        }

        if(name.find('`') != std::string::npos) return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Invalid character (`) in name\"}");
        if(name == "ID")
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid name\"}");

        if(!checkSIN(sin, req))
            return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Wrong SIN\"}");

        if (isSystemTable(table) || table.rfind("dic_", 0) == 0)
            return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Invalid table\"}");

        MYSQL sql;
        if(!devi::sql_start(&sql, "db_" + SINs[sin].db)) 
            return crow::response(crow::SERVICE_UNAVAILABLE, "{\"response\":\"Can't connect to DB\"}");
        MYSQL_RES* sql_response;
        MYSQL_ROW sql_row;
        std::string sqlError;

        if(!exec_NOquery(&sql, {"ALTER TABLE `", table,"` DROP `", name,"`;"}, false, false, &sqlError)) 
        {
            if(sqlError.find("needed in a foreign key constraint") != std::string::npos)
            {
                if(!exec_NOquery(&sql, {"ALTER TABLE `", table,"` DROP CONSTRAINT `", sqlError.substr(19, sqlError.find("'", 20) - 19),"`;"})) 
                    return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table\"}");        
                
                if(!exec_NOquery(&sql, {"ALTER TABLE `", table,"` DROP `", name,"`;"})) 
                    return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table\"}");
            }
            else
                return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table\"}");
        }
        
        if(!exec_NOquery(&sql, {"ALTER TABLE `log_", table,"` DROP `old_", name,"`;"})) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table\"}");

        if(!exec_NOquery(&sql, {"ALTER TABLE `log_", table,"` DROP `new_", name,"`;"})) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table\"}");

        std::string query = "SELECT * FROM `" + table + "` ORDER BY `ID` ;";
        mysql_query(&sql, query.c_str());
        sql_response = mysql_store_result(&sql);

        std::string stm_insert = "INSERT INTO log_" + table + " VALUES(NULL, \"INSERT\", NOW(), NEW.ID",
                    stm_update = "INSERT INTO log_" + table + " VALUES(NULL, \"UPDATE\", NOW(), NEW.ID",
                    stm_delete = "INSERT INTO log_" + table + " VALUES(NULL, \"DELETE\", NOW(), OLD.ID";
        MYSQL_FIELD* sql_fil = mysql_fetch_fields(sql_response);
        for (int i = 0; i < mysql_num_fields(sql_response); i++) 
        {
            std::string temp = std::string(sql_fil[i].name);
            if(temp == "ID") 
                continue;
            stm_insert += ", NEW.`" + temp + "`, NULL";
            stm_update += ", NEW.`" + temp + "`, OLD.`" + temp + "`";
            stm_delete += ", NULL, OLD.`" + temp + "`";
        }

        if(!devi::sql_start(&sql, "db_" + SINs[sin].db)) 
            return crow::response(crow::SERVICE_UNAVAILABLE, "{\"response\":\"Can't connect to DB\"}");

        exec_NOquery(&sql, {"START TRANSACTION"});

        if(!exec_NOquery(&sql, {"DROP TRIGGER IF EXISTS ", SINs[sin].db,"_", table, "_log_insert;"}, true)) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table1\"}");
        
        if(!exec_NOquery(&sql, {"DROP TRIGGER IF EXISTS ", SINs[sin].db,"_", table, "_log_update;"}, true)) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table2\"}");
        
        if(!exec_NOquery(&sql, {"DROP TRIGGER IF EXISTS ", SINs[sin].db,"_", table, "_log_delete;"}, true)) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table3\"}");
        
        if(!exec_NOquery(&sql, {"CREATE TRIGGER ", SINs[sin].db,"_", table, "_log_insert AFTER INSERT ON ", table, " FOR EACH ROW ", stm_insert, "); "}, true)) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table4\"}");

        if(!exec_NOquery(&sql, {"CREATE TRIGGER ", SINs[sin].db,"_", table, "_log_update AFTER UPDATE ON ", table, " FOR EACH ROW ", stm_update, "); "}, true)) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table5\"}");

        if(!exec_NOquery(&sql, {"CREATE TRIGGER ", SINs[sin].db,"_", table, "_log_delete AFTER DELETE ON ", table, " FOR EACH ROW ", stm_delete, "); "}, true)) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table6\"}");

        mysql_close(&sql);
        return crow::response(crow::OK);
    });

    CROW_ROUTE(app, "/tables/<string>/dropTable")
    .methods(crow::HTTPMethod::DELETE)
    ([&](const crow::request& req, std::string table){
        auto body = crow::json::load(req.body);
        if(!body) 
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid body\"}");

        std::string sin;
        table = urlDecode(table);
        if(table.find('`') != std::string::npos) return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Invalid character (`) in table\"}");

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

        if (isSystemTable(table))
            return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Invalid table\"}");

        MYSQL sql;
        if(!devi::sql_start(&sql, "db_" + SINs[sin].db)) 
            return crow::response(crow::SERVICE_UNAVAILABLE, "{\"response\":\"Can't connect to DB\"}");
        MYSQL_RES* sql_response;
        MYSQL_ROW sql_row;

        if(!exec_NOquery(&sql, {"DROP TABLE `", table,"`;"})) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table\"}");
        
        if(!exec_NOquery(&sql, {"DROP TABLE `log_", table,"`;"})) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table\"}");

        if(!exec_NOquery(&sql, {"DROP TRIGGER IF EXISTS ", SINs[sin].db,"_", table, "_log_insert;"}, true)) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table1\"}");
        
        if(!exec_NOquery(&sql, {"DROP TRIGGER IF EXISTS ", SINs[sin].db,"_", table, "_log_update;"}, true)) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table2\"}");
        
        if(!exec_NOquery(&sql, {"DROP TRIGGER IF EXISTS ", SINs[sin].db,"_", table, "_log_delete;"}, true)) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table3\"}");

        mysql_close(&sql);
        return crow::response(crow::OK);
    });

    /*CROW_ROUTE(app, "/tables/Dictionaries")
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
            if (!isSystemTable(temp))
                response += temp + ",";
        }

        if(response.length() > 1)
            response[response.length() - 1] = ' ';

        mysql_close(&sql);
        return crow::response(crow::OK, "{\"tables\": \"" + response +"\"}");
    });*/

    CROW_ROUTE(app, "/tables/DictionariesAdd")
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

        if(name.find('`') != std::string::npos) return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Invalid character (`) in name\"}");
        if (isSystemTable(name))
            return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Invalid name\"}");

        if(!checkSIN(sin, req))
            return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Wrong SIN\"}");

        MYSQL sql;
        if(!devi::sql_start(&sql, "db_" + SINs[sin].db)) return crow::response(crow::SERVICE_UNAVAILABLE, "{\"response\":\"Can't connect to DB\"}");

        MYSQL_RES* sql_response;
        MYSQL_ROW sql_row;
        mysql_query(&sql, "START TRANSACTION;");

        if(!exec_NOquery(&sql, {"CREATE TABLE `dic_", name ,"`(ID INT UNSIGNED NOT NULL AUTO_INCREMENT, value TEXT NOT NULL, PRIMARY KEY(ID));"}, true)) 
            return crow::response(crow::CONFLICT, "{\"response\":\"Can't create table\"}");

        mysql_query(&sql, "COMMIT;");
        mysql_close(&sql);
        return crow::response(crow::OK);
    });
}
