#pragma once
#include <crow.h>
#include <crow/middlewares/cors.h>

namespace devi
{
    void Tables(crow::App<crow::CORSHandler>& app);
}