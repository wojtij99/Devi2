#include "tools.hpp"

bool devi::isSystemTable(std::string _table)
{
    if (_table.rfind("system_", 0) != 0 && _table.rfind("log_",0) != 0) return false;
    return true;
}

void devi::getURL_param(const crow::request& _req, std::string _key, std::function<void(char*)> _succes)
{
    char* value = _req.url_params.get(_key);
    if(value == nullptr) throw 1;
    else _succes(value);
}

bool devi::is_number(const std::string& s)
{
    return !s.empty() && std::find_if(s.begin(), s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

std::string devi::urlDecode(std::string SRC) 
{
    std::string ret;
    char ch;
    int i, ii;
    for (i=0; i<SRC.length(); i++) {
        if (SRC[i]=='%') {
            sscanf(SRC.substr(i+1,2).c_str(), "%x", &ii);
            ch=static_cast<char>(ii);
            ret+=ch;
            i=i+2;
        } 
        else ret+=SRC[i];
    }
    return (ret);
}

std::vector<std::string> devi::split(std::string _str, std::string _separator)
{
    std::vector<std::string> result;

    int actual = 0;
    while((actual = _str.find(_separator)) != std::string::npos)
    {
        result.emplace_back(_str.substr(0, actual));
        _str = _str.substr(actual + 1);
    }
    result.emplace_back(_str);

    return result;
}

std::string devi::toUpper(std::string _str)
{
    std::string result;

    for(char c : _str)
        result += std::toupper(c);

    return result;
}

std::string devi::toLower(std::string _str)
{
    std::string result;

    for(char c : _str)
        result += std::tolower(c);

    return result;
}

std::string devi::serialize(std::string _str, char _c)
{
    std::string result = "";

    for(char c : _str)
    {
        if(c == _c) result += '\\';
        result += c;
    }

    return result;
}

std::vector<std::string> devi::getJSONList(crow::json::rvalue json)
{
    if (json.t() != crow::json::type::Object && json.t() != crow::json::type::List)
        throw std::runtime_error("value is not a container");

    std::vector<std::string> result;

    for(auto j : json)
        result.emplace_back(j.s());

    return result;
}