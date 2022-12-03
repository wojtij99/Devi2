#include "sin.hpp"
//#include <boost/format.hpp>
#include <mysql/mysql.h>
#include "../tools/sql.hpp"

const std::string currentDateTime() 
{
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

    return buf;
}

std::vector<std::string> split(std::string _string, char _delimiter)
{
    std::vector<std::string> result;
    std::string temp = "";

    for(char c : _string)
    {
        if(c == _delimiter)
        {
            result.push_back(temp);
            temp = "";
            continue;
        }
        temp += c;
    }
    result.push_back(temp);

    return result;
}

/*devi::dateTime_t convertDTstr2t(std::string _string)
{
    std::vector<std::string> temp = split(_string, '.');
    std::vector<std::string> date = split(temp[0], '-');
    std::vector<std::string> time = split(temp[1], ':');

    devi::dateTime_t result =
    {
        std::stoi(date[0]),
        std::stoi(date[1]),
        std::stoi(date[2]),
        std::stoi(time[0]),
        std::stoi(time[1]),
        std::stoi(time[2])
    };
    return result;
}*/

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
        std::cout << req.remote_ip_address << std::endl;

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


        for(auto _s : SINs)
        {
            if(_s.second.expiredate - time(NULL) < 0)
            {
                SINs.erase(SINs.find(_s.first));
                if(_s.second.user == user && _s.second.db == db)
                    return crow::response(crow::GONE, "{\"response\":\"Your SIN is expired\"}");
            }
            else if(_s.second.user == user && _s.second.db == db)
                return crow::response(crow::OK, "{\"sin\": \"" + _s.first +"\"}");
        }

        
        /*for(auto _s : SINs)
        {
            if(_s.second.user == user && _s.second.db == db)
            {
                if(_s.second.expiredate - time(NULL) < 0)
                {
                    SINs.erase(SINs.find(_s.first));
                    return crow::response(crow::GONE, "{\"response\":\"Your SIN is expired\"}");
                }
                return crow::response(crow::OK, "{\"sin\": \"" + _s.first +"\"}");
            }
        }*/

        try
        {
            user_agent = req.get_header_value("User-Agent");
            //host = req.get_header_value("Host");
        }
        catch(const std::runtime_error& e)
        {
            return crow::response(crow::BAD_REQUEST, "{\"response\":\"Invalid haeder\"}");
        }

        MYSQL sql;
        if(!devi::sql_start(&sql, "db_" + db)) return crow::response(crow::SERVICE_UNAVAILABLE, "{\"response\":\"Can't connect to DB'}");

        MYSQL_RES* sql_response;
        MYSQL_ROW sql_row;
        std::stringstream command;
        command << "SELECT id FROM `system_users` WHERE `name` = '" << user << "' AND `pass` = '" << pass << "' ;";

        std::cout << command.str() << std::endl;
        mysql_query(&sql, command.str().c_str());
        sql_response = mysql_store_result(&sql);

        if ((sql_row = mysql_fetch_row(sql_response)) == NULL)
        {
            mysql_close(&sql);
            return crow::response(crow::UNAUTHORIZED, "{\"response\":\"Invalid user or password\"}");
        }

        std::string sin;
        do
        {
            sin = generateSIN();
        } while (SINs.find(sin) != SINs.end());
        

        sin_t sin_struct = {host, user_agent, user, db, time(NULL), time(NULL) + 5 * 60};

        SINs[sin] = sin_struct;

        //std::cout << host << " - " << user_agent << std::endl;
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
}

bool devi::checkSIN(std::string _sin, const crow::request& req)
{
    if(SINs.find(_sin) == SINs.end()) return false;

    std::string user_agent, host = req.remote_ip_address;
    try
    {
        user_agent = req.get_header_value("User-Agent");
        //host = req.get_header_value("Host");
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

    SINs[_sin].expiredate = time(NULL) + 5 * 60;
    return true;
}

/*
std::string devi::encrypt(std::string ip, std::string user_agent, std::string user, std::string db)
{
    std::string stage0 = ip + "|" + user_agent + "|" + user + "|" + db + "|" + currentDateTime();
    
    std::string stage1 = "";

    for(char c : stage0)
    {
        std::string temp = (boost::format("%x") % (int)c).str();
        //if(temp.length() == 2) 
            ///temp = "00" + temp;
        stage1 += temp;
    }
    srand(time(NULL));
    char key[] = { '0','1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

    std::string stage2;
    for(char c : stage1)
    {
        stage2 += c;
        stage2 += key[rand() % 16];
    }

    std::string stage3 = "";
    for(char c : stage2)
        stage3 = c + stage3;

    return stage3;
}

std::string devi::decrypt(std::string sin)
{
    std::string stage0 = "";
    for(char c : sin)
        stage0 = c + stage0;
    
    std::string stage1 = "";
    bool cut = false;

    for(char c : stage0)
    {
        if(!cut) stage1 += c;
        cut = !cut;
    }

    std::cout << stage1 << std::endl;

    std::string stage2 = "";
    for(int i=0; i< stage1.length(); i+=2)
    {
        std::string byte = stage1.substr(i,2);
        char chr = (char) (int)strtol(byte.c_str(), NULL, 16);
        stage2.push_back(chr);
    }
    return stage2;
}
*/