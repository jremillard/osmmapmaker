#include <QtGlobal>
#include <QtCore/qglobal.h>

extern "C" int qInitResources_mapmaker_resources();

static int init_resources()
{
    return qInitResources_mapmaker_resources();
}

static int dummy = init_resources();
