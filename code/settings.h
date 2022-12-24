#define DEBUG_MODE true

#define SQL_HOST "127.0.0.1"
#define SQL_NAME "mainDB"
#if DEBUG_MODE == false
    #define SQL_USER "API_MySQLuser"
    #define SQL_PASS "jSmmwERr4i#G^O2vhP3J*pBayW%eFlZ7"
    #define SQL_PORT 3306
#else
    #define SQL_USER "root"
    #define SQL_PASS ""
    #define SQL_PORT 3306
#endif

#define API_PORT 3001