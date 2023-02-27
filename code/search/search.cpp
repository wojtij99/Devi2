#include "search.hpp"
#include "../tools/sql.hpp"
#include "../tools/sin.hpp"
#include "../tools/tools.hpp"

void devi::Search(crow::App<crow::CORSHandler>& app)
{
    CROW_ROUTE(app, "/search/<string>")
    .methods(crow::HTTPMethod::POST)
    ([&](const crow::request& req, std::string phrase){
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
        std::map<std::string, std::vector<std::string>> response;
        crow::json::wvalue result;

        mysql_query(&sql, "SHOW TABLES;");
        sql_response = mysql_store_result(&sql);

        while ((sql_row = mysql_fetch_row(sql_response)) != NULL)
        {
            std::string temp = sql_row[0];
            if (!isSystemTable(temp) && !temp.rfind("dic_", 0) == 0)
                response[temp];
        }

        phrase = urlDecode(phrase); 
        phrase = serialize(phrase, '\'');

        for(auto r : response)
        {
            std::string query = "SELECT * FROM information_schema.key_column_usage WHERE constraint_schema = 'db_" + SINs[sin].db + "' AND table_name = '" + r.first + "' AND REFERENCED_TABLE_NAME IS NOT NULL;";
            mysql_query(&sql, query.c_str());
            sql_response = mysql_store_result(&sql);

            std::map<std::string, std::string> keys; // column - ref

            while ((sql_row = mysql_fetch_row(sql_response)) != NULL)
                keys[sql_row[6]] = sql_row[10];

            mysql_query(&sql, ("DESCRIBE `" + r.first + "`").c_str());
            sql_response = mysql_store_result(&sql);

            std::vector<std::string> legend;
            query = "SELECT * FROM `" + r.first + "` WHERE ";

            while ((sql_row = mysql_fetch_row(sql_response)) != NULL)
            {
                legend.emplace_back(sql_row[0]);
                query += "`" + (std::string)sql_row[0] + "` LIKE '%" + phrase + "%' OR ";
            }
            query.replace(query.length() - 3, 3, "");

            mysql_query(&sql, query.c_str());
            sql_response = mysql_store_result(&sql);

            if(mysql_num_rows(sql_response) == 0) continue;

            std::vector<std::string> resrow;
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
                result[r.first]["row" + std::to_string(mysql_num_rows(sql_response) - i - 1)] = resrow;
                i++;
            }

            result[r.first]["Types"]  = types;
            result[r.first]["Legend"] = legend;
        }

        mysql_close(&sql);
        return crow::response(crow::OK, result); //, "{\"tables\": \"" + response +"\"}");
    });
}