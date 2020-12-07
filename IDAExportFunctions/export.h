#pragma once
#include <filesystem>
#include "ida.hpp"
#include "idp.hpp"
#include "typeinf.hpp"
#include "enum.hpp"
#include "struct.hpp"
#include "bytes.hpp"
#include "name.hpp"
#include "mystring.h"
#include "range.h"
#include "ida.h"
#include <map>
#include "Type.h"
#include "VTClassInfo.h"

using namespace std::filesystem;

void addInheritedClasses(vector<Type>& classes, qvector<VTClassInfo> vtables);
void addNonVirtualFunctions(vector<Type>& classes, map<unsigned int, unsigned int> virtualFuncs);
nlohmann::ordered_json createJSONFromData(vector<Type> classes);
void exportdb(path const& output);
