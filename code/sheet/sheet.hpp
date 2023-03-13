#pragma once
#include <crow.h>
#include <crow/middlewares/cors.h> 

namespace devi
{
    void Sheet(crow::App<crow::CORSHandler>& app);
}
