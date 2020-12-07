#include "Function.h"
#include "mystring.h"
#include "ida.h"
#include <sstream>

Function Function::getFunction(unsigned int functionAddress, std::map<unsigned int, unsigned int> virtualFunctions)
{
    Function entry;
    tinfo_t type;

    entry.address = getHexAddress(functionAddress);
    get_tinfo(&type, functionAddress);

    entry.name = getFunctionName(functionAddress).c_str();

    qstring functionType;
    type.print(&functionType);
    entry.type = functionType.c_str();

    if (isFunctionPrefixReserved(entry.name.c_str()))
    {
        entry.name.clear();
    }
    else
    {
        entry.demangledName = getDemangledFunctionName(functionAddress, entry.name.c_str());
    }

    entry.callingConvention = getFunctionCallingConvention(type);

    qstring returnType;

    type.get_rettype().print(&returnType);
    entry.returnType = returnType.c_str();

    if (entry.returnType.length() > 0)
    {
        entry.prototype = entry.returnType + " " + getFunctionPrototype(functionAddress, entry.name.c_str());
    }
    else
    {
        entry.prototype = getFunctionPrototype(functionAddress, entry.name.c_str());
    }

    entry.parameters = getFunctionParameters(type);

    auto it = virtualFunctions.find(functionAddress);

    if (it != virtualFunctions.end())
    {
        entry.vtableIndex = it->second;
    }

    return entry;
}

std::string Function::getFunctionPrototype(unsigned int functionAddress, qstring functionName)
{
    qstring prototype;

    get_short_name(&prototype, functionAddress);
    qstring tmpdem = prototype;
    tmpdem.replace("__", "::");

    if (functionName == tmpdem)
    {
        prototype = functionName;
    }

    return prototype.c_str();
}

std::string Function::getHexAddress(unsigned int functionAddress)
{
    std::stringstream stream;
    stream << std::uppercase << std::hex << functionAddress;

    std::string hexAddress = stream.str().c_str();

    return hexAddress;
}

std::string Function::getFunctionCallingConvention(tinfo_t type)
{
    std::string callingConvention;

    switch (type.get_cc())
    {
    case CM_CC_INVALID:
        callingConvention = "";
        break;
    case CM_CC_VOIDARG:
        callingConvention = "voidarg";
        break;
    case CM_CC_CDECL:
        callingConvention = "cdecl";
        break;
    case CM_CC_ELLIPSIS:
        callingConvention = "ellipsis";
        break;
    case CM_CC_STDCALL:
        callingConvention = "stdcall";
        break;
    case CM_CC_PASCAL:
        callingConvention = "pascal";
        break;
    case CM_CC_FASTCALL:
        callingConvention = "fastcall";
        break;
    case CM_CC_THISCALL:
        callingConvention = "thiscall";
        break;
    case CM_CC_MANUAL:
        callingConvention = "manual";
        break;
    case CM_CC_SPOILED:
        callingConvention = "spoiled";
        break;
    default:
        callingConvention = "unknown";
        break;
    }

    return callingConvention;
}

std::string Function::getDemangledFunctionName(unsigned int functionAddress, qstring functionName)
{
    qstring demangledName;

    get_short_name(&demangledName, functionAddress);
    qstring tmpdem = demangledName;
    tmpdem.replace("__", "::");

    if (functionName == tmpdem)
    {
        demangledName = functionName;
    }

    std::string demangledName2 = demangledName.c_str();
    int index = demangledName2.find_last_of("::");
    int startIndex, count;

    if (demangledName2.find("::`") != std::string::npos)
    {
        startIndex = index + 2;
    }
    else
    {
        startIndex = index + 1;
    }

    if (demangledName2.find("'(") != std::string::npos)
    {
        count = demangledName2.find('(', index) - startIndex - 1;
    }
    else if (demangledName2.find("' (") != std::string::npos)
    {
        count = demangledName2.find('(', index) - startIndex - 2;
    }
    else
    {
        count = demangledName2.find('(', index) - startIndex;
    }

    demangledName2 = demangledName2.substr(startIndex, count);
    demangledName = demangledName2.c_str();

    return demangledName.c_str();
}

std::vector<Function::Parameter> Function::getFunctionParameters(tinfo_t type)
{
    func_type_data_t fi;
    std::vector<Function::Parameter> parameters;

    if (type.get_func_details(&fi))
    {
        for (auto const& p : fi)
        {
            Function::Parameter funcParam;
            funcParam.name = p.name.c_str();

            qstring type;

            p.type.print(&type);
            funcParam.type = type.c_str();

            parameters.push_back(funcParam);
        }
    }

    return parameters;
}
