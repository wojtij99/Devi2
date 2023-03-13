#include "sheet.hpp"
#include <mysql/mysql.h>
#include <fstream>
#include "../tools/sql.hpp"
#include "../tools/sin.hpp"
#include "../tools/tools.hpp"

#define MYSQL_CON_ERROR 1
#define INVALID_METHOD 2
#define INVALID_QUERY 3
#define INVALID_NUMS_OF_ARGS 4
#define INVALID_ARGS 5
#define IS_NOT_DB_FUNCTION 6

// bool isDBobject(std::string _str)
// {
//     if(_str.)
// }

float aggregateFun(std::string _method, std::vector<std::string> _args, std::string _sin)
{
    if(_args.size() == 0) throw INVALID_NUMS_OF_ARGS;
    if(_args[0][0] != '`' || _args[0][_args[0].length() - 1] != '`') throw IS_NOT_DB_FUNCTION;

    const static std::vector<std::string> parameters = {"SUM", "AVG", "MAX", "MIN", "COUNT", "SUM_IF", "AVG_IF", "SUM_IFS", "AVG_IFS"};
    const static std::vector<std::string> args1 = {"SUM", "AVG", "MAX", "MIN", "COUNT"};
    const static std::vector<std::string> args2 = {"SUM_IF", "AVG_IF"};
    const static std::vector<std::string> argsMult = {"SUM_IFS", "AVG_IFS"};
    // for(std::string p : parameters)
    //     if(p == _method) {isCorrectType = true; break;}
    // if(!isCorrectType) throw PARAMETER_OUT_OF_RANGE; 
    if(std::find(parameters.begin(), parameters.end(), _method) == parameters.end()) throw INVALID_METHOD;
    if(
        (std::find(args1.begin(), args1.end(), _method) != args1.end() && _args.size() != 1) || 
        (std::find(args2.begin(), args2.end(), _method) != args2.end() && _args.size() != 2) || 
        (std::find(argsMult.begin(), argsMult.end(), _method) != argsMult.end() && _args.size() < 3) 
    ) throw INVALID_NUMS_OF_ARGS;

    std::string meth_arg = "", table = "", where = "";

    if(_method == "COUNT") 
    {
        table = _args[0].substr(1, _args[0].length() - 2);
        meth_arg = "*";
    }
    else
    {
        if(_args[0].find("`.`") == std::string::npos) throw INVALID_ARGS;
        table = _args[0].substr(1, _args[0].find("`.`") - 1);
        meth_arg = _args[0].substr(_args[0].find("`.`") + 2, _args[0].length() - _args[0].find("`.`") - 2);
    }

    if(std::find(args2.begin(), args2.end(), _method) != args2.end())
    {
        _method = _method.substr(0, 3);

        where = "WHERE " + meth_arg + _args[1];
    }

    MYSQL sql;
    if(!devi::sql_start(&sql, "db_" + SINs[_sin].db)) throw MYSQL_CON_ERROR;

    MYSQL_RES* sql_response;
    MYSQL_ROW sql_row;

    if(mysql_query(&sql, ("SELECT " + _method + "(" + meth_arg + ") FROM `" + table +"` " + where + ";").c_str()) != 0)
    {
        std::string err = mysql_error(&sql);
        std::cout << "Q: " << "SELECT " + _method + "(" + meth_arg + ") FROM `" + table +"` " + where + ";" << " E: " << mysql_error(&sql) << std::endl;
        throw INVALID_QUERY;
    }
    sql_response = mysql_store_result(&sql);
    
    float res = atof((sql_row = mysql_fetch_row(sql_response))[0]);

    mysql_close(&sql);
    return res;
}

