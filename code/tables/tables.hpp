#pragma once
#include <crow.h>
#include <crow/middlewares/cors.h> 

#define ENABLE_SELECT_ONE false

namespace devi
{
    void Tables(crow::App<crow::CORSHandler>& app);
}