#include "tp_stub.h"
#include "ncbind/ncbind.hpp"
#include <sys/stat.h>
#include <dirent.h>

#define NCB_MODULE_NAME TJS_W("fstat.dll")

class tGetDirListFunction : public tTJSDispatch
{
	tjs_error TJS_INTF_METHOD FuncCall(
		tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
		tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
	{
		if(membername) return TJS_E_MEMBERNOTFOUND;
		if(numparams < 1) return TJS_E_BADPARAMCOUNT;

		ttstr dir(*param[0]);

		if(dir.GetLastChar() != TJS_W('/'))
			TVPThrowExceptionMessage(TJS_W("'/' must be specified at the end of given directory name."));

		dir = TVPNormalizeStorageName(dir);

		iTJSDispatch2 * array = TJSCreateArrayObject();
		if (!result) return TJS_S_OK;
		try {
			TVPGetLocalName(dir);
			tjs_string dirstr(dir.c_str());
			std::string ndir;
			for (size_t i = 0; i < dirstr.size(); i++) {
				ndir += (char)dirstr[i];
			}

			DIR *d = opendir(ndir.c_str());
			if (d) {
				struct dirent *ent;
				tjs_int idx = 0;
				while ((ent = readdir(d)) != NULL) {
					if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
						continue;
					std::string fullpath = ndir + ent->d_name;
					struct stat st;
					if (stat(fullpath.c_str(), &st) == 0) {
						if (S_ISREG(st.st_mode) || S_ISDIR(st.st_mode)) {
							ttstr name;
							const char *p = ent->d_name;
							while (*p) { name += (tjs_char)*p; p++; }
							tTJSVariant val(name);
							array->PropSetByNum(TJS_MEMBERENSURE, idx++, &val, array);
						}
					}
				}
				closedir(d);
			}
			*result = tTJSVariant(array, array);
			array->Release();
		}
		catch (...) {
			array->Release();
			throw;
		}

		return TJS_S_OK;
	}
} * GetDirListFunction;

static void PostRegistCallback()
{
	tTJSVariant val;
	iTJSDispatch2 * global = TVPGetScriptDispatch();

	GetDirListFunction = new tGetDirListFunction();
	val = tTJSVariant(GetDirListFunction);
	GetDirListFunction->Release();

	global->PropSet(
		TJS_MEMBERENSURE,
		TJS_W("getDirList"),
		NULL,
		&val,
		global
		);

	global->Release();
	val.Clear();
}
NCB_POST_REGIST_CALLBACK(PostRegistCallback);
