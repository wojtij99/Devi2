#pragma once
#include <crow.h>
#include <crow/middlewares/cors.h>

namespace devi
{
    void Client(crow::App<crow::CORSHandler>& app);
}