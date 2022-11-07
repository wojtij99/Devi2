#include "sql.hpp"
#include "../settings.h"
#include "iostream"

bool devi::sql_start(MYSQL* sql)
{
    mysql_init(sql);

    if(!mysql_real_connect(sql, SQL_HOST, SQL_USER, SQL_PASS, SQL_NAME, SQL_PORT, NULL, 0))
    {
        printf("Błąd połączenia z bazą MySQL: %d, %s\n", mysql_errno(sql), mysql_error(sql));
        return false;
    }

    return true;
}