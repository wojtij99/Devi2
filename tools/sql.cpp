#include "sql.hpp"
#include "../settings.h"
#include "iostream"

bool devi::sql_start(MYSQL* sql, std::string db_name, std::string user, std::string pass)
{
    mysql_init(sql);

    if(!mysql_real_connect(sql, SQL_HOST, user.c_str(), pass.c_str(), db_name.c_str(), SQL_PORT, NULL, 0))
    {
        printf("Błąd połączenia z bazą MySQL: %d, %s\n", mysql_errno(sql), mysql_error(sql));
        return false;
    }

    return true;
}

bool devi::exec_NOquery(MYSQL* sql,std::initializer_list<std::string> _commnd, bool closeFlag)
{
    std::string command;
    for (std::string part : _commnd)
        command += part;

    if(mysql_query(sql, command.c_str()) != 0) 
    {
        if(closeFlag)
            mysql_close(sql);
        return false;
    }
    return true;
}