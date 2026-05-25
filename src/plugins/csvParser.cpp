#include "tp_stub.h"
#include <string>
#include "ncbind/ncbind.hpp"

#define NCB_MODULE_NAME TJS_W("csvParser.dll")
using namespace std;

//---------------------------------------------------------------------------

// Array �N���X�����o
static iTJSDispatch2 *ArrayClearMethod   = NULL;   // Array.clear

// -----------------------------------------------------------------

class IFile {
public:
	virtual ~IFile() {};
	virtual bool addNextLine(ttstr &str) = 0;
};

class IFileStr : public IFile {

	ttstr dat;
	uint32_t pos;

public:
	IFileStr(const ttstr &str) {
		dat = str;
		pos = 0;
	}

	int getc() {
		return pos < (uint32_t)dat.length() ? dat[pos++] : EOF;
	}

	void ungetc() {
		if (pos > 0) {
			pos--;
		}
	}

	bool eof() {
		return pos >= (uint32_t)dat.length();
	}

	/**
	 * ���s�`�F�b�N
	 */
	bool endOfLine(tjs_char c) {
		bool eol = (c =='\r' || c == '\n');
		if (c == '\r'){
			c = getc();
			if (!eof() && c != '\n') {
				ungetc();
			}
		}
		return eol;
	}

	bool addNextLine(ttstr &str) {
		int l = 0;
		int c;
		while ((c = getc()) != EOF && !endOfLine(c)) {
			str += c;
			l++;
		}
		if (l > 0 || c != EOF) {
			return true;
		} else {
			return false;
		}
	}
};

// -----------------------------------------------------------------

static void
addMember(iTJSDispatch2 *dispatch, const tjs_char *name, iTJSDispatch2 *member)
{
	tTJSVariant var (member);
	member->Release();
	dispatch->PropSet(
		TJS_MEMBERENSURE, // �����o���Ȃ������ꍇ�ɂ͍쐬����悤�ɂ���t���O
		name, // �����o�� ( ���Ȃ炸 TJS_W( ) �ň͂� )
		NULL, // �q���g ( �{���̓����o���̃n�b�V���l�����ANULL �ł��悢 )
		&var, // �o�^����l
		dispatch // �R���e�L�X�g
		);
}

static iTJSDispatch2*
getMember(iTJSDispatch2 *dispatch, const tjs_char *name)
{
	tTJSVariant val;
	if (TJS_FAILED(dispatch->PropGet(TJS_IGNOREPROP,
									 name,
									 NULL,
									 &val,
									 dispatch))) {
		ttstr msg = TJS_W("can't get member:");
		msg += name;
		TVPThrowExceptionMessage(msg.c_str());
	}
	return val.AsObject();
}

static bool
isValidMember(iTJSDispatch2 *dispatch, const tjs_char *name)
{
	return dispatch->IsValid(TJS_IGNOREPROP,
							 name,
							 NULL,
							 dispatch) == TJS_S_TRUE;
}

static void
delMember(iTJSDispatch2 *dispatch, const tjs_char *name)
{
	dispatch->DeleteMember(
		0, // �t���O ( 0 �ł悢 )
		name, // �����o��
		NULL, // �q���g
		dispatch // �R���e�L�X�g
		);
}
//---------------------------------------------------------------------------

#ifdef TJS_NATIVE_CLASSID_NAME
#undef TJS_NATIVE_CLASSID_NAME
#undef TJS_NCM_REG_THIS
#undef TJS_NATIVE_SET_ClassID
#endif
#define TJS_NCM_REG_THIS classobj
#define TJS_NATIVE_SET_ClassID TJS_NATIVE_CLASSID_NAME = TJS_NCM_CLASSID;
#define TJS_NATIVE_CLASSID_NAME ClassID_CSVParser
static tjs_int32 TJS_NATIVE_CLASSID_NAME = -1;

/**
 * CSVParser
 */
class NI_CSVParser : public tTJSNativeInstance // �l�C�e�B�u�C���X�^���X
{
protected:
	iTJSDispatch2 *target;
	IFile *file;
	tjs_int32 lineNo;

	// ��؂蕶��
	tjs_char separator;

	// ���s����
	ttstr newline;
	
	// �s���(���C�h�L�����ŏ�������)
	ttstr line;
	
	bool addline() {
		return file->addNextLine(line);
	}
	
	// ����������
	int find(ttstr &line, tjs_char ch, int start) {
		int i;
		for (i=start; i < line.length(); i++) {
			if (ch == line[i]) {
				return i;
			}
		}
		return i;
	}

