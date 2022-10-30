#pragma once
#include <crow.h>
#include <mysql/mysql.h>

namespace devi
{
    void Admin(crow::SimpleApp& app, MYSQL& sql);
} // namespace devi

