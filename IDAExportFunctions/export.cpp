#pragma once
#include "export.h"

using namespace std;

void exportdb(path const& output)
{
    msg("--------------------\nExport started\n--------------------\n");

    path dbFolderPath = output / "database";

    if (!exists(dbFolderPath))
    {
        error_code errCode;
        create_directories(dbFolderPath, errCode);

        if (errCode)
        {
            warning("Unable to create database folder (%s):\n%s", dbFolderPath.string().c_str(), errCode.message().c_str());
            return;
        }
    }

    vector<Type> classes;
    qvector<VTClassInfo> vtables;
    map<unsigned int, unsigned int> virtualFuncs;

    auto seg = get_first_seg();

    while (seg)
    {
        qstring segName;

        get_segm_name(&segName, seg);

        if (isDataSegment(segName))
        {
            msg("Scanning segment %s: (0x%X;0x%X)\n", segName.c_str(), seg->start_ea, seg->end_ea);
            auto ea = seg->start_ea;

            while (ea < seg->end_ea)
            {
                int size = 1;
                tinfo_t type;

                if (get_tinfo(&type, ea))
                {
                    size = type.get_size();

                    if (size < 1)
                    {
                        size = 1;
                    }

                    auto itemSize = get_item_size(ea);

                    if (itemSize > size)
                    {
                        size = itemSize;
                    }
                }

                qstring addrName = getAddrName(ea);

                if (!addrName.empty() && !isDataPrefixReserved(addrName) && get_str_type(ea) == -1)
                {
                    qstring vtClassName = getVTableClassName(get_short_name(ea));

                    if (!vtClassName.empty())
                    {
                        qstring forClass;
                        Type classType;
                        VirtualFunctionTable vTable;

                        if (contains(vtClassName, "{for"))
                        {
                            size_t startPosition = vtClassName.find('{') + 6;
                            forClass = vtClassName.substr(startPosition, vtClassName.find("'}"));

                            vtClassName = vtClassName.substr(0, vtClassName.find('`') - 2);
                            bool exists = false;

                            for (int i = 0; i < classes.size(); i++)
                            {
                                string className = classes[i].className;

                                if (className.compare(vtClassName.c_str()) == 0)
                                {
                                    vTable.forClass = forClass.c_str();
                                    classes[i].virtualFunctionTables.push_back(vTable);

                                    exists = true;
                                    break;
                                }
                            }

                            if (!exists)
                            {
                                classType.className = vtClassName.c_str();
                                vTable.forClass = forClass.c_str();
                                classType.virtualFunctionTables.push_back(vTable);
                            }
                        }
                        else
                        {
                            classType.className = vtClassName.c_str();
                        }

                        if (classType.className.length() > 0)
                        {
                            classes.push_back(classType);
                        }

                        VTClassInfo vtClassInfo;
                        vtClassInfo.addr = ea;
                        vtClassInfo.className = vtClassName;
                        unsigned int vtSize = 0;
                        auto vtFuncAddr = ea;

                        while (isOffsetAtAddress(vtFuncAddr) && (vtSize == 0 || getAddrName(vtFuncAddr).empty()))
                        {
                            auto funcAddr = getDword(vtFuncAddr);

                            if (funcAddr != 0)
                            {
                                if (get_func(funcAddr))
                                {
                                    qstring funcName = getFunctionName(funcAddr);

                                    if (!isPureFunctionName(funcName) && !isNullFunctionName(funcName))
                                    {
                                        if (virtualFuncs.find(funcAddr) == virtualFuncs.end())
                                        {
                                            virtualFuncs[funcAddr] = vtSize;

                                            Function virtualFunction = Function::getFunction(funcAddr, virtualFuncs);

                                            for (int i = 0; i < classes.size(); i++)
                                            {
                                                string className = classes[i].className;

                                                if (className.compare(vtClassName.c_str()) == 0)
                                                {
                                                    bool exists = false;

                                                    for (int j = 0; j < classes[i].virtualFunctionTables.size(); j++)
                                                    {
                                                        VirtualFunctionTable vTable1 = classes[i].virtualFunctionTables[j];
                                                        string forClass2 = vTable1.forClass;

                                                        if (forClass2.compare(forClass.c_str()) == 0)
                                                        {
                                                            classes[i].virtualFunctionTables[j].virtualFunctions.push_back(virtualFunction);

                                                            exists = true;
                                                            break;
                                                        }
                                                    }

                                                    if (!exists)
                                                    {
                                                        VirtualFunctionTable vTable1;
                                                        vTable1.virtualFunctions.push_back(virtualFunction);

                                                        classes[i].virtualFunctionTables.push_back(vTable1);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            vtFuncAddr += 4;
                            vtSize++;
                        }

                        vtClassInfo.size = vtSize;
                        vtables.push_back(vtClassInfo);
                    }
                }

                ea += size;
            }
        }

        seg = get_next_seg(seg->start_ea);
    }

    addInheritedClasses(classes, vtables);
    addNonVirtualFunctions(classes, virtualFuncs);

    nlohmann::ordered_json json = createJSONFromData(classes);

    string fileName = "functions.json";
    path filePath = dbFolderPath / fileName;

    writeJSONToFile(json, filePath.string().c_str());

    warning("Export finished");
}

void addInheritedClasses(vector<Type>& classes, qvector<VTClassInfo> vtables)
{
    for (size_t i = 0; i < get_struc_qty(); i++)
    {
        auto stid = get_struc_by_idx(i);
        auto s = get_struc(stid);
        qstring name = get_struc_name(stid);
        unsigned int size = get_struc_size(s);

        qvector<unsigned int> baseClassMembers;
        tinfo_t stinfo;

        if (guessTInfo(&stinfo, stid) == GUESS_FUNC_OK)
        {
            udt_type_data_t udttd;

            if (stinfo.get_udt_details(&udttd))
            {
                for (auto& mudt : udttd)
                {
                    if (mudt.is_baseclass())
                    {
                        baseClassMembers.push_back(mudt.offset / 8);
                    }
                }
            }
        }

        VTClassInfo* vtClassInfo = nullptr;

        for (auto vtable : vtables)
        {
            if (vtable.className == name)
            {
                vtClassInfo = &vtable;
                break;
            }
        }

        unsigned int offset = 0;

        vector<string> inheritedClasses;

        while (offset <= size)
        {
            member_t* member = get_member(s, offset);

            if (member)
            {
                auto mid = member->id;
                unsigned int msize = get_member_size(member);
                unsigned int moffset = member->get_soff();

                qstring mtype;
                tinfo_t mtinfo;

                if (get_or_guess_member_tinfo(&mtinfo, member))
                {
                    mtinfo.print(&mtype);
                }

                bool isBaseClass = false;

                if (baseClassMembers.size() > 0)
                {
                    for (auto bcm : baseClassMembers)
                    {
                        if (moffset == bcm)
                        {
                            isBaseClass = true;
                            break;
                        }
                    }
                }

                if (isBaseClass)
                {
                    string className = mtype.c_str();
                    inheritedClasses.push_back(className);
                }
            }

            offset = get_struc_next_offset(s, offset);
        }

        for (int i = 0; i < classes.size(); i++)
        {
            string className = name.c_str();
            string className2 = classes[i].className;

            if (className.compare(className2.c_str()) == 0)
            {
                classes[i].inheritedClasses.insert(std::end(classes[i].inheritedClasses), std::begin(inheritedClasses),
                    std::end(inheritedClasses));

                break;
            }
        }
    }
}

void addNonVirtualFunctions(vector<Type>& classes, map<unsigned int, unsigned int> virtualFuncs)
{
    auto func = get_next_func(0);

    while (func)
    {
        auto ea = func->start_ea;

        Function function = Function::getFunction(ea, virtualFuncs);
        string className = "";

        string prototype = Function::getFunctionPrototype(ea, function.name.c_str());

        if (prototype.length() > 0)
        {
            if (prototype.find("::") != std::string::npos)
            {
                className = prototype.substr(0, prototype.find_last_of("::") - 1);
            }
        }

        if (className.length() > 0 && function.vtableIndex == -1)
        {
            for (int i = 0; i < classes.size(); i++)
            {
                string className2 = classes[i].className;

                if (className.compare(className2) == 0)
                {
                    classes[i].nonVirtualFunctions.push_back(function);
                    break;
                }
            }
        }

        func = get_next_func(func->start_ea);
    }
}

nlohmann::ordered_json createJSONFromData(vector<Type> classes)
{
    nlohmann::ordered_json json;
    auto& jClasses = json["classes"];

    for (int i = 0; i < classes.size(); i++)
    {
        nlohmann::ordered_json jClass;
        jClass["className"] = classes[i].className;

        auto& inheritedClasses = jClass["inheritedClasses"];

        for (int j = 0; j < classes[i].inheritedClasses.size(); j++)
        {
            string inheritedClass = classes[i].inheritedClasses[j];
            inheritedClasses.push_back(inheritedClass);
        }

        auto& virtualFunctionTables = jClass["virtualFunctionTables"];

        for (int j = 0; j < classes[i].virtualFunctionTables.size(); j++)
        {
            VirtualFunctionTable vTable = classes[i].virtualFunctionTables[j];
            nlohmann::ordered_json jVTable;

            jVTable["forClass"] = vTable.forClass;

            auto& virtualFunctions = jVTable["virtualFunctions"];

            for (int k = 0; k < vTable.virtualFunctions.size(); k++)
            {
                Function virtualFunction = vTable.virtualFunctions[k];
                nlohmann::ordered_json jVirtualFunction;

                jVirtualFunction["prototype"] = virtualFunction.prototype;
                jVirtualFunction["functionName"] = virtualFunction.demangledName;
                jVirtualFunction["address"] = virtualFunction.address;
                jVirtualFunction["callingConvention"] = virtualFunction.callingConvention;
                jVirtualFunction["returnType"] = virtualFunction.returnType;

                auto& parameters = jVirtualFunction["parameters"];

                for (int i = 0; i < virtualFunction.parameters.size(); i++)
                {
                    nlohmann::ordered_json parameter;

                    parameter["type"] = virtualFunction.parameters[i].type;
                    parameter["name"] = virtualFunction.parameters[i].name;

                    parameters.push_back(parameter);
                }

                jVirtualFunction["virtualTableIndex"] = to_string(virtualFunction.vtableIndex);

                virtualFunctions.push_back(jVirtualFunction);
            }

            virtualFunctionTables.push_back(jVTable);
        }

        auto& nonVirtualFunctions = jClass["nonVirtualFunctions"];

        for (int j = 0; j < classes[i].nonVirtualFunctions.size(); j++)
        {
            Function nonVirtualFunction = classes[i].nonVirtualFunctions[j];
            nlohmann::ordered_json jNonVirtualFunction;

            jNonVirtualFunction["prototype"] = nonVirtualFunction.prototype;
            jNonVirtualFunction["functionName"] = nonVirtualFunction.demangledName;
            jNonVirtualFunction["address"] = nonVirtualFunction.address;
            jNonVirtualFunction["callingConvention"] = nonVirtualFunction.callingConvention;
            jNonVirtualFunction["returnType"] = nonVirtualFunction.returnType;

            auto& parameters = jNonVirtualFunction["parameters"];

            for (int i = 0; i < nonVirtualFunction.parameters.size(); i++)
            {
                nlohmann::ordered_json parameter;

                parameter["type"] = nonVirtualFunction.parameters[i].type;
                parameter["name"] = nonVirtualFunction.parameters[i].name;

                parameters.push_back(parameter);
            }

            jNonVirtualFunction["virtualTableIndex"] = to_string(nonVirtualFunction.vtableIndex);

            nonVirtualFunctions.push_back(jNonVirtualFunction);
        }

        jClasses.push_back(jClass);
    }

    return json;
}