	// ��������
	void split(iTJSDispatch2 *fields) {

		ttstr fld;
		int i, j;
		tjs_int cnt = 0;
	
		if (line.length() == 0) {
			return;
		}
		i = 0;
		do {
			if (i < line.length() && line[i] == '"') {
				++i;
				fld = TJS_W("");
				j = i;
				do {
					for (;j < line.length(); j++){
						if (line[j] == '"' && line[++j] != '"'){
							int k = find(line, separator, j);
							if (k > line.length()) {
								k = line.length();
							}
							for (k -= j; k-- > 0;)
								fld += line[j++];
							goto next;
						}
						fld += line[j];
					}
					// ���s�ǉ�����
					fld += newline;
				} while (addline());
			} else {
				j = find(line, separator, i);
				if (j > line.length()) {
					j = line.length();
				}
				fld = ttstr(line.c_str() + i, j-i);
			}
		next:
			{
				// �o�^
				tTJSVariant var(fld);
				fields->PropSetByNum(TJS_MEMBERENSURE, cnt++, &var, fields);
			}
			i = j + 1;
		} while (j < line.length());
	}
	
public:

	/**
	 * �R���X�g���N�^
	 */
	NI_CSVParser() {
		target = NULL;
		file = NULL;
		lineNo = 0;
		separator = ',';
		newline = TJS_W("\r\n");
	}

	~NI_CSVParser() {
		clear();
	}

	/**
	 * TJS �R���X�g���N�^
	 * @param numparams �p�����[�^��
	 * @param param
	 * @param tjs_obj this �I�u�W�F�N�g
	 */
	tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) {
		if (numparams > 0) {
			target = param[0]->AsObject();
			if (numparams > 1) {
				separator = (tjs_int)*param[1];
				if (numparams > 2) {
					newline = *param[2];
				}
			}
		}
		return TJS_S_OK;
	}

	/**
	 * �t�@�C���N���[�Y����
	 */
	void clear() {
		if (file) {
			delete file;
			file = NULL;
		}
	}
	
	/**
	 * TJS invalidate
	 */
	void TJS_INTF_METHOD Invalidate() {
		clear();
		if (target) {
			target->Release();
			target = NULL;
		}
	}

	/**
	 * �p�[�T�̏���������
	 */
	void init(tTJSVariantString *text) {
		clear();
		file = new IFileStr(text);
		lineNo = 0;
	}

	/**
	 * ����������
	 */
	void initStorage(tTJSVariantString *filename, bool utf8=false) {
		clear();
		ttstr text;
		if (utf8) {
			tTJSBinaryStream* pStream = TVPCreateStream(filename, TJS_BS_READ);
			tjs_uint streamsize = pStream->GetSize();
			char *buff = new char[streamsize + 1];
			buff[streamsize] = 0;
			pStream->ReadBuffer(buff, streamsize);
			text = buff;
			delete []buff;
			delete pStream;
		} else {
			iTJSTextReadStream *pStream = TVPCreateTextStreamForRead(filename, TJS_W(""));
			pStream->Read(text, 0);
			delete pStream;
		}
		file = new IFileStr(text);
		lineNo = 0;
	}


	// 1�s�ǂݏo��
	bool getNextLine(tTJSVariant *result = NULL) {
		bool ret = false;
		if (file) {
			line = TJS_W("");
			if (addline()) {
				lineNo++;
				iTJSDispatch2 *fields = TJSCreateArrayObject();
				split(fields);
				if (result) {
					result->SetObject(fields,fields);
				}
				fields->Release();
				ret = true;
			} else {
				clear();
			}
		}
		return ret;
	}
	
	/**
	 * ���݂̍s�ԍ��̎擾
	 * @return �s�ԍ�
	 */
	tjs_int32 getLineNumber() {
		return lineNo;
	}
	
	/**
	 * �p�[�X�̎��s
	 */
	void parse(iTJSDispatch2 *objthis) {
		iTJSDispatch2 *target = this->target ? this->target : objthis;
		if (file && isValidMember(target, TJS_W("doLine"))) {
			iTJSDispatch2 *method = getMember(target, TJS_W("doLine"));
			tTJSVariant result;
			while (getNextLine(&result)) {
				tTJSVariant var2 (lineNo);
				tTJSVariant *vars[2];
				vars[0] = &result;
				vars[1] = &var2;
				method->FuncCall(0, NULL, NULL, NULL, 2, vars, target);
			}
			clear();
			method->Release();
		}
	}

};

