#pragma once
#include <crow.h>
#include <crow/middlewares/cors.h>//

namespace devi
{
    void Search(crow::App<crow::CORSHandler>& app);
    
}