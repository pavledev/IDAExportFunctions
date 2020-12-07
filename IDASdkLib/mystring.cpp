#include "mystring.h"
#include "idp.hpp"

bool startsWith(qstring const& strToCheck, qstring const& strStart)
{
    return strToCheck.substr(0, strStart.length()) == strStart;
}

bool endsWith(qstring const& strToCheck, qstring const& strEnd)
{
    auto strLen = strToCheck.length();
    auto endLen = strEnd.length();

    if (endLen > strLen)
    {
        return false;
    }

    return strToCheck.substr(strLen - endLen) == strEnd;
}

bool writeJSONToFile(nlohmann::ordered_json const& json, char const* filepath)
{
    auto outFile = qfopen(filepath, "wt");

    if (outFile)
    {
        bool result = true;

        try
        {
            qstring jsonStr = json.dump(4).c_str();
            qfputs(jsonStr.c_str(), outFile);
        }
        catch (std::exception & e)
        {
            warning("Unable to write json data to file\n%s\n%s", filepath, e.what());
            result = false;
        }

        qfclose(outFile);

        return result;
    }

    warning("Unable to open '%s'", filepath);

    return false;
}

bool isNumber(qstring const& str)
{
    if (str.empty())
    {
        return false;
    }

    for (size_t i = 0; i < str.length(); i++)
    {
        if (!isdigit(str[i]))
        {
            return false;
        }
    }

    return true;
}

bool contains(qstring const& str, qstring const& substr)
{
    return str.find(substr) != qstring::npos;
}

qstring toUpper(qstring const& str)
{
    qstring result;

    for (size_t i = 0; i < str.length(); i++)
    {
        result += toupper(static_cast<unsigned char>(str[i]));
    }

    return result;
}

qstring toLower(qstring const& str)
{
    qstring result;

    for (size_t i = 0; i < str.length(); i++)
    {
        result += tolower(static_cast<unsigned char>(str[i]));
    }

    return result;
}

std::vector<qstring> splitqstr(qstring& str, qstring& delimiter)
{
    size_t pos = 0;
    std::vector<qstring> result;
    qstring str2 = str;

    while ((pos = str2.find(delimiter)) != qstring::npos)
    {
        result.push_back(str2.substr(0, pos));
        str2.remove(0, pos + delimiter.length());
    }

    return result;
}

std::vector<std::string> splitstr(qstring& str, qstring& delimiter)
{
    size_t pos = 0;
    std::vector<std::string> result;
    qstring str2 = str;

    while ((pos = str2.find(delimiter)) != qstring::npos)
    {
        result.push_back(str2.substr(0, pos).c_str());
        str2.remove(0, pos + delimiter.length());
    }

    return result;
}