#include "search.hpp"
#include "../tools/tools.hpp"

void devi::Search(crow::App<crow::CORSHandler>& app)
{
    CROW_ROUTE(app, "/search")
    .methods(crow::HTTPMethod::POST)
    ([&](const crow::request& req){
        return "";
    });
}