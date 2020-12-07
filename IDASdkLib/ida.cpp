#include "ida.h"
#include "mystring.h"

bool isFunctionPrefixReserved(qstring const& name)
{
    return startsWith(name, "sub_") ||
        startsWith(name, "_sub_") ||
        startsWith(name, "j_sub_") ||
        startsWith(name, "nullsub_") ||
        startsWith(name, "_nullsub_") ||
        startsWith(name, "j_nullsub_") ||
        startsWith(name, "dummy_") ||
        startsWith(name, "_dummy_") ||
        startsWith(name, "j_dummy_") ||
        startsWith(name, "unknown_libname_") ||
        startsWith(name, "_unknown_libname_") ||
        startsWith(name, "j_unknown_libname_") ||
        startsWith(name, "locret_") ||
        startsWith(name, "loc_") ||
        startsWith(name, "unk_");
}

bool isDataPrefixReserved(qstring const& name)
{
    return startsWith(name, "off_") ||
        startsWith(name, "seg_") ||
        startsWith(name, "asc_") ||
        startsWith(name, "byte_") ||
        startsWith(name, "word_") ||
        startsWith(name, "dword_") ||
        startsWith(name, "qword_") ||
        startsWith(name, "byte3_") ||
        startsWith(name, "xmmword_") ||
        startsWith(name, "ymmword_") ||
        startsWith(name, "packreal_") ||
        startsWith(name, "flt_") ||
        startsWith(name, "dbl_") ||
        startsWith(name, "tbyte_") ||
        startsWith(name, "stru_") ||
        startsWith(name, "custdata_") ||
        startsWith(name, "algn_") ||
        startsWith(name, "unk_") ||
        startsWith(name, "GUID_") ||
        startsWith(name, "IID_") ||
        startsWith(name, "CLSID_") ||
        //startsWith(name, "??_") ||
        startsWith(name, "_eh_") ||
        (startsWith(name, "a") && isNumber(name.substr(1)));
}

bool isPrefixReserved(qstring const& name)
{
    return isFunctionPrefixReserved(name) || isDataPrefixReserved(name);
}

bool isDataSegment(qstring const& name)
{
    return name == ".data" || name == ".rdata" || name == ".bss";
}

bool isInDataSegment(ea_t ea)
{
    auto seg = get_first_seg();

    while (seg)
    {
        if (ea >= seg->start_ea && ea <= seg->end_ea)
        {
            qstring segName;

            get_segm_name(&segName, seg);

            if (isDataSegment(segName))
            {
                return true;
            }
        }

        seg = get_next_seg(seg->start_ea);
    }

    return false;
}

bool isPureFunctionName(qstring const& name)
{
    return startsWith(name, "__pure");
}

bool isNullFunctionName(qstring const& name)
{
    return startsWith(name, "nullsub") || startsWith(name, "j_nullsub");
}

bool parseType(qstring const& typeName, tinfo_t& out, bool silent)
{
    qstring fixedTypeName = typeName;

    if (typeName.last() != ';')
    {
        fixedTypeName += ";";
    }

    qstring outTypeName;

    if (parse_decl(&out, &outTypeName, NULL, fixedTypeName.c_str(), silent ? PT_SIL : 0))
    {
        return true;
    }

    return false;
}

bool setType(ea_t ea, qstring const& typeName, bool silent)
{
    tinfo_t tif;

    if (!typeName.empty())
    {
        if (!parseType(typeName, tif, silent))
        {
            return false;
        }
    }

    return apply_tinfo(ea, tif, TINFO_DEFINITE);
}

bool setType(struc_t* struc, size_t offset, qstring const& typeName, bool silent)
{
    auto member = get_member(struc, offset);

    if (!member)
    {
        return false;
    }

    return setType(struc, member, offset, typeName, silent);
}

bool setType(struc_t* struc, member_t* member, size_t offset, qstring const& typeName, bool silent)
{
    tinfo_t tif;

    if (!typeName.empty())
    {
        if (!parseType(typeName, tif, silent))
        {
            return false;
        }
    }

    return set_member_tinfo(struc, member, offset, tif, 0) == SMT_OK;
}

bool getLine(qstring* buf, FILE* fp)
{
    return qgetline(buf, fp) != -1;
}

qstring getVTableClassName(qstring const& vtableVarName)
{
    qstring result;

    if (contains(vtableVarName, "`vtable for'"))
    {
        result = vtableVarName.substr(12);
    }
    else if (contains(vtableVarName, "::`vftable'"))
    {
        if (contains(vtableVarName, "{for"))
        {
            if (contains(vtableVarName, "const "))
            {
                result = vtableVarName.substr(6);
            }
            else
            {
                result = vtableVarName;
            }
        }
        else
        {
            if (contains(vtableVarName, "const "))
            {
                result = vtableVarName.substr(6, vtableVarName.length() - 11);
            }
            else
            {
                result = vtableVarName.substr(0, vtableVarName.length() - 11);
            }
        }
    }

    return result;
}

qstring getAddrName(ea_t ea)
{
    return get_name(ea);
}

qstring getFunctionName(ea_t ea)
{
    qstring name;

    get_func_name(&name, ea);

    return name;
}

unsigned int getDword(ea_t ea)
{
    return get_dword(ea);
}

bool isOffsetAtAddress(ea_t ea)
{
    return is_off0(get_flags(ea));
}

bool isCodeAtAddress(ea_t ea)
{
    return is_code(get_flags(ea));
}

int guessTInfo(tinfo_t* tif, tid_t id)
{
    return guess_tinfo(tif, id);
}

int getInstructionSize(ea_t ea)
{
    insn_t insn;

    return decode_insn(&insn, ea);
}
