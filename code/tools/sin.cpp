#include "sin.hpp"
#include "../tools/sql.hpp"
#include "../tools/sha1.hpp"

std::string generateSIN()
{
    std::string result = "";
    srand(time(NULL));
    std::string codes = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    for (int i = 0; i < 256; i++)
        result += codes[rand() % codes.length()];
    return result;
}

void devi::SIN(crow::App<crow::CORSHandler>& app)
{
    CROW_ROUTE(app, "/getSIN")
    .methods(crow::HTTPMethod::POST)
    ([](const crow::request& req){
        auto body = crow::json::load(req.body);

        if(!body) 
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid body\"}");

        std::string user, pass, db, user_agent, host = req.remote_ip_address;

        try
        {
            user    = parseStr(body["user"].s());
            pass    = parseStr(body["pass"].s());
            db      = parseStr(body["db"].s());
        }
        catch(const std::runtime_error& e)
        {
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid body\"}");
        }

        if(user.find('\'') != std::string::npos) return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Invalid character (') in user\"}");
        if(db.find('\'') != std::string::npos) return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Invalid character (') in DB\"}");

        try
        {
            user_agent = req.get_header_value("User-Agent");
        }
        catch(const std::runtime_error& e)
        {
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid haeder\"}");
        }

        std::vector<std::map<std::string, devi::sin_t>::iterator> toErase;
        std::pair<crow::status, std::string> SINresponse = {crow::status::CONTINUE, ""};
        for(auto _s : SINs)
        {
            if(_s.second.expiredate - time(NULL) < 0)
            {
                toErase.push_back(SINs.find(_s.first));
                if(_s.second.user == user && _s.second.db == db) 
                    SINresponse = {crow::GONE, "{\"response\":\"Your SIN is expired\"}"} ;
                    //return crow::response(crow::GONE, "{\"response\":\"Your SIN is expired\"}");
            }
            else if(_s.second.user == user && _s.second.db == db)
                if(_s.second.ip != host || _s.second.user_agent != user_agent) toErase.push_back(SINs.find(_s.first));
                else SINresponse = {crow::OK, "{\"sin\": \"" + _s.first +"\"}"};
                //return crow::response(crow::OK, "{\"sin\": \"" + _s.first +"\"}");
        }

        for(auto e : toErase) SINs.erase(e);
        if(SINresponse.second != "") return crow::response(SINresponse.first, SINresponse.second);

        SHA1 checksum;
        checksum.update(pass);
        MYSQL sql;
        if(!devi::sql_start(&sql, "db_" + db)) return crow::response(crow::SERVICE_UNAVAILABLE, "{\"response\":\"Can't connect to DB'}");
        MYSQL_RES* sql_response;
        MYSQL_ROW sql_row;
        std::stringstream command;
        command << "SELECT id FROM `system_users` WHERE `name` = '" << user << "' AND `pass` = '" << checksum.final() << "' ;";

        mysql_query(&sql, command.str().c_str());
        sql_response = mysql_store_result(&sql);

        if ((sql_row = mysql_fetch_row(sql_response)) == NULL)
        {
            mysql_close(&sql);
            return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Invalid user or password\"}");
        }

        std::string sin;
        do sin = generateSIN();
        while (SINs.find(sin) != SINs.end());
        
        sin_t sin_struct = {std::stoi(sql_row[0]), host, user_agent, user, db, time(NULL), time(NULL) + EXPIRE_TIME};

        SINs[sin] = sin_struct;
        return crow::response(crow::OK, "{\"sin\": \"" + sin +"\"}");
    }); 

    CROW_ROUTE(app, "/dropSIN")
    .methods(crow::HTTPMethod::DELETE)
    ([](const crow::request& req){
        auto body = crow::json::load(req.body);
        if(!body) 
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid body\"}");

        std::string sin;

        try
        {
            sin    = parseStr(body["sin"].s());
        }
        catch(const std::runtime_error& e)
        {
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid body\"}");
        }

        if(!checkSIN(sin, req))
            return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Wrong SIN\"}");

        SINs.erase(SINs.find(sin));
        return crow::response(crow::OK);
    });

#if DEBUG_MODE == true
    CROW_ROUTE(app, "/logSIN")
    .methods(crow::HTTPMethod::GET)
    ([](const crow::request& req){
        crow::json::wvalue res;
        for(auto s : SINs)
        {
            res[s.first]["ip"] = s.second.ip;
            res[s.first]["user_agent"] = s.second.user_agent;
            res[s.first]["user"] = s.second.user;
            res[s.first]["db"] = s.second.db;
            res[s.first]["creationDT"] = s.second.creationDT;
            res[s.first]["expiredate"] = s.second.expiredate;
        }
        res["time"] = time(NULL);
        return crow::response(crow::OK, res);
    });
#endif
}

bool devi::checkSIN(std::string _sin, const crow::request& req)
{
    if(SINs.find(_sin) == SINs.end()) return false;

    std::string user_agent, host = req.remote_ip_address;
    try
    {
        user_agent = req.get_header_value("User-Agent");
    }
    catch(const std::runtime_error& e)
    {
        return false;
    }

    if(SINs[_sin].ip != host || SINs[_sin].user_agent != user_agent) return false;

    if(SINs[_sin].expiredate - time(NULL) < 0)
    {
        SINs.erase(SINs.find(_sin));
        return false;
    }

    SINs[_sin].expiredate = time(NULL) + EXPIRE_TIME;
    return true;
}