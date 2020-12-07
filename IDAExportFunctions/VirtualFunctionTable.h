#pragma once
#include <vector>
#include "Function.h"

using namespace std;

class VirtualFunctionTable
{
public:
	string forClass;
	vector<Function> virtualFunctions;
};
