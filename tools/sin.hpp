#pragma once
#include <crow.h>
#include <map>
#include <crow/middlewares/cors.h>

namespace devi
{
    struct dateTime_t
    {
        int   year;
        short month;
        short day;
        short hour;
        short minute;
        short second;
    };

    struct sin_t //Securyty Indentified Number
    {
        std::string ip;
        std::string user_agent;
        std::string user;
        std::string db;
        dateTime_t  creationDT;
        dateTime_t  expiredate;
    };

    void SIN(crow::App<crow::CORSHandler>& app);
    bool checkSIN(std::string SIN, const crow::request& req);
    //std::string encrypt(std::string ip, std::string user_agent, std::string user, std::string db);
    //std::string decrypt(std::string sin);
}

inline std::map<std::string, devi::sin_t> SINs;