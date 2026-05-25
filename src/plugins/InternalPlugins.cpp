#include "ncbind/ncbind.hpp"
#include "LogFilter.h"
#include <stdio.h>

void TVPLoadInternalPlugins()
{
    ncbAutoRegister::AllRegist();
    ncbAutoRegister::LoadModule(TJS_W("xp3filter.dll"));
}

void TVPUnloadInternalPlugins()
{
    ncbAutoRegister::AllUnregist();
}

bool TVPLoadInternalPlugin(const ttstr &_name)
{
    ttstr storage = TVPExtractStorageName(_name);
    bool loaded = ncbAutoRegister::LoadModule(storage);
    ttstr module = storage;
    if (!loaded) {
        const tjs_char *last_sep = nullptr;
        for (const tjs_char *c = storage.c_str(); *c; c++) {
            if (*c == TJS_W('/') || *c == TJS_W('\\')) last_sep = c;
        }
        if (last_sep && *(last_sep + 1)) {
            module = ttstr(last_sep + 1);
            loaded = ncbAutoRegister::LoadModule(module);
        }
    }
    KRKR_LOG_L2("[INTERNAL-PLUGIN] request=%s storage=%s module=%s loaded=%d\n",
        _name.AsNarrowStdString().c_str(), storage.AsNarrowStdString().c_str(),
        module.AsNarrowStdString().c_str(), loaded ? 1 : 0);
    return loaded;
}
