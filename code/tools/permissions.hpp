#pragma once
#include "sin.hpp"

enum permStruct_e
{
    Table,
    Sheet,
    Page,
    Global
};

enum permType_e
{
    ALL,
    ACCOUNT,
    MODIFY,
    DELETE,
    UPADTE,
    READ,
    NONE
};

namespace devi
{
    bool havePermissions(std::string sin, permStruct_e pStruct, std::string name, permType_e pType);
    crow::json::rvalue getPermissions(std::string sin);
    void setPermissions(std::string sin, permStruct_e pStruct, std::string name, permType_e pType);
    void remPermissions(std::string sin, permStruct_e pStruct, std::string name, permType_e pType);

    void Permissions(crow::App<crow::CORSHandler>& app);
}
