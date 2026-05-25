//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Universal Storage System
//---------------------------------------------------------------------------
#include "tjsCommHead.h"

#include <algorithm>
#include <stdexcept>
#include <memory>
#include <set>
#include "StorageIntf.h"
#include "tjsUtils.h"
#include "MsgIntf.h"
#include "EventIntf.h"
#include "DebugIntf.h"
#include "tjsArray.h"
#include "SysInitIntf.h"
#include "XP3Archive.h"
#include "TickCount.h"
#include "StringUtil.h"
#include "FilePathUtil.h"
#include "tjsDictionary.h"
#include "LogFilter.h"

#define TVP_DEFAULT_ARCHIVE_CACHE_NUM 64
#define TVP_DEFAULT_AUTOPATH_CACHE_NUM 256


//---------------------------------------------------------------------------
// オプション
//---------------------------------------------------------------------------
static bool TVPIsInitStorageOptions = false;
static bool TVPIgnoreFileProperty = false;
//---------------------------------------------------------------------------
static void TVPInitStorageOptions() {
	if( TVPIsInitStorageOptions ) return;

	tTJSVariant val;
	if( TVPGetCommandLine( TJS_W( "-ignorefileprop" ), &val ) ) {
		ttstr str( val );
		if( str == TJS_W( "yes" ) )
			TVPIgnoreFileProperty = true;
		else
			TVPIgnoreFileProperty = false;
	}
	TVPIsInitStorageOptions = true;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// global variables
//---------------------------------------------------------------------------
// current media ( ex. "http" "ftp" "file" )
ttstr TVPCurrentMedia;
// archive delimiter
// this changes '>' from '#' since 2.19 beta 14
tjs_char  TVPArchiveDelimiter = '>';
//---------------------------------------------------------------------------

static bool TVPTraceFontStorageName(const ttstr &name)
{
	std::string n = name.AsNarrowStdString();
	return n.find("default.ttf") != std::string::npos ||
		n.find("default.ttc") != std::string::npos ||
		n.find("default.otf") != std::string::npos ||
		n.find("DroidSansFallback") != std::string::npos ||
		n.find("simhei") != std::string::npos;
}

#ifdef __EMSCRIPTEN__
static bool TVPTracePsbStorageName(const ttstr &name)
{
	std::string n = name.AsNarrowStdString();
	return n.find(".psb") != std::string::npos ||
		n.find(".PSB") != std::string::npos;
}

static std::string TVPTraceStorageNameUtf8(const ttstr &name)
{
	std::string out;
	if(!TVPUtf16ToUtf8(out, name.AsStdString())) out = name.AsNarrowStdString();
	return out;
}
#endif




//---------------------------------------------------------------------------
// statics
//---------------------------------------------------------------------------
static tTJSCriticalSection TVPCreateStreamCS;
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// utilities
//---------------------------------------------------------------------------
ttstr TVPStringFromBMPUnicode(const tjs_uint16 *src, tjs_int maxlen)
{
	// convert to ttstr from BMP unicode
	if(sizeof(tjs_char) == 2)
	{
		// sizeof(tjs_char) is 2 (windows native)
		if(maxlen == -1)
			return ttstr((const tjs_char*)src);
		else
			return ttstr((const tjs_char*)src, maxlen);
	}
	else if(sizeof(tjs_char) == 4)
	{
		// sizeof(tjs_char) is 4 (UCS32)
  		// FIXME: NOT TESTED CODE
		tjs_int len = 0;
		const tjs_uint16 *p = src;
		while(*p) len++, p++;
		if(maxlen != -1 && len > maxlen) len = maxlen;
		ttstr ret((tTJSStringBufferLength)(len));
		tjs_char *dest = ret.Independ();
		p = src;
		while(len && *p)
		{
			*dest = *p;
			dest++;
			p++;
			len --;
		}
		*dest = 0;
		ret.FixLen();
		return ret;
	}
	return (const tjs_char*)TVPTjsCharMustBeTwoOrFour;
}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
// tTVPStorageMediaManager
//---------------------------------------------------------------------------
class tTVPStorageMediaManager
{
	class tMediaNameString : public tTJSString
	{
	public:
		bool operator == (const tMediaNameString &rhs) const
		{
			const tjs_char * l_p = c_str();
			const tjs_char * r_p = rhs.c_str();

			while(*l_p && *r_p)
			{
				if(*l_p == TJS_W(':')) break;
				if(*r_p == TJS_W(':')) break;
				if(*l_p != *r_p) break;
				l_p++;
				r_p++;
			}
			if((*l_p == TJS_W(':') || *l_p == 0) &&
				(*r_p == TJS_W(':') || *r_p == 0)) return true;
			return false;
		}
	};

	class tHashFunc
	{
	public:
		static tjs_uint32 Make(const tMediaNameString &key)
		{
			if(key.IsEmpty()) return 0;
			const tjs_char *str = key.c_str();
			tjs_uint32 ret = 0;
			while(*str && *str != ':')
			{
				ret += *str;
				ret += (ret << 10);
				ret ^= (ret >> 6);
				str++;
			}
			ret += (ret << 3);
			ret ^= (ret >> 11);
			ret += (ret << 15);
			if(!ret) ret = (tjs_uint32)-1;
			return ret;
		}
	};

	class tMediaRecord
	{
	public:
		ttstr CurrentDomain;
		ttstr CurrentPath;
		tTJSRefHolder<iTVPStorageMedia> MediaIntf;
		tjs_int MediaNameLen;
//		bool IsCaseSensitive;

		tMediaRecord(iTVPStorageMedia *media) : MediaIntf(media), CurrentDomain("."), CurrentPath("/")
			{ ttstr name; media->GetName(name); MediaNameLen = name.GetLen();
			/*IsCaseSensitive = media->IsCaseSensitive();*/ }

		const tjs_char *GetDomainAndPath(const ttstr &name)
		{
			return name.c_str() + MediaNameLen + 3;
				// 3 = strlen("://")
		}
	};

	typedef tTJSHashTable<tMediaNameString, tMediaRecord, tHashFunc, 16> tHashTable;

	tHashTable HashTable;

public:
	tTVPStorageMediaManager();
	~tTVPStorageMediaManager();

private:
	static void ThrowUnsupportedMediaType(const ttstr &name);
	tMediaRecord * GetMediaRecord(const ttstr &name);

public:
	void Register(iTVPStorageMedia * media);
	void Unregister(iTVPStorageMedia * media);

	ttstr NormalizeStorageName(const ttstr &name, ttstr *ret_media = NULL,
		ttstr *ret_domain = NULL, ttstr *ret_path = NULL);

	void SetCurrentDirectory(const ttstr &name);

	static ttstr ExtractMediaName(const ttstr &name);

	bool CheckExistentStorage(const ttstr & name);
	tTJSBinaryStream * Open(const ttstr & name, tjs_uint32 flags);
	void GetListAt(const ttstr &name, iTVPStorageLister *lister);
	ttstr GetLocallyAccessibleName(const ttstr &name);
} TVPStorageMediaManager;
//---------------------------------------------------------------------------

extern "C" int emscripten_sync_http_check_exist(const char* url_ptr);
extern "C" int emscripten_sync_http_list(const char* url_ptr, char* out_buf, int max_len);

class tTVPHTTPStorageMedia : public iTVPStorageMedia {
	tjs_uint RefCount;
public:
	tTVPHTTPStorageMedia() { RefCount = 1; }
	virtual ~tTVPHTTPStorageMedia() {}

	void TJS_INTF_METHOD AddRef() { RefCount++; }
	void TJS_INTF_METHOD Release() {
		if(RefCount == 1) delete this;
		else RefCount--;
	}

	void TJS_INTF_METHOD GetName(ttstr &name) { name = TJS_W("http"); }

	void TJS_INTF_METHOD NormalizeDomainName(ttstr &name) {}
	void TJS_INTF_METHOD NormalizePathName(ttstr &name) {}
		bool TJS_INTF_METHOD CheckExistentStorage(const ttstr &name) {
			ttstr url = TJS_W("http://") + name;
			std::string utf8url;
			TVPUtf16ToUtf8(utf8url, url.AsStdString());
			bool exists = emscripten_sync_http_check_exist(utf8url.c_str()) == 1;
			if(TVPTraceFontStorageName(url) && TVPLogL3())
			{
				fprintf(stderr, "[HTTP-FONT] exists url=%s result=%d\n",
					utf8url.c_str(), (int)exists);
				fflush(stderr);
			}
			return exists;
		}

		tTJSBinaryStream * TJS_INTF_METHOD Open(const ttstr &name, tjs_uint32 flags) {
			extern tTJSBinaryStream * TVPCreateHTTPStream(const ttstr& url);
			ttstr url = TJS_W("http://") + name;
			if(TVPTraceFontStorageName(url) && TVPLogL3())
			{
				std::string utf8url;
				TVPUtf16ToUtf8(utf8url, url.AsStdString());
				fprintf(stderr, "[HTTP-FONT] media-open url=%s flags=%u\n",
					utf8url.c_str(), (unsigned)flags);
				fflush(stderr);
			}
			return TVPCreateHTTPStream(url);
		}

	void TJS_INTF_METHOD GetListAt(const ttstr &name, iTVPStorageLister *lister) {
		ttstr url = TJS_W("http://") + name;
		std::string utf8url;
		TVPUtf16ToUtf8(utf8url, url.AsStdString());
		
		char buf[65536];
		int len = emscripten_sync_http_list(utf8url.c_str(), buf, sizeof(buf));
		printf("C++ GetListAt called for %s, len=%d\n", utf8url.c_str(), len);
		if (len > 0) {
			std::string list_str(buf, len);
			size_t start = 0;
			while (start < list_str.length()) {
				size_t pos = list_str.find('\n', start);
				if (pos == std::string::npos) pos = list_str.length();
				std::string item = list_str.substr(start, pos - start);
				if (!item.empty()) {
					tjs_string item_w;
					TVPUtf8ToUtf16(item_w, item);
					lister->Add(ttstr(item_w));
				}
				start = pos + 1;
			}
		}
	}
	void TJS_INTF_METHOD GetLocallyAccessibleName(ttstr &name) { name = TJS_W(""); }
};

class tTVPHTTPSStorageMedia : public tTVPHTTPStorageMedia {
public:
	void TJS_INTF_METHOD GetName(ttstr &name) { name = TJS_W("https"); }
		bool TJS_INTF_METHOD CheckExistentStorage(const ttstr &name) {
			ttstr url = TJS_W("https://") + name;
			std::string utf8url;
			TVPUtf16ToUtf8(utf8url, url.AsStdString());
			bool exists = emscripten_sync_http_check_exist(utf8url.c_str()) == 1;
			if(TVPTraceFontStorageName(url) && TVPLogL3())
			{
				fprintf(stderr, "[HTTP-FONT] exists url=%s result=%d\n",
					utf8url.c_str(), (int)exists);
				fflush(stderr);
			}
			return exists;
		}
		tTJSBinaryStream * TJS_INTF_METHOD Open(const ttstr &name, tjs_uint32 flags) {
			extern tTJSBinaryStream * TVPCreateHTTPStream(const ttstr& url);
			ttstr url = TJS_W("https://") + name;
			if(TVPTraceFontStorageName(url) && TVPLogL3())
			{
				std::string utf8url;
				TVPUtf16ToUtf8(utf8url, url.AsStdString());
				fprintf(stderr, "[HTTP-FONT] media-open url=%s flags=%u\n",
					utf8url.c_str(), (unsigned)flags);
				fflush(stderr);
			}
			return TVPCreateHTTPStream(url);
		}
	void TJS_INTF_METHOD GetListAt(const ttstr &name, iTVPStorageLister *lister) {
		ttstr url = TJS_W("https://") + name;
		std::string utf8url;
		TVPUtf16ToUtf8(utf8url, url.AsStdString());
		
		char buf[65536];
		int len = emscripten_sync_http_list(utf8url.c_str(), buf, sizeof(buf));
		if (len > 0) {
			std::string list_str(buf, len);
			size_t start = 0;
			while (start < list_str.length()) {
				size_t pos = list_str.find('\n', start);
				if (pos == std::string::npos) pos = list_str.length();
				std::string item = list_str.substr(start, pos - start);
				if (!item.empty()) {
					tjs_string item_w;
					TVPUtf8ToUtf16(item_w, item);
					lister->Add(ttstr(item_w));
				}
				start = pos + 1;
			}
		}
	}
};

//---------------------------------------------------------------------------
tTVPStorageMediaManager::tTVPStorageMediaManager()
{
	iTVPStorageMedia *filemedia = TVPCreateFileMedia();
	Register(filemedia);
	filemedia->Release();

	iTVPStorageMedia *httpmedia = new tTVPHTTPStorageMedia();
	Register(httpmedia);
	httpmedia->Release();

	iTVPStorageMedia *httpsmedia = new tTVPHTTPSStorageMedia();
	Register(httpsmedia);
	httpsmedia->Release();
}
//---------------------------------------------------------------------------
tTVPStorageMediaManager::~tTVPStorageMediaManager()
{
}
//---------------------------------------------------------------------------
void tTVPStorageMediaManager::ThrowUnsupportedMediaType(const ttstr &name)
{
	TVPThrowExceptionMessage(TVPUnsupportedMediaName, ExtractMediaName(name));
}
//---------------------------------------------------------------------------
tTVPStorageMediaManager::tMediaRecord *
	tTVPStorageMediaManager::GetMediaRecord(const ttstr &name)
{
	tMediaRecord *rec = HashTable.Find(*(tMediaNameString*)&name);
	if(!rec) ThrowUnsupportedMediaType(name);
	return rec;
}
//---------------------------------------------------------------------------
void tTVPStorageMediaManager::Register(iTVPStorageMedia * media)
{
	ttstr medianame;
	media->GetName(medianame);

	tMediaRecord *rec = HashTable.Find(*(tMediaNameString*)&medianame);
	if(rec)
		TVPThrowExceptionMessage( TVPMediaNameHadAlreadyBeenRegistered, medianame );

	tMediaRecord new_rec(media);

	HashTable.Add(*(tMediaNameString*)&medianame, new_rec);
}
//---------------------------------------------------------------------------
void tTVPStorageMediaManager::Unregister(iTVPStorageMedia * media)
{
	ttstr medianame;
	media->GetName(medianame);

	tMediaRecord *rec = HashTable.Find(*(tMediaNameString*)&medianame);
	if(!rec)
		TVPThrowExceptionMessage( TVPMediaNameIsNotRegistered, medianame );
	HashTable.Delete(*(tMediaNameString*)&medianame);
}
//---------------------------------------------------------------------------
ttstr tTVPStorageMediaManager::NormalizeStorageName(const ttstr &name,
	ttstr *ret_media, ttstr *ret_domain, ttstr *ret_path)
{
	// Normalize storage name.

	// storage name is basically in following form:
	// media://domain/path

	// media is sort of access method, like "file", "http" ...etc.
	// domain represents in which computer the data is.
	// path is where the data is in the computer.

	// empty check
	if(name.IsEmpty()) return name; // empty name is empty name

	// pre-normalize
	const tjs_char *pca;//, *pcb, *pcc;
	tjs_char *pa, *pb, *pc;

	ttstr tmp(name);
	TVPPreNormalizeStorageName(tmp);

	// unify path delimiter
	pa = tmp.Independ();
	while(*pa)
	{
		if(*pa == TJS_W('\\')) *pa = TJS_W('/');
		pa++;
	}

	// save in-archive storage name and normalize it
	ttstr inarchive_name;
	bool inarc_name_found = false;
	pca = tmp.c_str();
	pa = const_cast<tjs_char *>(TJS_strchr(pca, TVPArchiveDelimiter));
	if(pa)
	{
		inarchive_name = ttstr(pa + 1);
		tTVPArchive::NormalizeInArchiveStorageName(inarchive_name);
		inarc_name_found = true;
		tmp = ttstr(pca, (int)(pa - pca));
	}
	if(tmp.IsEmpty()) TVPThrowExceptionMessage(TVPInvalidPathName, name);


	// split the name into media, domain, path
	// (and guess what component is omitted)
	ttstr media, domain, path;

	// - find media name
	//   media name is: /^[A-Za-z]+:/
	pa = pb = tmp.Independ();
	while(*pa)
	{
		if(!(
			*pa >= TJS_W('A') && *pa <= TJS_W('Z') ||
			*pa >= TJS_W('a') && *pa <= TJS_W('z') )) break;
		pa ++;
	}

	if(*pa == TJS_W(':'))
	{
		// media name found
		media = ttstr(pb, (int)(pa - pb));
		pa ++;
	}
	else
	{
		pa = pb;
	}

	// - find domain name
	// at this place, pa may point one of following:
	//  ///path        (domain is omitted)
	//  //domain/path  (none is omitted)
	//  /path          (domain is omitted)
	//  relative-path  (domain and current path are omitted)

	if(pa[0] == TJS_W('/'))
	{
		if(pa[1] == TJS_W('/'))
		{
			if(pa[2] == TJS_W('/'))
			{
				// slash count 3: domain is ommited
				pa += 2;
			}
			else
			{
				// slash count 2: none is omitted
				pa += 2;
				// find '/' as a domain delimiter
				pc = TJS_strchr(pa, TJS_W('/'));
				if(!pc)
					TVPThrowExceptionMessage(TVPInvalidPathName, name);
				domain = ttstr(pa, (int)(pc - pa));
				pa = pc;
			}
		}
		else
		{
			// slash count 1: domain is omitted
			;
			//
		}
	}

	// - get path name
	path = pa;

	// supply omitted and normalize
	if(media.IsEmpty())
	{
		media = TVPCurrentMedia;
	}
	else
	{
		// normalize media name ( make them all small )
		tjs_char *p = media.Independ();
		while(*p)
		{
			if(*p >= TJS_W('A') && *p <= TJS_W('Z'))
				*p += (TJS_W('a') - TJS_W('A'));
			p ++;
		}
	}

	tMediaRecord * mediarec = GetMediaRecord(media);

	if(domain.IsEmpty()) domain = mediarec->CurrentDomain;
	mediarec->MediaIntf.GetObjectNoAddRef()->NormalizeDomainName(domain);

	if(path.IsEmpty())
	{
		path = TJS_W("/");
	}
	else if(path.c_str()[0] != TJS_W('/'))
	{
		path = mediarec->CurrentPath + path;
	}
	mediarec->MediaIntf.GetObjectNoAddRef()->NormalizePathName(path);

	// compress redudant path accesses
	if(inarc_name_found)
	{
		tjs_char tmp[2];
		tmp[0] = TVPArchiveDelimiter;
		tmp[1] = 0;
		path += tmp + inarchive_name;
	}

	pa = pb = pc = path.Independ(); // pa = read pointer, pb = write pointer, pc = start
	tjs_int dot_count = -1;

	while(true)
	{
		if(*pa == TVPArchiveDelimiter || *pa == TJS_W('/') || *pa == 0)
		{
			tjs_char delim = 0;

			if(*pa && dot_count == 0)
			{
				// duplicated slashes
				pb --;
			}
			else if(dot_count > 0)
			{
				pb --;
				while(pb >= pc)
				{
					if(*pb == TJS_W('/') || *pb == TVPArchiveDelimiter)
					{
						dot_count --;
						if(dot_count == 0)
						{
							delim = *pb;
							break;
						}
						if(*pb == TVPArchiveDelimiter) TVPThrowExceptionMessage(TVPInvalidPathName, name);
					}
					pb --;
				}
				if(pb < pc) TVPThrowExceptionMessage(TVPInvalidPathName, name);
			}

			if(!delim)
				*pb = *pa;
			else
				*pb = delim;
			if(*pa == 0) break;
			pb ++;
			pa ++;
			dot_count = 0;
		}
		else if(*pa == TJS_W('.'))
		{
			*(pb++) = *(pa++);
			if(dot_count != -1) dot_count ++;
		}
		else
		{
			*(pb++) = *(pa++);
			dot_count = -1;
		}
	}

	path.FixLen();

	// merge and return normalize storage name
	if(ret_media) *ret_media = media;
	if(ret_domain) *ret_domain = domain;
	if(ret_path) *ret_path = path;

	tmp = media + TJS_W("://") + domain + path;

	return tmp;
}
//---------------------------------------------------------------------------
void tTVPStorageMediaManager::SetCurrentDirectory(const ttstr &name)
{
	tjs_char ch = name.GetLastChar();
	if(ch != TJS_W('/') && ch != TJS_W('\\') && ch != TVPArchiveDelimiter)
		TVPThrowExceptionMessage(TVPMissingPathDelimiterAtLast);

	ttstr media, domain, path;
	NormalizeStorageName(name, &media, &domain, &path);

	tMediaRecord *rec = GetMediaRecord(media);
	rec->CurrentDomain = domain;
	rec->CurrentPath = path;
	TVPCurrentMedia = media;
}
//---------------------------------------------------------------------------
ttstr tTVPStorageMediaManager::ExtractMediaName(const ttstr &name)
{
	// extract media name from normalized storage named "name".
	// returned media name does not contain colon.

	const tjs_char * p = name.c_str();
	const tjs_char * po = p;
	while(*p && *p != TJS_W(':')) p++;
	return ttstr(po, (int)(p - po));
}
//---------------------------------------------------------------------------
bool tTVPStorageMediaManager::CheckExistentStorage(const ttstr & name)
{
	// gateway for CheckExistentStorage
	// name must not be an in-archive storage name
	tMediaRecord *rec = GetMediaRecord(name);
	return rec->MediaIntf.GetObjectNoAddRef()->CheckExistentStorage(rec->GetDomainAndPath(name));
}
//---------------------------------------------------------------------------
tTJSBinaryStream * tTVPStorageMediaManager::Open(const ttstr & name, tjs_uint32 flags)
{
	// gateway for Open
	// name must not be an in-archive storage name
	tMediaRecord *rec = GetMediaRecord(name);
	return rec->MediaIntf.GetObjectNoAddRef()->Open(rec->GetDomainAndPath(name), flags);
}
//---------------------------------------------------------------------------
void tTVPStorageMediaManager::GetListAt(const ttstr &name, iTVPStorageLister * lister)
{
	// gateway for GetListAt
	// name must not be an in-archive storage name
	tMediaRecord *rec = GetMediaRecord(name);
	/*return */rec->MediaIntf.GetObjectNoAddRef()->GetListAt(rec->GetDomainAndPath(name), lister);
}
//---------------------------------------------------------------------------
ttstr tTVPStorageMediaManager::GetLocallyAccessibleName(const ttstr &name)
{
	// gateway for GetLocallyAccessibleName
	// name must not be an in-archive storage name
	tMediaRecord *rec = GetMediaRecord(name);
	ttstr dname = rec->GetDomainAndPath(name);
	rec->MediaIntf.GetObjectNoAddRef()->GetLocallyAccessibleName(dname);
	return dname;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void TVPRegisterStorageMedia(iTVPStorageMedia *media)
{
	TVPStorageMediaManager.Register(media);
}
//---------------------------------------------------------------------------
void TVPUnregisterStorageMedia(iTVPStorageMedia *media)
{
	TVPStorageMediaManager.Unregister(media);
}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
// TVPNormalizeStorgeName : storage name normalization
//---------------------------------------------------------------------------
ttstr TVPNormalizeStorageName(const ttstr & _name)
	// TODO: check what is done in TVPNormalizeStorageName
{
	return TVPStorageMediaManager.NormalizeStorageName(_name);
}
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
// TVPSetCurrentDirectory
//---------------------------------------------------------------------------
void TVPSetCurrentDirectory(const ttstr & _name)
{
	TVPStorageMediaManager.SetCurrentDirectory(_name);
	TVPClearStorageCaches();
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// TVPGetLocalName and TVPGetLocallyAccessibleName
//---------------------------------------------------------------------------
void TVPGetLocalName(ttstr &name)
{
	ttstr tmp = TVPGetLocallyAccessibleName(name);
	if(tmp.IsEmpty()) TVPThrowExceptionMessage(TVPCannotGetLocalName, name);
	name = tmp;
}
//---------------------------------------------------------------------------
ttstr TVPGetLocallyAccessibleName(const ttstr &name)
{
	if(TJS_strchr(name.c_str(), TVPArchiveDelimiter)) return TJS_W("");
		 // in-archive storage is always not accessible from local file system
	return TVPStorageMediaManager.GetLocallyAccessibleName(name);
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// tTVPArchive
//---------------------------------------------------------------------------
void tTVPArchive::NormalizeInArchiveStorageName(ttstr & name)
{
	// normalization of in-archive storage name does :
	if(name.IsEmpty()) return;

	// make all characters small
	// change '\\' to '/'
	tjs_char *ptr = name.Independ();
	while(*ptr)
	{
		if(*ptr >= TJS_W('A') && *ptr <= TJS_W('Z'))
			*ptr += TJS_W('a') - TJS_W('A');
		else if(*ptr == TJS_W('\\'))
			*ptr = TJS_W('/');
		ptr++;
	}

	// eliminate duplicated slashes
	ptr = name.Independ();
	tjs_char *org_ptr = ptr;
	tjs_char *dest = ptr;
	while(*ptr)
	{
		if(*ptr != TJS_W('/'))
		{
			*dest = *ptr;
			ptr ++;
			dest ++;
		}
		else
		{
			if(ptr != org_ptr)
			{
				*dest = *ptr;
				ptr ++;
				dest ++;
			}
			while(*ptr == TJS_W('/')) ptr++;
		}
	}
	*dest = 0;

	name.FixLen();
}
//---------------------------------------------------------------------------
void tTVPArchive::AddToHash()
{
	// enter all names to the hash table
	tjs_uint Count = GetCount();
	tjs_uint i;
	for(i = 0; i < Count; i++)
	{
		ttstr name = GetName(i);
		NormalizeInArchiveStorageName(name);
		Hash.Add(name, i);
	}
}
//---------------------------------------------------------------------------
tTJSBinaryStream * tTVPArchive::CreateStream(const ttstr & name)
{
#ifdef __EMSCRIPTEN__
	bool tracePsb = TVPTracePsbStorageName(name);
	if(tracePsb)
	{
		KRKR_LOG_L3("[XP3-ARCHIVE] create begin archive=%s item=%s init=%d\n",
			TVPTraceStorageNameUtf8(ArchiveName).c_str(),
			TVPTraceStorageNameUtf8(name).c_str(),
			Init ? 1 : 0);
	}
#endif
	if(name.IsEmpty()) return NULL;

	if(!Init)
	{
		Init = true;
		AddToHash();
	}

	tjs_uint *p = Hash.Find(name);
	if(!p) TVPThrowExceptionMessage(TVPStorageInArchiveNotFound,
		name, ArchiveName);

	tTJSBinaryStream *stream = CreateStreamByIndex(*p);
#ifdef __EMSCRIPTEN__
	if(tracePsb)
	{
		KRKR_LOG_L3("[XP3-ARCHIVE] create done archive=%s item=%s index=%u stream=%p\n",
			TVPTraceStorageNameUtf8(ArchiveName).c_str(),
			TVPTraceStorageNameUtf8(name).c_str(),
			(unsigned)*p, (void*)stream);
	}
#endif
	return stream;
}
//---------------------------------------------------------------------------
bool tTVPArchive::IsExistent(const ttstr & name)
{
	if(name.IsEmpty()) return false;

	if(!Init)
	{
		Init = true;
		AddToHash();
	}

	return Hash.Find(name) != NULL;
}
//---------------------------------------------------------------------------
tjs_int tTVPArchive::GetFirstIndexStartsWith(const ttstr & prefix)
{
	// returns first index which have 'prefix' at start of the name.
	// returns -1 if the target is not found.
	// the item must be sorted by ttstr::operator < , otherwise this function
	// will not work propertly.
	tjs_uint total_count = GetCount();
	tjs_int s = 0, e = total_count;
	while(e - s > 1)
	{
		tjs_int m = (e + s) / 2;
		if(!(GetName(m) < prefix))
		{
			// m is after or at the target
			e = m;
		}
		else
		{
			// m is before the target
			s = m;
		}
	}

	// at this point, s or s+1 should point the target.
	// be certain.
	if(s >= (tjs_int)total_count) return -1; // out of the index
	if(GetName(s).StartsWith(prefix)) return s;
	s++;
	if(s >= (tjs_int)total_count) return -1; // out of the index
	if(GetName(s).StartsWith(prefix)) return s;
	return -1;
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// tTVPArchiveCache
//---------------------------------------------------------------------------
class tTVPArchiveCache
{
	typedef tTJSRefHolder<tTVPArchive> tHolder;
	tTJSHashCache<ttstr, tHolder> ArchiveCache;
	tTJSCriticalSection CS;


public:
	tTVPArchiveCache() : ArchiveCache(TVP_DEFAULT_ARCHIVE_CACHE_NUM)
	{
	}

	~tTVPArchiveCache()
	{
	}

	void SetMaxCount(tjs_int maxcount)
	{
		ArchiveCache.SetMaxCount(maxcount);
	}

	void Clear()
	{
		// releases all elements
		ArchiveCache.Clear();
	}

	tTVPArchive * Get(ttstr name)
	{
		name = TVPNormalizeStorageName(name);
		tTJSCSH csh(CS);
		tjs_uint32 hash = tTJSHashCache<ttstr, tHolder>::MakeHash(name);
		tHolder *ptr = ArchiveCache.FindAndTouchWithHash(name, hash);
		if(ptr)
		{
			// exist in the cache
			return ptr->GetObject();
		}

		if(!TVPIsExistentStorageNoSearch(name))
		{
			// storage not found
			TVPThrowExceptionMessage(TVPCannotFindStorage, name);
		}

		// not exist in the cache
		tTVPArchive *arc = TVPOpenArchive(name);
		tHolder holder(arc);
		ArchiveCache.AddWithHash(name, hash, holder);
		return arc;
	}

private:

} TVPArchiveCache;
static void TVPClearArchiveCache() { TVPArchiveCache.Clear(); }
static tTVPAtExit TVPClearArchiveCacheAtExit
	(TVP_ATEXIT_PRI_SHUTDOWN, TVPClearArchiveCache);
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
// TVPIsExistentStorageNoSearch
//---------------------------------------------------------------------------
bool TVPIsExistentStorageNoSearchNoNormalize(const ttstr &name)
{
	// does name contain > ?
	tTJSCriticalSectionHolder cs_holder(TVPCreateStreamCS);

	const tjs_char * sharp_pos = TJS_strchr(name.c_str(), TVPArchiveDelimiter);
	if(sharp_pos)
	{
		// this storagename indicates a file in an archive

		ttstr arcname(name, (int)(sharp_pos - name.c_str()));

		tTVPArchive *arc;
		arc = TVPArchiveCache.Get(arcname);
		bool ret;
		try
		{
			ttstr in_arc_name(sharp_pos + 1);
			tTVPArchive::NormalizeInArchiveStorageName(in_arc_name);
			ret = arc->IsExistent(in_arc_name);
		}
		catch(...)
		{
			arc->Release();
			throw;
		}
		arc->Release();
		return ret;
	}

	return TVPStorageMediaManager.CheckExistentStorage(name);
}
//---------------------------------------------------------------------------
bool TVPIsExistentStorageNoSearch(const ttstr &_name)
{
	return TVPIsExistentStorageNoSearchNoNormalize(TVPNormalizeStorageName(_name));
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// TVPExtractStorageExt
//---------------------------------------------------------------------------
ttstr TVPExtractStorageExt(const ttstr & name)
{
	// extract an extension from name.
	// returned string will contain extension delimiter ( '.' ), except for
	// missing extension of the input string.
	// ( returns null string when input string does not have an extension )

	const tjs_char * s = name.c_str();
	tjs_int slen = name.GetLen();
	const tjs_char * p = s + slen;
	p--;
	while(p>=s)
	{
		if(*p == TJS_W('\\')) break;
		if(*p == TJS_W('/')) break;
		if(*p == TVPArchiveDelimiter) break;
		if(*p == TJS_W('.'))
		{
			// found extension delimiter
			tjs_int extlen = (tjs_int)(slen - ( p - s ));
			return ttstr(p, extlen);
		}

		p--;
	}

	// not found
	return ttstr();
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// TVPExtractStorageName
//---------------------------------------------------------------------------
ttstr TVPExtractStorageName(const ttstr & name)
{
	// extract "name"'s storage name ( excluding path ) and return it.
	const tjs_char * s = name.c_str();
	tjs_int slen = name.GetLen();
	const tjs_char * p = s + slen;
	p--;
	while(p>=s)
	{
		if(*p == TJS_W('\\')) break;
		if(*p == TJS_W('/')) break;
		if(*p == TVPArchiveDelimiter) break;

		p--;
	}

	p++;
	if(p == s)
		return name;
	else
		return ttstr(p, (int)(slen - (p -s)));
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// TVPExtractStoragePath
//---------------------------------------------------------------------------
ttstr TVPExtractStoragePath(const ttstr & name)
{
	// extract "name"'s path ( including last delimiter ) and return it.
	const tjs_char * s = name.c_str();
	tjs_int slen = name.GetLen();
	const tjs_char * p = s + slen;
	p--;
	while(p>=s)
	{
		if(*p == TJS_W('\\')) break;
		if(*p == TJS_W('/')) break;
		if(*p == TVPArchiveDelimiter) break;

		p--;
	}

	p++;
	return ttstr(s, (int)(p-s));
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// TVPChopStorageExt
//---------------------------------------------------------------------------
extern ttstr TVPChopStorageExt(const ttstr & name)
{
	// chop storage's extension and return it.
	const tjs_char * s = name.c_str();
	tjs_int slen = name.GetLen();
	const tjs_char * p = s + slen;
	p--;
	while(p>=s)
	{
		if(*p == TJS_W('\\')) break;
		if(*p == TJS_W('/')) break;
		if(*p == TVPArchiveDelimiter) break;
		if(*p == TJS_W('.'))
		{
			// found extension delimiter
			return ttstr(s, (int)(p-s));
		}

		p--;
	}

	// not found
	return name;
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// Auto search path support
//---------------------------------------------------------------------------
struct tTVPFileInfo
{
	static const tjs_int EXIST_PROP = 0x01;
	static const tjs_int EMPTY_FILE = 0x02;

	ttstr FilePath;
	iTJSDispatch2* Property = nullptr;
	tjs_int Flag = 0;

	tTVPFileInfo( const ttstr& path, tjs_int exist )
		: FilePath( path ), Flag( exist ) {
	}
	tTVPFileInfo( const ttstr& path, iTJSDispatch2* prop = nullptr )
	: FilePath(path), Property(prop)
	{
		if( Property ) Property->AddRef();
	}
	tTVPFileInfo( const tTVPFileInfo& info )
	: FilePath(info.FilePath), Property(info.Property), Flag(info.Flag )
	{
		if( Property ) Property->AddRef();
	}
	~tTVPFileInfo()
	{
		if( Property ) Property->Release();
	}
	tTVPFileInfo &operator=(const tTVPFileInfo &rhs)
	{
		if (this != &rhs) {
			if( Property ) Property->Release();
			FilePath = rhs.FilePath;
			Property = rhs.Property;
			Flag = rhs.Flag;
			if( Property )
			{
				Property->AddRef();
			}
		}
		return *this;
	}
	bool ExistProp() const {
		return (Flag & EXIST_PROP) != 0; 
	}
	bool ExistFile() const {
		return ( Flag & EMPTY_FILE ) == 0;
	}
};
#define TVP_AUTO_PATH_HASH_SIZE 1024
std::vector<ttstr> TVPAutoPathList;
tTJSHashCache<ttstr, ttstr> TVPAutoPathCache(TVP_DEFAULT_AUTOPATH_CACHE_NUM);
tTJSHashTable<ttstr, tTVPFileInfo, tTJSHashFunc<ttstr>, TVP_AUTO_PATH_HASH_SIZE>
	TVPAutoPathTable;
bool AutoPathTableInit = false;
//---------------------------------------------------------------------------
static void TVPClearAutoPathCache()
{
	TVPAutoPathCache.Clear();
	TVPAutoPathTable.Clear();
	AutoPathTableInit = false;
}
//---------------------------------------------------------------------------
struct tTVPClearAutoPathCacheCallback : public tTVPCompactEventCallbackIntf
{
	virtual void TJS_INTF_METHOD OnCompact(tjs_int level)
	{
		if(level >= TVP_COMPACT_LEVEL_DEACTIVATE)
		{
			// clear the auto search path cache on application deactivate
			tTJSCriticalSectionHolder cs_holder(TVPCreateStreamCS);
			TVPClearAutoPathCache();
		}
	}
} static TVPClearAutoPathCacheCallback;
static bool TVPClearAutoPathCacheCallbackInit = false;
//---------------------------------------------------------------------------
#ifdef __EMSCRIPTEN__
static bool TVPResolveSiblingArchiveAutoPath(const ttstr &normalized, ttstr &mirror)
{
	const tjs_char *sharp_pos = TJS_strchr(normalized.c_str(), TVPArchiveDelimiter);
	if(!sharp_pos) return false;

	ttstr arcname(normalized, (int)(sharp_pos - normalized.c_str()));
	ttstr in_arc_name(sharp_pos + 1);
	tTVPArchive::NormalizeInArchiveStorageName(in_arc_name);
	if(in_arc_name.IsEmpty()) return false;

	const tjs_char *in_arc = in_arc_name.c_str();
	const tjs_char *slash_pos = TJS_strchr(in_arc, TJS_W('/'));
	ttstr first;
	ttstr rest;
	if(slash_pos)
	{
		first = ttstr(in_arc, (int)(slash_pos - in_arc));
		rest = ttstr(slash_pos + 1);
	}
	else
	{
		first = in_arc_name;
		rest = ttstr();
	}
	if(first.IsEmpty()) return false;

	ttstr sibling_archive = TVPExtractStoragePath(arcname) + first + TJS_W(".xp3");
	sibling_archive = TVPNormalizeStorageName(sibling_archive);
	if(sibling_archive == arcname) return false;
	if(!TVPIsExistentStorageNoSearchNoNormalize(sibling_archive)) return false;

	mirror = sibling_archive + ttstr(TVPArchiveDelimiter) + rest;
	return true;
}
#endif
//---------------------------------------------------------------------------
void TVPAddAutoPath(const ttstr & name)
{
	tTJSCriticalSectionHolder cs_holder(TVPCreateStreamCS);

	tjs_char lastchar = name.GetLastChar();
	if(lastchar != TVPArchiveDelimiter && lastchar != TJS_W('/') && lastchar != TJS_W('\\'))
		TVPThrowExceptionMessage(TVPMissingPathDelimiterAtLast);

	ttstr normalized = TVPNormalizeStorageName(name);

	std::vector<ttstr>::iterator i =
		std::find(TVPAutoPathList.begin(), TVPAutoPathList.end(), normalized);
	if(i == TVPAutoPathList.end())
		TVPAutoPathList.push_back(normalized);

#ifdef __EMSCRIPTEN__
	ttstr mirror;
	if(TVPResolveSiblingArchiveAutoPath(normalized, mirror))
	{
		i = std::find(TVPAutoPathList.begin(), TVPAutoPathList.end(), mirror);
		if(i == TVPAutoPathList.end())
		{
			TVPAutoPathList.push_back(mirror);
			std::string src, dst;
			TVPUtf16ToUtf8(src, normalized.AsStdString());
			TVPUtf16ToUtf8(dst, mirror.AsStdString());
			KRKR_LOG_L2("[AUTO-PATH] sibling-archive source=%s mirror=%s\n",
				src.c_str(), dst.c_str());
		}
	}
#endif

	TVPClearAutoPathCache();
}
//---------------------------------------------------------------------------
void TVPRemoveAutoPath(const ttstr &name)
{
	tTJSCriticalSectionHolder cs_holder(TVPCreateStreamCS);

	tjs_char lastchar = name.GetLastChar();
	if(lastchar != TVPArchiveDelimiter && lastchar != TJS_W('/') && lastchar != TJS_W('\\'))
		TVPThrowExceptionMessage(TVPMissingPathDelimiterAtLast);

	ttstr normalized = TVPNormalizeStorageName(name);

	std::vector<ttstr>::iterator i =
		std::find(TVPAutoPathList.begin(), TVPAutoPathList.end(), normalized);
	if(i != TVPAutoPathList.end())
		TVPAutoPathList.erase(i);

	TVPClearAutoPathCache();
}
//---------------------------------------------------------------------------
static tjs_uint TVPRebuildAutoPathTable()
{
	// rebuild auto path table
	if(AutoPathTableInit) return 0;

	TVPInitStorageOptions();

	tTJSCriticalSectionHolder cs_holder(TVPCreateStreamCS);

	TVPAutoPathTable.Clear();

	tjs_uint64 tick = TVPGetTickCount();
	if(TVPLogL2()) TVPAddLog( (const tjs_char*)TVPInfoRebuildingAutoPath );

	tjs_uint totalcount = 0;

	std::vector<ttstr>::iterator it;
	for(it = TVPAutoPathList.begin(); it != TVPAutoPathList.end(); it++)
	{
		const ttstr & path = *it;
		tjs_uint count = 0;

		std::string utf8path;
		TVPUtf16ToUtf8(utf8path, path.AsStdString());
		KRKR_LOG_L2("AutoPath checking: %s\n", utf8path.c_str());

		const tjs_char * sharp_pos = TJS_strchr(path.c_str(), TVPArchiveDelimiter);
		if(sharp_pos)
		{
			// this storagename indicates a file in an archive

			ttstr arcname(path, (int)(sharp_pos - path.c_str()));
			ttstr in_arc_name(sharp_pos + 1);
			tTVPArchive::NormalizeInArchiveStorageName(in_arc_name);
			tjs_int in_arc_name_len = in_arc_name.GetLen();

			tTVPArchive *arc;
			arc = TVPArchiveCache.Get(arcname);

			try
			{
				tjs_uint storagecount = arc->GetCount();

				// get first index which the item has 'in_arc_name' as its start
				// of the string.
				tjs_int i = arc->GetFirstIndexStartsWith(in_arc_name);
				if(i != -1)
				{
					for(; i < (tjs_int)storagecount; i++)
					{
						ttstr name = arc->GetName(i);

						if(name.StartsWith(in_arc_name))
						{
							if(!TJS_strchr(name.c_str() + in_arc_name_len, TJS_W('/')))
							{
								ttstr sname = TVPExtractStorageName(name);
								sname.ToLowerCase();
								// TODO アーカイブの時もプロパティ情報追加
								TVPAutoPathTable.Add(sname, tTVPFileInfo(path) );
								count ++;
							}
						}
						else
						{
							// no need to check more;
							// because the list is sorted by the name.
							break;
						}
					}
				}
			}
			catch(...)
			{
				arc->Release();
				throw;
			}
			arc->Release();
		}
		else
		{
			// normal folder
			class tLister : public iTVPStorageLister
			{
				const ttstr EXT;
			public:
				tLister() : EXT(TJS_W(".prop")) {}
				std::set<ttstr>		list;
				std::vector<ttstr>	prop;
				void TJS_INTF_METHOD Add(const ttstr &file)
				{
					ttstr ext = TVPExtractStorageExt( file );
					if( ext == EXT )
					{
						prop.push_back( file );
					}
					list.insert( file );
				}
			} lister;
			TVPStorageMediaManager.GetListAt(path, &lister);

			if( !TVPIgnoreFileProperty )
			{
				// プロパティがあるファイルを追加する
				for( auto i = lister.prop.begin(); i != lister.prop.end(); i++ ) {
					// プロパティがある場合はとりあえず登録だけしておき、プロパティ取得時に実際に読み込みを行う
					ttstr fname = TVPChopStorageExt( *i );
					auto file = lister.list.find( fname );
					if( file != lister.list.end() ) {
						ttstr sname = *file;
						sname.ToLowerCase();
						// ファイルがある場合
						lister.list.erase( sname );
						TVPAutoPathTable.Add( sname, tTVPFileInfo( path, tTVPFileInfo::EXIST_PROP ) );
					} else {
						ttstr sname = fname;
						sname.ToLowerCase();
						TVPAutoPathTable.Add( sname, tTVPFileInfo( path, tTVPFileInfo::EXIST_PROP | tTVPFileInfo::EMPTY_FILE ) );
					}
				}
			}
			// プロパティのないファイルを追加する
			for( auto i = lister.list.begin(); i != lister.list.end(); i++)
			{
				ttstr sname = *i;
				sname.ToLowerCase();
				TVPAutoPathTable.Add(sname, tTVPFileInfo(path) );
				count ++;
			}
		}

//		TVPAddLog(ttstr(TJS_W("(info) Path ")) + path + TJS_W(" contains ") +
//			ttstr((tjs_int)count) + TJS_W(" file(s)."));

		totalcount += count;
	}

	tjs_uint64 endtick = TVPGetTickCount();

	if(TVPLogL2()) {
		TVPAddLog(ttstr(TJS_W("(info) Total ")) +
				ttstr((tjs_int)totalcount) + TJS_W(" file(s) found, ") +
				ttstr((tjs_int)TVPAutoPathTable.GetCount()) + TJS_W(" file(s) activated.") +
				TJS_W(" (") + ttstr((tjs_int)(endtick - tick)) + TJS_W("ms)"));
	}

	AutoPathTableInit = true;

	return totalcount;
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// TVPGetPlacedPath
//---------------------------------------------------------------------------
ttstr TVPGetPlacedPath(const ttstr & name)
{
	// search path and return the path which the "name" is placed.
	// returned name is normalized. returns empty string if the storage is not
	// found.
	bool trace_font_storage = TVPTraceFontStorageName(name) && TVPLogL3();
#ifdef __EMSCRIPTEN__
	bool trace_psb_storage = TVPTracePsbStorageName(name) && TVPLogL3();
	if(trace_psb_storage)
	{
		std::string raw = TVPTraceStorageNameUtf8(name);
		fprintf(stderr, "[VFS-PLACE] enter name=%s\n", raw.c_str());
		fflush(stderr);
	}
#endif
	if(trace_font_storage)
	{
		std::string raw;
		TVPUtf16ToUtf8(raw, name.AsStdString());
		fprintf(stderr, "[PLACE-FONT] query=%s\n", raw.c_str());
		fflush(stderr);
	}
	if(!TVPClearAutoPathCacheCallbackInit)
	{
		TVPAddCompactEventHook(&TVPClearAutoPathCacheCallback);
		TVPClearAutoPathCacheCallbackInit = true;
	}

	ttstr * incache = TVPAutoPathCache.FindAndTouch(name);
	if(incache)
	{
#ifdef __EMSCRIPTEN__
		if(trace_psb_storage)
		{
			std::string cached = TVPTraceStorageNameUtf8(*incache);
			fprintf(stderr, "[VFS-PLACE] cache-hit name=%s result=%s\n",
				TVPTraceStorageNameUtf8(name).c_str(), cached.c_str());
			fflush(stderr);
		}
#endif
		if(trace_font_storage)
		{
			std::string cached;
			TVPUtf16ToUtf8(cached, incache->AsStdString());
			fprintf(stderr, "[PLACE-FONT] cache-hit result=%s\n", cached.c_str());
			fflush(stderr);
		}
		return *incache; // found in cache
	}

	tTJSCriticalSectionHolder cs_holder(TVPCreateStreamCS);

	ttstr normalized(TVPNormalizeStorageName(name));
#ifdef __EMSCRIPTEN__
	if(trace_psb_storage)
	{
		std::string norm = TVPTraceStorageNameUtf8(normalized);
		fprintf(stderr, "[VFS-PLACE] normalized name=%s normalized=%s\n",
			TVPTraceStorageNameUtf8(name).c_str(), norm.c_str());
		fflush(stderr);
	}
#endif
	if(trace_font_storage)
	{
		std::string norm;
		TVPUtf16ToUtf8(norm, normalized.AsStdString());
		fprintf(stderr, "[PLACE-FONT] normalized=%s\n", norm.c_str());
		fflush(stderr);
	}

	bool found = TVPIsExistentStorageNoSearchNoNormalize(normalized);
	if(found)
	{
		// found in current folder
		TVPAutoPathCache.Add(name, normalized);
#ifdef __EMSCRIPTEN__
		if(trace_psb_storage)
		{
			std::string norm = TVPTraceStorageNameUtf8(normalized);
			fprintf(stderr, "[VFS-PLACE] current-hit name=%s result=%s\n",
				TVPTraceStorageNameUtf8(name).c_str(), norm.c_str());
			fflush(stderr);
		}
#endif
		if(trace_font_storage)
		{
			std::string norm;
			TVPUtf16ToUtf8(norm, normalized.AsStdString());
			fprintf(stderr, "[PLACE-FONT] current-hit result=%s\n", norm.c_str());
			fflush(stderr);
		}
		return normalized;
	}

	// not found in current folder
	// search through auto path table

	ttstr storagename = TVPExtractStorageName(normalized);
	storagename.ToLowerCase();

#ifdef __EMSCRIPTEN__
	if(trace_psb_storage)
	{
		fprintf(stderr, "[VFS-PLACE] before-autopath name=%s storage=%s\n",
			TVPTraceStorageNameUtf8(name).c_str(),
			TVPTraceStorageNameUtf8(storagename).c_str());
		fflush(stderr);
	}
#endif
	TVPRebuildAutoPathTable(); // ensure auto path table
#ifdef __EMSCRIPTEN__
	if(trace_psb_storage)
	{
		fprintf(stderr, "[VFS-PLACE] after-autopath name=%s storage=%s\n",
			TVPTraceStorageNameUtf8(name).c_str(),
			TVPTraceStorageNameUtf8(storagename).c_str());
		fflush(stderr);
	}
#endif
	tTVPFileInfo *result = TVPAutoPathTable.Find(storagename);
	if(result && (result->Flag & tTVPFileInfo::EMPTY_FILE) == 0 )
	{
		// found in table
		ttstr found = result->FilePath + storagename;
		TVPAutoPathCache.Add(name, found);
#ifdef __EMSCRIPTEN__
		if(trace_psb_storage)
		{
			fprintf(stderr, "[VFS-PLACE] autopath-hit name=%s result=%s\n",
				TVPTraceStorageNameUtf8(name).c_str(),
				TVPTraceStorageNameUtf8(found).c_str());
			fflush(stderr);
		}
#endif
		if(trace_font_storage)
		{
			std::string placed;
			TVPUtf16ToUtf8(placed, found.AsStdString());
			fprintf(stderr, "[PLACE-FONT] autop-hit storage=%s result=%s\n",
				storagename.AsNarrowStdString().c_str(), placed.c_str());
			fflush(stderr);
		}
		return found;
	}

	// not found
	TVPAutoPathCache.Add(name, ttstr());
#ifdef __EMSCRIPTEN__
	if(trace_psb_storage)
	{
		fprintf(stderr, "[VFS-PLACE] miss name=%s\n",
			TVPTraceStorageNameUtf8(name).c_str());
		fflush(stderr);
	}
#endif
	if(trace_font_storage)
	{
		fprintf(stderr, "[PLACE-FONT] miss\n");
		fflush(stderr);
	}
	return ttstr();
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
/**
 * TVPGetPlacedPath
 * @param name file name
 * @param extlist extension list(delimiter |) ex. ".bmp|.png|.jpg"
 * @return normalized path.
 */
ttstr TVPGetPlacedPath(const ttstr & name, const ttstr& extlist )
{
	tjs_string exts = extlist.AsStdString();
	std::vector<tjs_string> ext;
	split( exts, tjs_string( TJS_W( "|" ) ), ext );
	ttstr filename = TVPChopStorageExt( name );
	for( auto i = ext.begin(); i != ext.end(); i++ ) {
		if( !((*i).empty()) ) {
			ttstr fullname = filename + ttstr( *i );
			ttstr ret = TVPGetPlacedPath( fullname );
			if( !ret.IsEmpty() ) {
				return ret;
			}
		}
	}
	return ttstr();
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// TVPSearchPlacedPath
//---------------------------------------------------------------------------
ttstr TVPSearchPlacedPath(const ttstr & name)
{
	ttstr place = TVPGetPlacedPath(name);
	if(place.IsEmpty()) TVPThrowExceptionMessage(TVPCannotFindStorage, name);
	return place;
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// TVPIsExistentStorage
//---------------------------------------------------------------------------
bool TVPIsExistentStorage(const ttstr &name)
{
#ifdef __EMSCRIPTEN__
	// [PLAN-B+] Force-yes for plugin DLL existence checks so that
	// Initialize.tjs's CanLoadPlugin('motionplayer.dll') returns true
	// → KAGLoadScript('motion.tjs') runs → Motion.* TJS classes register
	// → MotionAffineSourceLayer can be instantiated → game falls into
	// the "no native motionplayer but TJS-side fallback" path that
	// Kirikiroid2 Android relies on.
	{
		std::string sn = name.AsNarrowStdString();
		// Match anything ending with motionplayer.dll / psbfile.dll, regardless of dir prefix.
		const char *targets[] = {
			"motionplayer.dll", "motionplayer_nod3d.dll",
			"psbfile.dll", "yuzuex.dll", "kagexopt.dll",
			"layerExDraw.dll", "menu.dll",
			NULL
		};
		for (int i = 0; targets[i]; i++) {
			size_t tl = strlen(targets[i]);
			if (sn.size() >= tl && sn.compare(sn.size() - tl, tl, targets[i]) == 0) {
				bool real = !TVPGetPlacedPath(name).IsEmpty();
				fprintf(stderr, "[STORAGE-FAKE] isExistent('%s') real=%d -> forced=true\n",
					sn.c_str(), (int)real);
				fflush(stderr);
				return true;
			}
		}
	}
#endif
	return !TVPGetPlacedPath(name).IsEmpty();
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// TVPGetFilePropertyNoAddRef
//---------------------------------------------------------------------------
iTJSDispatch2* TVPGetFilePropertyNoAddRef( const ttstr& name )
{
	TVPRebuildAutoPathTable(); // ensure auto path table
	tTVPFileInfo *result = TVPAutoPathTable.Find( name );
	if( result && ( result->Flag & tTVPFileInfo::EXIST_PROP) ) {
		// found in table
		if( !result->Property ) {
			ttstr path = result->FilePath + name + ".prop";
			tTJSVariant dic;
			ttstr mode;
			if( TJSReadDictionaryObject( dic, path, mode ) == TJS_S_OK ) {
				result->Property = dic.AsObject();
			}
		}
		return result->Property;
	}
	return nullptr;
}


//---------------------------------------------------------------------------
// TVPCreateStream
//---------------------------------------------------------------------------
static tTJSBinaryStream * _TVPCreateStream(const ttstr & _name, tjs_uint32 flags)
{
	if ((!TJS_strncmp(_name.c_str(), TJS_W("http://"), 7) || !TJS_strncmp(_name.c_str(), TJS_W("https://"), 8)) && !TJS_strchr(_name.c_str(), TJS_W('>'))) {
		extern tTJSBinaryStream * TVPCreateHTTPStream(const ttstr& url);
		return TVPCreateHTTPStream(_name);
	}

#ifdef __EMSCRIPTEN__
	bool trace_psb_storage = TVPTracePsbStorageName(_name) && TVPLogL3();
	if(trace_psb_storage)
	{
		fprintf(stderr, "[VFS-STREAM] enter name=%s flags=%u\n",
			TVPTraceStorageNameUtf8(_name).c_str(), (unsigned)flags);
		fflush(stderr);
	}
#endif
	tTJSCriticalSectionHolder cs_holder(TVPCreateStreamCS);

	ttstr name;

	tjs_uint32 access = flags & TJS_BS_ACCESS_MASK;
	if(access == TJS_BS_WRITE)
		name = TVPNormalizeStorageName(_name);
	else
		name = TVPGetPlacedPath(_name); // file must exist

#ifdef __EMSCRIPTEN__
	if(trace_psb_storage)
	{
		fprintf(stderr, "[VFS-STREAM] placed input=%s placed=%s access=%u\n",
			TVPTraceStorageNameUtf8(_name).c_str(),
			TVPTraceStorageNameUtf8(name).c_str(),
			(unsigned)access);
		fflush(stderr);
	}
#endif
	if(name.IsEmpty()) TVPThrowExceptionMessage(TVPCannotOpenStorage, _name);

	// does name contain > ?
	const tjs_char * sharp_pos = TJS_strchr(name.c_str(), TVPArchiveDelimiter);
	if(sharp_pos)
	{
		// this storagename indicates a file in an archive
		if((flags & TJS_BS_ACCESS_MASK ) !=TJS_BS_READ)
			TVPThrowExceptionMessage(TVPCannotWriteToArchive);

		ttstr arcname(name, (int)(sharp_pos - name.c_str()));

		tTVPArchive *arc;
		tTJSBinaryStream *stream;
#ifdef __EMSCRIPTEN__
		if(trace_psb_storage)
		{
			fprintf(stderr, "[VFS-STREAM] archive-get-begin arc=%s\n",
				TVPTraceStorageNameUtf8(arcname).c_str());
			fflush(stderr);
		}
#endif
		arc = TVPArchiveCache.Get(arcname);
#ifdef __EMSCRIPTEN__
		if(trace_psb_storage)
		{
			fprintf(stderr, "[VFS-STREAM] archive-get-done arc=%s ptr=%p\n",
				TVPTraceStorageNameUtf8(arcname).c_str(), (void*)arc);
			fflush(stderr);
		}
#endif
		try
		{
			ttstr in_arc_name(sharp_pos + 1);
			tTVPArchive::NormalizeInArchiveStorageName(in_arc_name);
#ifdef __EMSCRIPTEN__
			if(trace_psb_storage)
			{
				fprintf(stderr, "[VFS-STREAM] archive-create-begin arc=%s item=%s\n",
					TVPTraceStorageNameUtf8(arcname).c_str(),
					TVPTraceStorageNameUtf8(in_arc_name).c_str());
				fflush(stderr);
			}
#endif
			stream = arc->CreateStream(in_arc_name);
#ifdef __EMSCRIPTEN__
			if(trace_psb_storage)
			{
				fprintf(stderr, "[VFS-STREAM] archive-create-done arc=%s item=%s stream=%p\n",
					TVPTraceStorageNameUtf8(arcname).c_str(),
					TVPTraceStorageNameUtf8(in_arc_name).c_str(),
					(void*)stream);
				fflush(stderr);
			}
#endif
		}
		catch(...)
		{
			arc->Release();
			if(access >= 1) TVPClearStorageCaches();
			throw;
		}
		if(access >= 1) TVPClearStorageCaches();
		arc->Release();
		return stream;
	}

	tTJSBinaryStream *stream;
	try
	{
		stream = TVPStorageMediaManager.Open(name, flags);
	}
	catch(...)
	{
		if(access >= 1) TVPClearStorageCaches();
		throw;
	}
	if(access >= 1) TVPClearStorageCaches();
	return stream;
}

tTJSBinaryStream * TVPCreateStream(const ttstr & _name, tjs_uint32 flags)
{
#ifdef __EMSCRIPTEN__
	{
		std::string n = _name.AsNarrowStdString();
		if (TVPLogL3() && (n.find(".psb") != std::string::npos || n.find(".PSB") != std::string::npos)) {
			KRKR_LOG_L3("[VFS-PSB] TVPCreateStream: %s flags=%u\n", n.c_str(), (unsigned)flags);
		}
	}
#endif
	try
	{
		return _TVPCreateStream(_name, flags);
	}
	catch(eTJSScriptException &e)
	{
		if(TJS_strchr(_name.c_str(), '#'))
			e.AppendMessage(TJS_W("[") +
				TVPFormatMessage(TVPFilenameContainsSharpWarn, _name) + TJS_W("]"));
		throw e;
	}
	catch(eTJSScriptError &e)
	{
		if(TJS_strchr(_name.c_str(), '#'))
			e.AppendMessage(TJS_W("[") +
				TVPFormatMessage(TVPFilenameContainsSharpWarn, _name) + TJS_W("]"));
		throw e;
	}
	catch(eTJSError &e)
	{
		if(TJS_strchr(_name.c_str(), '#'))
			e.AppendMessage(TJS_W("[") +
				TVPFormatMessage(TVPFilenameContainsSharpWarn, _name) + TJS_W("]"));
		throw e;
	}
	catch(...)
	{
		// check whether the filename contains '#' (former delimiter for archive
		// filename before 2.19 beta 14)
		if(TJS_strchr(_name.c_str(), '#'))
			TVPAddLog(TVPFormatMessage(TVPFilenameContainsSharpWarn, _name));
		throw;
	}
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// TVPClearStorageCaches
//---------------------------------------------------------------------------
void TVPClearStorageCaches()
{
	// clear all storage related caches
	TVPClearXP3SegmentCache();
	TVPClearAutoPathCache();
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// tTJSNC_Storages
//---------------------------------------------------------------------------
tjs_uint32 tTJSNC_Storages::ClassID = -1;
tTJSNC_Storages::tTJSNC_Storages() : inherited(TJS_W("Storages"))
{
	// registration of native members

	TJS_BEGIN_NATIVE_MEMBERS(Storages)
	TJS_DECL_EMPTY_FINALIZE_METHOD
//----------------------------------------------------------------------

//-- methods

//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/addAutoPath)
{
	if(numparams < 1) return TJS_E_BADPARAMCOUNT;

	ttstr path = *param[0];

	TVPAddAutoPath(path);

	if(result) result->Clear();

	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/addAutoPath)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/removeAutoPath)
{
	if(numparams < 1) return TJS_E_BADPARAMCOUNT;

	ttstr path = *param[0];

	TVPRemoveAutoPath(path);

	if(result) result->Clear();

	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/removeAutoPath)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/getFullPath)
{
	if(numparams < 1) return TJS_E_BADPARAMCOUNT;

	ttstr path = *param[0];

	if(result)
		*result = TVPNormalizeStorageName(path);

	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/getFullPath)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/getPlacedPath)
{
	if(numparams < 1) return TJS_E_BADPARAMCOUNT;

	ttstr path = *param[0];
	if(result)
	{
		if( numparams >= 2 )
		{
			ttstr ext = *param[1];
			*result = TVPGetPlacedPath( path, ext );
		}
		else
		{
			*result = TVPGetPlacedPath( path );
		}
	}

	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/getPlacedPath)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/isExistentStorage)
{
	if(numparams < 1) return TJS_E_BADPARAMCOUNT;

	ttstr path = *param[0];

	bool exists = TVPIsExistentStorage(path);
	{
		std::string sn = path.AsNarrowStdString();
		if(sn.find(".stand") != std::string::npos || sn.find(".stage") != std::string::npos ||
			sn.find("_info.txt") != std::string::npos)
			KRKR_LOG_L2("[STG-EXIST] path=%s exists=%d\n", sn.c_str(), (int)exists);
	}
	if(result)
		*result = (tjs_int)exists;

	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/isExistentStorage)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/extractStorageExt)
{
	if(numparams < 1) return TJS_E_BADPARAMCOUNT;

	ttstr path = *param[0];

	if(result)
		*result = TVPExtractStorageExt(path);

	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/extractStorageExt)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/extractStorageName)
{
	if(numparams < 1) return TJS_E_BADPARAMCOUNT;

	ttstr path = *param[0];

	if(result)
		*result = TVPExtractStorageName(path);

	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/extractStorageName)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/extractStoragePath)
{
	if(numparams < 1) return TJS_E_BADPARAMCOUNT;

	ttstr path = *param[0];

	if(result)
		*result = TVPExtractStoragePath(path);

	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/extractStoragePath)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/chopStorageExt)
{
	if(numparams < 1) return TJS_E_BADPARAMCOUNT;

	ttstr path = *param[0];

	if(result)
		*result = TVPChopStorageExt(path);

	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/chopStorageExt)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/clearArchiveCache)
{
	TVPClearArchiveCache();
	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/clearArchiveCache)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/getFileProperty) {
	if( numparams < 1 ) return TJS_E_BADPARAMCOUNT;

	ttstr path = *param[0];
	if( result ) {
		iTJSDispatch2* dic = TVPGetFilePropertyNoAddRef( path );
		*result = tTJSVariant( dic, dic );
	}
	return TJS_S_OK;
}
TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/getFileProperty )
//----------------------------------------------------------------------
	TJS_END_NATIVE_MEMBERS
}
//---------------------------------------------------------------------------
tTJSNativeInstance * tTJSNC_Storages::CreateNativeInstance()
{
	// this class cannot create an instance
	TVPThrowExceptionMessage(TVPCannotCreateInstance);

	return NULL;
}
//---------------------------------------------------------------------------



