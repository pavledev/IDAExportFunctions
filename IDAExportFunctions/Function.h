#pragma once
#include "idp.hpp"
#include "mystring.h"

class Function
{
public:
    class Parameter
    {
    public:
        std::string name;
        std::string type;
    };

    std::string prototype;
    std::string address;
    std::string name;
    std::string demangledName;
    std::string type;
    std::string returnType;
    std::string callingConvention;
    std::vector<Parameter> parameters;
    int vtableIndex = -1;

    static Function getFunction(unsigned int functionAddress, std::map<unsigned int, unsigned int> virtualFunctions);
    static std::string getFunctionPrototype(unsigned int functionAddress, qstring functionName);
    static std::string getHexAddress(unsigned int functionAddress);
    static std::string getFunctionCallingConvention(tinfo_t type);
    static std::string getDemangledFunctionName(unsigned int functionAddress, qstring functionName);
    static std::vector<Function::Parameter> getFunctionParameters(tinfo_t type);
};