void devi::Sheet(crow::App<crow::CORSHandler>& app) 
{
    CROW_ROUTE(app, "/sheet/<int>/update")
    .methods(crow::HTTPMethod::POST)
    ([&](const crow::request& req, int _sheet){
        auto body = crow::json::load(req.body);
        if(!body) 
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid body\"}");

        std::string sin, coord, value;

        try
        {
            sin     = parseStr(body["sin"].s());
            coord   = parseStr(body["coord"].s());
            value   = body["value"].s();
        }
        catch(const std::runtime_error& e)
        {
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid body\"}");
        }

        if(!checkSIN(sin, req))
            return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Wrong SIN\"}");
        
        std::string sheet_s = "", line = "";
        std::ifstream file("files/sheet/db_" + SINs[sin].db + "/sheet" + std::to_string(_sheet) + ".json");
        if(!file.is_open()) 
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"This sheet not exist\"}");

        while(getline(file, line))
            sheet_s += line;

        if(sheet_s == "") sheet_s = "{}";
        auto sheetF = crow::json::load(sheet_s);
        file.close();
        if(!sheetF) 
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid sheet\"}");

        crow::json::wvalue response = sheetF;
        response[coord] = value;

        std::ofstream fileO("files/sheet/db_" + SINs[sin].db + "/sheet" + std::to_string(_sheet) + ".json");
        fileO << response.dump();
        fileO.close();
        //return crow::response(crow::OK, response);
        return crow::response(crow::OK);
        //return crow::response(crow::OK, std::to_string(aggregateFun(meth, "zamówienia", "cena(wKartoflach)", sin)));
    });

    CROW_ROUTE(app, "/sheet/<int>/get")
    .methods(crow::HTTPMethod::POST)
    ([&](const crow::request& req, int _sheet){
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
        
        std::string sheet_s = "", line = "";
        std::ifstream file("files/sheet/db_" + SINs[sin].db + "/sheet" + std::to_string(_sheet) + ".json");
        if(!file.is_open()) 
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"This sheet not exist\"}");

        while(getline(file, line))
            sheet_s += line;

        if(sheet_s == "") sheet_s = "{}";
        auto sheetF = crow::json::load(sheet_s);
        file.close();
        if(!sheetF) 
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid sheet\"}");

        crow::json::wvalue response;
        response["sheet"] = sheetF;

        // CELI(AVG(`tab`.`col`) + (MIN(`tab`.`col`)) / COUNT(`tab`)) + [!1A12]
        // CELI(AVG(`zamówienia`.`cena(wKartoflach)`) + (MIN(`zamówienia`.`cena(wKartoflach)`)) / COUNT(`zamówienia`)) + [!1A12]
        std::string parameters[] = {"SUM", "AVG", "MAX", "MIN", "COUNT"};
        std::map<std::string, float> func;
        for(auto s : sheetF)
        {
            std::string str = s.s();
            if(str[0] != '=') continue;

            std::string meth = "";

            for(int i = 1; i < str.length(); i++)
            {
                if(str[i] != '(') {meth += str[i]; continue;}

                bool isDBfunction = false;
                int poz;
                for(std::string p : parameters) 
                    if((poz = meth.find(p)) != std::string::npos)
                    {
                        isDBfunction = true;
                        break;
                    }
                if(!isDBfunction) {meth = ""; continue;}
                meth = meth.substr(poz);

                std::vector<std::string> args = {""};

                bool quote = false;
                for(++i; i < str.length(); i++)
                {
                    if(str[i] == ')') break;
                    if(str[i] == '`') 
                    {
                        for(i; i < str.length(); i++)
                        {
                            if(str[i] == '`') quote = !quote;
                            else if(str[i] == ')' && !quote) 
                            {
                                i--;
                                break;
                            }
                            if(str[i] == ',' && !quote)
                            {
                                args.push_back("");
                                break;
                            }
                            if(str[i] == ' ' && !quote) continue;
                            args.back() += str[i];
                        }
                    }
                    else if(str[i] == '"') 
                    {
                        for(i; i < str.length(); i++)
                        {
                            if(str[i] == '"') quote = !quote;
                            else if(str[i] == ')' && !quote)
                            {
                                i--;
                                break;
                            }
                            if(str[i] == ',' && !quote)
                            {
                                args.push_back("");
                                break;
                            }
                            args.back() += str[i];
                        }
                    }
                    else if(str[i] == ',') args.push_back("");
                    else if(str[i] == ' ') continue;
                    else args.back() += str[i];
                }

                std::string args_str = "";
                for (std::string a : args) args_str += a + ",";
                if(args_str[args_str.length() - 1] == ',') args_str = args_str.substr(0, args_str.length() - 1);
                //std::cout << args_str << std::endl;

                if(func.find(meth + "|" + args_str) != func.end()) {meth = ""; continue;}
                float value = 0;

                try
                {
                    value = aggregateFun(meth, args, sin);
                }
                catch(const int& e)
                {
                    //std::cout << meth + "|" + args_str << " : " << e << std::endl;
                    meth = "";
                    continue;
                }
                //std::cout << "COMPLET" << std::endl;
                
                func[meth + "|" + args_str] = value;
                response["data"][meth + "(" + args_str + ")"] = value;
                
                meth = "";
            }

        }

        return crow::response(crow::OK, response);
    });
}