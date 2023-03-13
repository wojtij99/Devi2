#pragma once
#include <crow.h>
#include <map>
#include <crow/middlewares/cors.h>

#define EXPIRE_TIME 5*60

namespace devi
{
    struct sin_t //Securyty Indentified Number
    {
        std::string ip;
        std::string user_agent;
        std::string user;
        std::string db;
        time_t  creationDT;
        time_t  expiredate;
    };

    void SIN(crow::App<crow::CORSHandler>& app);
    bool checkSIN(std::string SIN, const crow::request& req);
}

inline std::map<std::string, devi::sin_t> SINs;