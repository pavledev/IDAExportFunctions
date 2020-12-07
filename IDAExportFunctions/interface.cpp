#include "ida.hpp"
#include "idp.hpp"
#include "name.hpp"
#include "export.h"
#include <Windows.h>
#include <filesystem>

using namespace std::filesystem;

enum eInputField
{
    FIELD_OUTPUTFOLDER = 1,
    FIELD_EXPORTBUTTON = 2
};

char gOutputFolder[QMAXPATH];

static int idaapi exportcb(int, form_actions_t&)
{
    if (gOutputFolder[0] == '\0')
    {
        warning("Output folder was not selected");

        return 0;
    }

    static char outputPathStr[QMAXPATH];
    ExpandEnvironmentStringsA(gOutputFolder, outputPathStr, QMAXPATH);
    path output = outputPathStr;

    if (!exists(output))
    {
        warning("Output folder does not exist (%s)", output.string().c_str());

        return 0;
    }

    exportdb(output);

    return 0;
}

static int idaapi modcb(int fid, form_actions_t& fa)
{
    if (fid == FIELD_OUTPUTFOLDER)
    {
        fa.get_path_value(FIELD_OUTPUTFOLDER, gOutputFolder, QMAXPATH);
    }

    return 1;
}

void showForm()
{
    qstring formdef =
        "BUTTON NO NONE\n"
        "BUTTON YES NONE\n"
        "BUTTON CANCEL NONE\n"
        "Export functions\n"
        "\n"
        "%/"
        "<Output folder:F1::35:::>"
        "\n"
        "<Export:B2:::::>\n"
        "\n";

    ask_form(formdef.c_str(), modcb, gOutputFolder, exportcb);
}
