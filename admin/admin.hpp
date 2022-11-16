#pragma once
#include <crow.h>
#include <crow/middlewares/cors.h>

namespace devi
{
    void Admin(crow::App<crow::CORSHandler>& app);
} // namespace devi

