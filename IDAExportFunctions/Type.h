#pragma once
#include <vector>
#include <string>
#include "VirtualFunctionTable.h"

using namespace std;

class Type
{
public:
	string className;
	vector<string> inheritedClasses;
	vector<VirtualFunctionTable> virtualFunctionTables;
	vector<Function> nonVirtualFunctions;
};

