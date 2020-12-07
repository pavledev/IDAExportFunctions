#pragma once
#include "ida.hpp"

struct VTClassInfo
{
    unsigned int addr;
    unsigned int size;
    qstring className;

    VTClassInfo() : addr(0), size(0), className("") {}
};