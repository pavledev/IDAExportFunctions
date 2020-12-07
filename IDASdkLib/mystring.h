#pragma once
#include "ida.hpp"
#include "idp.hpp"

#ifdef snprintf
#undef snprintf
#endif
#ifdef strtoull
#undef strtoull
#endif

#include "../shared/nlohmann/json.hpp"

bool startsWith(qstring const& strToCheck, qstring const& strStart);
bool endsWith(qstring const& strToCheck, qstring const& strEnd);
bool contains(qstring const& str, qstring const& substr);

bool isNumber(qstring const& str);

qstring toUpper(qstring const& str);
qstring toLower(qstring const& str);

std::vector<qstring> splitqstr(qstring& str, qstring& delimiter);
std::vector<std::string> splitstr(qstring& str, qstring& delimiter);

bool writeJSONToFile(nlohmann::ordered_json const& json, char const* filepath);