#include "ida.hpp"
#include "idp.hpp"
#include "loader.hpp"
#include "interface.h"

int idaapi init(void)
{
    return PLUGIN_OK;
}

bool idaapi run(size_t)
{
    msg("--------------------\nPLUGIN START\n--------------------\n");
    showForm();

    return true;
}

plugin_t __declspec(dllexport) PLUGIN = {
    IDP_INTERFACE_VERSION,
    0,                         // plugin flags
    init,                      // initialize
    NULL,                      // terminate. this pointer may be NULL.
    run,                       // invoke plugin
    NULL,                      // long comment about the plugin
    NULL,                      // multiline help about the plugin
    "Export Functions",        // the preferred short name of the plugin
    NULL                       // the preferred hotkey to run the plugin
};