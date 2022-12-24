#include "sql.hpp"
#include "../settings.h"
#include "iostream"
#include <algorithm>

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

bool devi::exec_NOquery(MYSQL* sql,std::initializer_list<std::string> _commnd,  bool rollback, bool closeFlag)
{
    std::string command;
    for (std::string part : _commnd)
        command += part;

    if(mysql_query(sql, command.c_str()) != 0) 
    {
        if(rollback)
            mysql_query(sql, "ROLLBACK;");

        #if DEBUG_MODE == true
            std::cout << command << std::endl << mysql_error(sql) << std::endl;
        #endif

        if(closeFlag)
            mysql_close(sql);
        return false;
    }
    return true;
}

std::string devi::parseStr(std::string _str)
{
    std::string result;
    std::string illegalChars = "/\\'\";";
    //std::cout << illegalChars.length() << " - " << illegalChars << std::endl;
    for(int i = 0; i < _str.length(); i++)
    {
        if(illegalChars.find(_str[i]) != std::string::npos)
            result += '\\';
        result += _str[i];
    }
    return result;
}