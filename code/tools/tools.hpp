#pragma once
#include <crow.h>

namespace devi
{
    bool isSystemTable(std::string _table);
    void getURL_param(const crow::request& _req, std::string _key, std::function<void(char*)> _succes);
    bool is_number(const std::string& s);
    std::string urlDecode(std::string SRC);
    std::vector<std::string> split(std::string _str, std::string _separator);
    std::string toUpper(std::string _str);
    std::string toLower(std::string _str);
    std::string serialize(std::string _str, char _c);
}