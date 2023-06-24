#include "permissions.hpp"
#include "sql.hpp"
#include "tools.hpp"

std::map<permStruct_e, std::string> struct_m{
    {permStruct_e::Table, "tables"},
    {permStruct_e::Sheet, "sheets"},
    {permStruct_e::Global, "global"},
    {permStruct_e::Page, "pages"}};
std::map<permType_e, std::string> type_m{
    {permType_e::ALL, "ALL"},
    {permType_e::ACCOUNT, "ACCOUNT"},
    {permType_e::MODIFY, "MODIFY"},
    {permType_e::DELETE, "DELETE"},
    {permType_e::UPADTE, "UPADTE"},
    {permType_e::READ, "READ"},
    {permType_e::NONE, "NONE"}};

int getId(std::string sin, permStruct_e pStruct, std::string name)
{
    if(pStruct == permStruct_e::Global) throw 1;
    MYSQL sql;
    if(!devi::sql_start(&sql, "db_" + SINs[sin].db)) 
        throw 0;

    MYSQL_RES* sql_response;
    MYSQL_ROW sql_row;
    int result = -1;

    mysql_query(&sql, std::string("SELECT `ID` FROM `system_" + struct_m[pStruct] + "` WHERE `name` = " + name + ";").c_str());
    sql_response = mysql_store_result(&sql);

    while ((sql_row = mysql_fetch_row(sql_response)) != NULL)
    {
        result = std::stoi(sql_row[0]);
    }

    mysql_close(&sql);
    return result;
}

bool devi::havePermissions(std::string sin, permStruct_e pStruct, std::string name, permType_e pType)
{
    crow::json::rvalue per = getPermissions(sin);

    try
    {
        std::vector<std::string> listG = getJSONList(per["global"]);
        if(std::find(listG.begin(), listG.end(), "ALL") != listG.end()) return true;
        if(std::find(listG.begin(), listG.end(), type_m[pType]) != listG.end()) return true;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }

    try
    {
        std::vector<std::string> listS = getJSONList(per[struct_m[pStruct]][getId(sin, pStruct, name)]);
        if(std::find(listS.begin(), listS.end(), "ALL") != listS.end()) return true;
        if(std::find(listS.begin(), listS.end(), type_m[pType]) != listS.end()) return true;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }

    return false;
}

crow::json::rvalue devi::getPermissions(std::string sin)
{
    MYSQL sql;
    if(!devi::sql_start(&sql, "db_" + SINs[sin].db)) 
        throw 1;

    MYSQL_RES* sql_response;
    MYSQL_ROW sql_row;
    crow::json::rvalue result;

    mysql_query(&sql, std::string("SELECT `permissions` FROM `system_users` WHERE `ID` = " + std::to_string(SINs[sin].id) + ";").c_str());
    sql_response = mysql_store_result(&sql);

    while ((sql_row = mysql_fetch_row(sql_response)) != NULL)
    {
        result = crow::json::load(sql_row[0]);
    }

    mysql_close(&sql);
    return result;
}

void devi::setPermissions(std::string sin, permStruct_e pStruct, std::string name, permType_e pType)
{
    //TODO: Dodawanie uprawnień konkretnemu userowi 
    if(pStruct > permStruct_e::Global) throw 3;
    if(pType > permType_e::NONE) throw 4;
    crow::json::rvalue per = getPermissions(sin);
    crow::json::wvalue resp = per;

    std::vector<std::string> listG;
    int id;
    if(pStruct == permStruct_e::Global) listG = getJSONList(per["global"]);
    else
    {
        id = getId(sin, pStruct, name);
        if(id == -1) throw 5;
        listG = getJSONList(per[struct_m[pStruct][id]]);
    }

    if(std::find(listG.begin(), listG.end(), "ALL") == listG.end() &&
       std::find(listG.begin(), listG.end(), type_m[pType]) == listG.end())
    {
        if(std::find(listG.begin(), listG.end(), "NONE") != listG.end()) listG = {};
        listG.emplace_back(type_m[pType]);
        if(pStruct == permStruct_e::Global) resp["global"] = listG;
        else resp[struct_m[pStruct][id]] = listG;;
    }

    MYSQL sql;
    if(!devi::sql_start(&sql, "db_" + SINs[sin].db)) 
        throw 1;

    if(!exec_NOquery(&sql, {"UPDATE `system_users` SET `permissions`='", resp.dump() ,"' WHERE `ID` = ", std::to_string(SINs[sin].id) ," ;"}))
        throw 2;
}