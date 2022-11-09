#pragma once
#include <crow.h>

namespace devi
{
    void SIN(crow::SimpleApp& app);
    std::string encrypt();
    void decrypt(std::string sin);
}