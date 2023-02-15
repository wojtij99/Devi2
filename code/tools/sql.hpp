#pragma once
#include <mysql/mysql.h>
#include "../settings.h"
#include "iostream"

namespace devi
{
    bool sql_start(MYSQL* sql, std::string db_name = SQL_NAME, std::string user = SQL_USER, std::string pass = SQL_PASS);    
    bool exec_NOquery(MYSQL* sql,std::initializer_list<std::string> _command, bool rollback = false, bool closeFlag = true, std::string* _error = nullptr);
    std::string parseStr(std::string _str);
}