static iTJSNativeInstance * TJS_INTF_METHOD Create_NI_CSVParser()
{
	return new NI_CSVParser();
}

static iTJSDispatch2 * Create_NC_CSVParser()
{
	tTJSNativeClassForPlugin * classobj = TJSCreateNativeClassForPlugin(TJS_W("CSVParser"), Create_NI_CSVParser);

	TJS_BEGIN_NATIVE_MEMBERS(/*TJS class name*/CSVParser)

		TJS_DECL_EMPTY_FINALIZE_METHOD

		TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(
			/*var.name*/_this,
			/*var.type*/NI_CSVParser,
			/*TJS class name*/CSVParser)
		{
			return TJS_S_OK;
		}
		TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/CSVParser)

		TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/init)
		{
			TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/NI_CSVParser);
			if (numparams < 1) return TJS_E_BADPARAMCOUNT;
			_this->init(param[0]->AsStringNoAddRef());
			return TJS_S_OK;
		}
		TJS_END_NATIVE_METHOD_DECL(/*func. name*/init)

		TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/initStorage)
		{
			TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/NI_CSVParser);
			if (numparams < 1) return TJS_E_BADPARAMCOUNT;
			_this->initStorage(param[0]->AsStringNoAddRef(), numparams > 1 && (tjs_int)*param[1] != 0);
			return TJS_S_OK;
		}
		TJS_END_NATIVE_METHOD_DECL(/*func. name*/initStorage)
			
		TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/getNextLine)
		{
			TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/NI_CSVParser);
			_this->getNextLine(result);
			return TJS_S_OK;
		}
		TJS_END_NATIVE_METHOD_DECL(/*func. name*/getNextLine)
			
		TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/parse)
		{
			TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/NI_CSVParser);
			if (numparams > 0) {
				_this->init(param[0]->AsStringNoAddRef());
			}
			_this->parse(objthis);
			return TJS_S_OK;
		}
		TJS_END_NATIVE_METHOD_DECL(/*func. name*/parse)

		TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/parseStorage)
		{
			TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/NI_CSVParser);
			if (numparams > 0) {
				_this->initStorage(param[0]->AsStringNoAddRef(), numparams > 1 && (tjs_int)*param[1] != 0);
			}
			_this->parse(objthis);
			return TJS_S_OK;
		}
		TJS_END_NATIVE_METHOD_DECL(/*func. name*/parseStorage)

		TJS_BEGIN_NATIVE_PROP_DECL(currentLineNumber)
		{
			TJS_BEGIN_NATIVE_PROP_GETTER
			{
				TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,	/*var. type*/NI_CSVParser);
				*result = _this->getLineNumber();
				return TJS_S_OK;
			}
			TJS_END_NATIVE_PROP_GETTER
			TJS_DENY_NATIVE_PROP_SETTER
		}
		TJS_END_NATIVE_PROP_DECL(currentLineNumber)

	TJS_END_NATIVE_MEMBERS

	// �萔�̓o�^

	/*
	 * ���̊֐��� classobj ��Ԃ��܂��B
	 */
	return classobj;
}

void InitPlugin_CSVParser() {
    // TJS �̃O���[�o���I�u�W�F�N�g���擾����
    iTJSDispatch2 * global = TVPGetScriptDispatch();

    if (global) {

        // Arary �N���X�����o�[�擾
        {
            tTJSVariant varScripts;
            TVPExecuteExpression(TJS_W("Array"), &varScripts);
            iTJSDispatch2 *dispatch = varScripts.AsObjectNoAddRef();
            // �����o�擾
            ArrayClearMethod = getMember(dispatch, TJS_W("clear"));
        }

        addMember(global, TJS_W("CSVParser"), Create_NC_CSVParser());
        global->Release();
    }
}
//---------------------------------------------------------------------------
// extern "C" __declspec(dllexport) HRESULT _stdcall V2Unlink()
// {
// 	// - �܂��ATJS �̃O���[�o���I�u�W�F�N�g���擾����
// 	iTJSDispatch2 * global = TVPGetScriptDispatch();
// 
// 	// - global �� DeleteMember ���\�b�h��p���A�I�u�W�F�N�g���폜����
// 	if (global)	{
// 		delMember(global, L"CSVParser");
// 		if (ArrayClearMethod) {
// 			ArrayClearMethod->Release();
// 			ArrayClearMethod = NULL;
// 		}
// 		global->Release();
// 	}
// }

NCB_PRE_REGIST_CALLBACK(InitPlugin_CSVParser);
