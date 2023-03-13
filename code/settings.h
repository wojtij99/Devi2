#define DEBUG_MODE true
#define SERVER_MODE false

#define SQL_HOST "127.0.0.1"
#define SQL_NAME "mainDB"
#if SERVER_MODE == true
    #define SQL_USER "admin"
    #define SQL_PASS "B@nanaSQL"
    #define SQL_PORT 3306
#else
    #define SQL_USER "root"
    #define SQL_PASS ""
    #define SQL_PORT 3306
#endif

#define API_PORT 3001