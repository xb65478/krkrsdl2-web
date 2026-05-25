#include "tjsCommHead.h"
#include "tjsArray.h"
#include "tjsDictionary.h"
#include "tjsVariant.h"
#include "tjsNative.h"
#include "StorageIntf.h"
#include "MsgIntf.h"
#include "ScriptMgnIntf.h"
#include "UtilStreams.h"
#include "CharacterSet.h"

#include <algorithm>
#include <climits>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <map>
#include <string>
#include <vector>
#include <zlib.h>

class tTJSNI_PSBFile;

namespace PSB
{
	enum PSBObjType : unsigned char
	{
		None = 0x0,
		Null = 0x1,
		False = 0x2,
		True = 0x3,

		NumberN0 = 0x4,
		NumberN1 = 0x5,
		NumberN2 = 0x6,
		NumberN3 = 0x7,
		NumberN4 = 0x8,
		NumberN5 = 0x9,
		NumberN6 = 0xA,
		NumberN7 = 0xB,
		NumberN8 = 0xC,

		ArrayN1 = 0xD,
		ArrayN2 = 0xE,
		ArrayN3 = 0xF,
		ArrayN4 = 0x10,
		ArrayN5 = 0x11,
		ArrayN6 = 0x12,
		ArrayN7 = 0x13,
		ArrayN8 = 0x14,

		KeyNameN1 = 0x11,
		KeyNameN2 = 0x12,
		KeyNameN3 = 0x13,
		KeyNameN4 = 0x14,

		StringN1 = 0x15,
		StringN2 = 0x16,
		StringN3 = 0x17,
		StringN4 = 0x18,

		ResourceN1 = 0x19,
		ResourceN2 = 0x1A,
		ResourceN3 = 0x1B,
		ResourceN4 = 0x1C,

		Float0 = 0x1D,
		Float = 0x1E,
		Double = 0x1F,

		List = 0x20,
		Objects = 0x21,

		ExtraChunkN1 = 0x22,
		ExtraChunkN2 = 0x23,
		ExtraChunkN3 = 0x24,
		ExtraChunkN4 = 0x25,
	};

	static void Log(const char *format, ...)
	{
		va_list args;
		va_start(args, format);
		fprintf(stderr, "[PSB-NATIVE] ");
		vfprintf(stderr, format, args);
		fprintf(stderr, "\n");
		va_end(args);
	}

	static tjs_uint8 ReadU8(tTJSBinaryStream *stream)
	{
		tjs_uint8 value = 0;
		stream->ReadBuffer(&value, 1);
		return value;
	}

	static bool AsciiEqualNoCase(char actual, char expected)
	{
		if (actual >= 'A' && actual <= 'Z')
			actual = static_cast<char>(actual - 'A' + 'a');
		if (expected >= 'A' && expected <= 'Z')
			expected = static_cast<char>(expected - 'A' + 'a');
		return actual == expected;
	}

	static bool SignatureEquals3(const char *signature, const char *expected)
	{
		return AsciiEqualNoCase(signature[0], expected[0]) &&
			AsciiEqualNoCase(signature[1], expected[1]) &&
			AsciiEqualNoCase(signature[2], expected[2]);
	}

	static ttstr Utf8ToTtstr(const std::string &text)
	{
		if (text.empty())
			return ttstr(TJS_W(""));

		tjs_int len = TVPUtf8ToWideCharString(text.c_str(), static_cast<tjs_uint>(text.size()), nullptr);
		if (len < 0)
			return ttstr(text.c_str());

		std::vector<tjs_char> wide(static_cast<size_t>(len) + 1);
		TVPUtf8ToWideCharString(text.c_str(), static_cast<tjs_uint>(text.size()), wide.data());
		wide[static_cast<size_t>(len)] = 0;
		return ttstr(wide.data());
	}

	static tjs_int FindChar(const ttstr &text, tjs_char ch)
	{
		const tjs_char *p = text.c_str();
		for (tjs_int i = 0; i < text.GetLen(); ++i)
		{
			if (p[i] == ch)
				return i;
		}
		return -1;
	}

	static ttstr SubString(const ttstr &text, tjs_int start, tjs_int length)
	{
		if (start < 0)
			start = 0;
		if (length < 0)
			length = 0;
		if (start > text.GetLen())
			start = text.GetLen();
		if (start + length > text.GetLen())
			length = text.GetLen() - start;
		return ttstr(text.c_str() + start, length);
	}

	static tjs_uint64 ReadUnsignedLE(tTJSBinaryStream *stream, tjs_uint bytes)
	{
		tjs_uint64 value = 0;
		for (tjs_uint i = 0; i < bytes; ++i)
			value |= static_cast<tjs_uint64>(ReadU8(stream)) << (i * 8);
		return value;
	}

	static tjs_int64 ReadSignedLE(tTJSBinaryStream *stream, tjs_uint bytes)
	{
		tjs_uint64 value = ReadUnsignedLE(stream, bytes);
		if (bytes > 0 && bytes < 8)
		{
			tjs_uint64 sign = static_cast<tjs_uint64>(1) << (bytes * 8 - 1);
			if (value & sign)
				value |= (~static_cast<tjs_uint64>(0)) << (bytes * 8);
		}
		return static_cast<tjs_int64>(value);
	}

	struct PSBHeader
	{
		char signature[4];
		tjs_uint16 version;
		tjs_uint16 encrypt;
		tjs_uint32 offsetEncrypt;
		tjs_uint32 offsetNames;
		tjs_uint32 offsetStrings;
		tjs_uint32 offsetStringsData;
		tjs_uint32 offsetChunkOffsets;
		tjs_uint32 offsetChunkLengths;
		tjs_uint32 offsetChunkData;
		tjs_uint32 offsetEntries;
		tjs_uint32 checksum;
		tjs_uint32 offsetExtraChunkOffsets;
		tjs_uint32 offsetExtraChunkLengths;
		tjs_uint32 offsetExtraChunkData;

		static constexpr int MAX_LENGTH = 56;

		void reset()
		{
			std::memset(this, 0, sizeof(*this));
		}

		bool isEncrypted() const
		{
			return offsetEncrypt > MAX_LENGTH + 16 || offsetNames == 0 ||
				(version > 1 && offsetEncrypt != offsetNames && offsetEncrypt != 0);
		}

		tjs_uint32 getHeaderLength() const
		{
			if (version < 3)
				return 40u;
			if (version > 3)
				return 56u;
			return 44u;
		}

		bool parsePSBHeader(tTJSBinaryStream *stream)
		{
			reset();
			if (stream->GetSize() < 8)
			{
				Log("file is too small for PSB header");
				return false;
			}

			stream->ReadBuffer(signature, 4);
			version = stream->ReadI16LE();
			encrypt = stream->ReadI16LE();
			offsetEncrypt = stream->ReadI32LE();
			offsetNames = stream->ReadI32LE();

			if (SignatureEquals3(signature, "MDF") || SignatureEquals3(signature, "MFL"))
			{
				Log("MDF/MFL wrapper reached PSB header parser");
				return false;
			}
			if (!SignatureEquals3(signature, "PSB"))
			{
				Log("not a valid PSB file, signature=%02x %02x %02x %02x",
					static_cast<unsigned char>(signature[0]),
					static_cast<unsigned char>(signature[1]),
					static_cast<unsigned char>(signature[2]),
					static_cast<unsigned char>(signature[3]));
				return false;
			}
			if (offsetNames >= stream->GetSize())
			{
				Log("bad PSB header: offsetNames=%u size=%llu",
					offsetNames, static_cast<unsigned long long>(stream->GetSize()));
				return false;
			}

			offsetStrings = stream->ReadI32LE();
			offsetStringsData = stream->ReadI32LE();
			offsetChunkOffsets = stream->ReadI32LE();
			offsetChunkLengths = stream->ReadI32LE();
			offsetChunkData = stream->ReadI32LE();
			offsetEntries = stream->ReadI32LE();

			if (version > 2)
				checksum = stream->ReadI32LE();
			if (version > 3)
			{
				offsetExtraChunkOffsets = stream->ReadI32LE();
				offsetExtraChunkLengths = stream->ReadI32LE();
				offsetExtraChunkData = stream->ReadI32LE();
			}
			return true;
		}
	};

	static bool ParsePSBArray(std::vector<tjs_uint32> *target, tjs_int countBytes,
		tTJSBinaryStream *stream)
	{
		target->clear();
		if (countBytes <= 0 || countBytes > 8)
		{
			Log("bad array count byte size: %d", static_cast<int>(countBytes));
			return false;
		}

		tjs_uint64 count64 = ReadUnsignedLE(stream, static_cast<tjs_uint>(countBytes));
		if (count64 > INT_MAX)
		{
			Log("long PSB array is not supported: %llu",
				static_cast<unsigned long long>(count64));
			return false;
		}

		tjs_uint8 entryType = ReadU8(stream);
		tjs_int entryLength = static_cast<tjs_int>(entryType) -
			static_cast<tjs_int>(PSBObjType::NumberN8);
		if (entryLength <= 0 || entryLength > 8)
		{
			Log("bad array entry byte size: type=0x%02x length=%d",
				entryType, static_cast<int>(entryLength));
			return false;
		}

		target->reserve(static_cast<size_t>(count64));
		for (tjs_uint64 i = 0; i < count64; ++i)
		{
			target->push_back(static_cast<tjs_uint32>(
				ReadUnsignedLE(stream, static_cast<tjs_uint>(entryLength))));
		}
		return true;
	}

	struct PSBMediaInfo
	{
		struct PSBMediaItemInfo
		{
			tjs_uint32 Offset;
			tjs_uint32 Length;
		};
		std::map<ttstr, PSBMediaItemInfo> resources;
	};

	class PSBMedia : public iTVPStorageMedia
	{
	public:
		PSBMedia() : refCount(1) {}

		void TJS_INTF_METHOD AddRef() override { ++refCount; }
		void TJS_INTF_METHOD Release() override
		{
			if (refCount == 1)
				delete this;
			else
				--refCount;
		}

		void TJS_INTF_METHOD GetName(ttstr &name) override { name = TJS_W("psb"); }
		void TJS_INTF_METHOD NormalizeDomainName(ttstr &name) override;
		void TJS_INTF_METHOD NormalizePathName(ttstr &name) override;
		bool TJS_INTF_METHOD CheckExistentStorage(const ttstr &name) override;
		tTJSBinaryStream * TJS_INTF_METHOD Open(const ttstr &name, tjs_uint32 flags) override;
		void TJS_INTF_METHOD GetListAt(const ttstr &name, iTVPStorageLister *lister) override;
		void TJS_INTF_METHOD GetLocallyAccessibleName(ttstr &name) override;

		void AddPSBFile(const ttstr &name, const PSBMediaInfo &data)
		{
			resources[name] = data;
		}

		void RemovePSBFile(const ttstr &name)
		{
			resources.erase(name);
		}

	private:
		~PSBMedia() {}

		bool FindResource(const ttstr &name, ttstr *fileName, ttstr *chunkName,
			PSBMediaInfo::PSBMediaItemInfo *item);

		tjs_uint refCount;
		std::map<ttstr, PSBMediaInfo> resources;
	};
}

static PSB::PSBMedia *gPSBMedia = nullptr;

class tTJSNI_PSBFile : public tTJSNativeInstance
{
	typedef tTJSNativeInstance inherited;

public:
	tTJSNI_PSBFile()
	{
		Owner = nullptr;
		filePtr = nullptr;
		_header.reset();
	}

	~tTJSNI_PSBFile()
	{
		Clear();
	}

	tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param,
		iTJSDispatch2 *tjs_obj) override
	{
		Owner = tjs_obj;
		return TJS_S_OK;
	}

	void TJS_INTF_METHOD Invalidate() override
	{
		Clear();
		Owner = nullptr;
		inherited::Invalidate();
	}

	void Clear()
	{
		if (filePtr)
		{
			delete filePtr;
			filePtr = nullptr;
		}
		_header.reset();
		stringsOffset.clear();
		namesData.clear();
		charset.clear();
		nameIndexes.clear();
		namesCache.clear();
		chunkOffsets.clear();
		chunkLengths.clear();
		extraChunkOffsets.clear();
		extraChunkLengths.clear();
		_resources.resources.clear();
		_root.Clear();
	}

	bool load(const ttstr &filePath)
	{
		Clear();

		try
		{
			filePtr = TVPCreateStream(filePath);
		}
		catch (...)
		{
			PSB::Log("failed to open: %s", filePath.AsNarrowStdString().c_str());
			return false;
		}
		if (!filePtr)
			return false;

		tjs_uint64 readSize = filePtr->GetSize();
		if (readSize < 9)
		{
			PSB::Log("file too small: %s size=%llu",
				filePath.AsNarrowStdString().c_str(),
				static_cast<unsigned long long>(readSize));
			return false;
		}

		char sign[4] = {};
		filePtr->ReadBuffer(sign, 4);
		if (PSB::SignatureEquals3(sign, "MDF"))
		{
			tjs_uint32 uncompressedSize32 = filePtr->ReadI32LE();
			uLongf uncompressedSize = uncompressedSize32;
			std::vector<tjs_uint8> compressed(static_cast<size_t>(readSize - 8));
			filePtr->ReadBuffer(compressed.data(), static_cast<tjs_uint>(compressed.size()));
			std::vector<tjs_uint8> uncompressed(static_cast<size_t>(uncompressedSize));

			int zret = uncompress(uncompressed.data(), &uncompressedSize,
				compressed.data(), static_cast<uLong>(compressed.size()));
			if (zret != Z_OK || uncompressedSize != uncompressedSize32)
			{
				PSB::Log("MDF decompression failed: %s zret=%d",
					filePath.AsNarrowStdString().c_str(), zret);
				return false;
			}

			tTVPMemoryStream *stream =
				new tTVPMemoryStream(nullptr, static_cast<tjs_uint>(uncompressedSize));
			std::memcpy(stream->GetInternalBuffer(), uncompressed.data(),
				static_cast<size_t>(uncompressedSize));
			delete filePtr;
			filePtr = stream;
			PSB::Log("decompressed MDF: %s %llu -> %lu",
				filePath.AsNarrowStdString().c_str(),
				static_cast<unsigned long long>(readSize),
				static_cast<unsigned long>(uncompressedSize));
		}

		filePtr->SetPosition(0);
		if (!_header.parsePSBHeader(filePtr))
			return false;

		if (_header.isEncrypted() && _header.getHeaderLength() > filePtr->GetSize())
		{
			PSB::Log("psb file is encrypted: %s", filePath.AsNarrowStdString().c_str());
			return false;
		}
		if (_header.version > 3)
		{
			PSB::Log("unsupported PSB version %u: %s",
				_header.version, filePath.AsNarrowStdString().c_str());
			return false;
		}

		if (!ReadArrayAt(_header.offsetStrings, &stringsOffset))
			return false;

		if (_header.version == 1)
		{
			if (_header.offsetEncrypt >= filePtr->GetSize())
				_header.offsetEncrypt = _header.getHeaderLength();
			if (!ReadArrayAt(_header.offsetEncrypt, &nameIndexes))
				return false;

			namesCache.reserve(nameIndexes.size());
			for (size_t i = 0; i < nameIndexes.size(); ++i)
			{
				filePtr->SetPosition(_header.offsetNames + nameIndexes[i]);
				namesCache.push_back(ReadNullTerminatedString());
			}
		}
		else
		{
			if (!ReadArrayAt(_header.offsetNames, &charset))
				return false;
			if (!ReadArrayHere(&namesData))
				return false;
			if (!ReadArrayHere(&nameIndexes))
				return false;

			namesCache.reserve(nameIndexes.size());
			for (size_t i = 0; i < nameIndexes.size(); ++i)
			{
				if (nameIndexes[i] >= namesData.size())
				{
					PSB::Log("bad name index: %u", nameIndexes[i]);
					return false;
				}

				std::string list;
				tjs_uint32 chr = namesData[nameIndexes[i]];
				size_t guard = 0;
				while (chr != 0)
				{
					if (chr >= namesData.size() || guard++ > namesData.size())
					{
						PSB::Log("bad name table chain");
						return false;
					}
					tjs_uint32 code = namesData[chr];
					if (code >= charset.size())
					{
						PSB::Log("bad charset index: %u", code);
						return false;
					}
					tjs_uint32 delta = charset[code];
					list.append(1, static_cast<char>(chr - delta));
					chr = code;
				}
				std::reverse(list.begin(), list.end());
				namesCache.push_back(PSB::Utf8ToTtstr(list));
			}
		}

		if (!ReadArrayAt(_header.offsetChunkOffsets, &chunkOffsets))
			return false;
		if (!ReadArrayAt(_header.offsetChunkLengths, &chunkLengths))
			return false;

		_root = readAllObjs(TJS_W("root"), _header.offsetEntries);
		if (gPSBMedia)
			gPSBMedia->AddPSBFile(filePath.AsLowerCase(), _resources);

		PSB::Log("loaded: %s version=%u names=%zu strings=%zu chunks=%zu",
			filePath.AsNarrowStdString().c_str(), _header.version,
			namesCache.size(), stringsOffset.size(), chunkOffsets.size());
		return true;
	}

	tTJSVariant root()
	{
		return _root;
	}

	void setRoot(const tTJSVariant &value)
	{
		_root = value;
	}

	tTJSVariant readAllObjs(const ttstr &key, tjs_uint32 objOffset)
	{
		if (objOffset >= filePtr->GetSize())
		{
			PSB::Log("object offset out of range: %u", objOffset);
			return tTJSVariant();
		}

		filePtr->SetPosition(objOffset);
		tjs_uint8 typeByte = PSB::ReadU8(filePtr);
		PSB::PSBObjType type = static_cast<PSB::PSBObjType>(typeByte);
		switch (type)
		{
		case PSB::PSBObjType::None:
		case PSB::PSBObjType::Null:
			return tTJSVariant();
		case PSB::PSBObjType::False:
			return tTJSVariant(static_cast<tjs_int>(0));
		case PSB::PSBObjType::True:
			return tTJSVariant(static_cast<tjs_int>(1));
		case PSB::PSBObjType::NumberN0:
			return tTJSVariant(static_cast<tjs_int>(0));
		case PSB::PSBObjType::NumberN1:
		case PSB::PSBObjType::NumberN2:
		case PSB::PSBObjType::NumberN3:
		case PSB::PSBObjType::NumberN4:
		{
			tjs_uint bytes = typeByte - static_cast<tjs_uint8>(PSB::PSBObjType::NumberN0);
			return tTJSVariant(static_cast<tjs_int>(PSB::ReadSignedLE(filePtr, bytes)));
		}
		case PSB::PSBObjType::NumberN5:
		case PSB::PSBObjType::NumberN6:
		case PSB::PSBObjType::NumberN7:
		case PSB::PSBObjType::NumberN8:
		{
			tjs_uint bytes = typeByte - static_cast<tjs_uint8>(PSB::PSBObjType::NumberN0);
			return tTJSVariant(static_cast<tjs_int64>(PSB::ReadSignedLE(filePtr, bytes)));
		}
		case PSB::PSBObjType::Float0:
			return tTJSVariant(static_cast<tjs_real>(0.0));
		case PSB::PSBObjType::Float:
		{
			float value = 0.0f;
			filePtr->ReadBuffer(&value, 4);
			return tTJSVariant(static_cast<tjs_real>(value));
		}
		case PSB::PSBObjType::Double:
		{
			double value = 0.0;
			filePtr->ReadBuffer(&value, 8);
			return tTJSVariant(static_cast<tjs_real>(value));
		}
		case PSB::PSBObjType::ArrayN1:
		case PSB::PSBObjType::ArrayN2:
		case PSB::PSBObjType::ArrayN3:
		case PSB::PSBObjType::ArrayN4:
		case PSB::PSBObjType::ArrayN5:
		case PSB::PSBObjType::ArrayN6:
		case PSB::PSBObjType::ArrayN7:
		case PSB::PSBObjType::ArrayN8:
		{
			std::vector<tjs_uint32> values;
			if (!PSB::ParsePSBArray(&values,
				typeByte - static_cast<tjs_uint8>(PSB::PSBObjType::ArrayN1) + 1, filePtr))
				return tTJSVariant();

			iTJSDispatch2 *array = TJSCreateArrayObject();
			for (tjs_uint i = 0; i < values.size(); ++i)
			{
				tTJSVariant value(static_cast<tjs_int64>(values[i]));
				array->PropSetByNum(TJS_MEMBERENSURE, i, &value, array);
			}
			tTJSVariant result(array, array);
			array->Release();
			return result;
		}
		case PSB::PSBObjType::StringN1:
		case PSB::PSBObjType::StringN2:
		case PSB::PSBObjType::StringN3:
		case PSB::PSBObjType::StringN4:
		{
			tjs_uint bytes = typeByte - static_cast<tjs_uint8>(PSB::PSBObjType::StringN1) + 1;
			tjs_uint32 index = static_cast<tjs_uint32>(PSB::ReadUnsignedLE(filePtr, bytes));
			if (index >= stringsOffset.size())
				return tTJSVariant(TJS_W(""));
			filePtr->SetPosition(_header.offsetStringsData + stringsOffset[index]);
			return tTJSVariant(ReadNullTerminatedString());
		}
		case PSB::PSBObjType::ResourceN1:
		case PSB::PSBObjType::ResourceN2:
		case PSB::PSBObjType::ResourceN3:
		case PSB::PSBObjType::ResourceN4:
		{
			if (key.IsEmpty())
				return tTJSVariant();

			tjs_uint bytes = typeByte - static_cast<tjs_uint8>(PSB::PSBObjType::ResourceN1) + 1;
			tjs_uint32 index = static_cast<tjs_uint32>(PSB::ReadUnsignedLE(filePtr, bytes));
			if (index >= chunkOffsets.size() || index >= chunkLengths.size())
				return tTJSVariant();

			tjs_uint32 offset = _header.offsetChunkData + chunkOffsets[index];
			tjs_uint32 length = chunkLengths[index];
			_resources.resources[key] = { offset, length };

			if (offset + length > filePtr->GetSize())
				return tTJSVariant();
			std::vector<tjs_uint8> buffer(length);
			filePtr->SetPosition(offset);
			if (length)
				filePtr->ReadBuffer(buffer.data(), length);
			tTJSVariantOctet *octet = TJSAllocVariantOctet(buffer.data(), length);
			tTJSVariant result(octet);
			octet->Release();
			return result;
		}
		case PSB::PSBObjType::ExtraChunkN1:
		case PSB::PSBObjType::ExtraChunkN2:
		case PSB::PSBObjType::ExtraChunkN3:
		case PSB::PSBObjType::ExtraChunkN4:
		{
			if (key.IsEmpty())
				return tTJSVariant();

			tjs_uint bytes = typeByte - static_cast<tjs_uint8>(PSB::PSBObjType::ExtraChunkN1) + 1;
			tjs_uint32 index = static_cast<tjs_uint32>(PSB::ReadUnsignedLE(filePtr, bytes));
			if (index < extraChunkOffsets.size() && index < extraChunkLengths.size())
			{
				_resources.resources[key] = {
					_header.offsetExtraChunkData + extraChunkOffsets[index],
					extraChunkLengths[index]
				};
			}
			return tTJSVariant();
		}
		case PSB::PSBObjType::List:
		{
			std::vector<tjs_uint32> objectOffsets;
			tjs_uint32 baseOffset = readListInfo(&objectOffsets);

			iTJSDispatch2 *array = TJSCreateArrayObject();
			for (tjs_uint i = 0; i < objectOffsets.size(); ++i)
			{
				tTJSVariant value = readAllObjs(ttstr(), baseOffset + objectOffsets[i]);
				array->PropSetByNum(TJS_MEMBERENSURE, i, &value, array);
			}
			tTJSVariant result(array, array);
			array->Release();
			return result;
		}
		case PSB::PSBObjType::Objects:
		{
			std::vector<tjs_uint32> objectOffsets;
			std::vector<tjs_uint32> objectNameIndexes;
			tjs_uint32 baseOffset = 0;
			if (_header.version == 1)
			{
				baseOffset = readListInfo(&objectOffsets);
				refreshListInfo(&objectOffsets, &objectNameIndexes);
			}
			else
			{
				readListInfo(&objectNameIndexes);
				baseOffset = readListInfo(&objectOffsets);
			}

			iTJSDispatch2 *dict = TJSCreateDictionaryObject();
			size_t count = std::min(objectNameIndexes.size(), objectOffsets.size());
			for (size_t i = 0; i < count; ++i)
			{
				if (objectNameIndexes[i] >= namesCache.size())
					continue;
				ttstr keyName = namesCache[objectNameIndexes[i]];
				tTJSVariant value = readAllObjs(keyName, baseOffset + objectOffsets[i]);
				dict->PropSet(TJS_MEMBERENSURE, keyName.c_str(), nullptr, &value, dict);
			}
			tTJSVariant result(dict, dict);
			dict->Release();
			return result;
		}
		default:
			PSB::Log("unknown psb object type: 0x%02x at %llu", typeByte,
				static_cast<unsigned long long>(objOffset));
			return tTJSVariant();
		}
	}

	tjs_uint32 readListInfo(std::vector<tjs_uint32> *target)
	{
		if (!ReadArrayHere(target))
			return static_cast<tjs_uint32>(filePtr->GetPosition());
		return static_cast<tjs_uint32>(filePtr->GetPosition());
	}

	void refreshListInfo(std::vector<tjs_uint32> *target1, std::vector<tjs_uint32> *target2)
	{
		target2->resize(target1->size());
		tjs_uint32 basePos = static_cast<tjs_uint32>(filePtr->GetPosition());
		for (size_t i = 0; i < target1->size(); ++i)
		{
			filePtr->SetPosition(basePos + target1->at(i));
			target2->at(i) = PSB::ReadU8(filePtr);
			target1->at(i) += 4;
		}
	}

private:
	bool ReadArrayAt(tjs_uint32 offset, std::vector<tjs_uint32> *target)
	{
		if (offset >= filePtr->GetSize())
		{
			PSB::Log("array offset out of range: %u size=%llu", offset,
				static_cast<unsigned long long>(filePtr->GetSize()));
			return false;
		}
		filePtr->SetPosition(offset);
		return ReadArrayHere(target);
	}

	bool ReadArrayHere(std::vector<tjs_uint32> *target)
	{
		tjs_uint8 type = PSB::ReadU8(filePtr);
		return PSB::ParsePSBArray(target,
			type - static_cast<tjs_uint8>(PSB::PSBObjType::ArrayN1) + 1, filePtr);
	}

	ttstr ReadNullTerminatedString()
	{
		std::string str;
		tjs_uint8 value = 0;
		while (filePtr->Read(&value, 1) == 1)
		{
			if (value == 0)
				break;
			str.append(1, static_cast<char>(value));
		}
		return PSB::Utf8ToTtstr(str);
	}

	iTJSDispatch2 *Owner;
	tTJSBinaryStream *filePtr;

	PSB::PSBHeader _header;
	std::vector<tjs_uint32> stringsOffset;
	std::vector<tjs_uint32> namesData;
	std::vector<tjs_uint32> charset;
	std::vector<tjs_uint32> nameIndexes;
	std::vector<ttstr> namesCache;
	std::vector<tjs_uint32> chunkOffsets;
	std::vector<tjs_uint32> chunkLengths;
	std::vector<tjs_uint32> extraChunkOffsets;
	std::vector<tjs_uint32> extraChunkLengths;
	PSB::PSBMediaInfo _resources;
	tTJSVariant _root;
};

namespace PSB
{
	void PSBMedia::NormalizeDomainName(ttstr &name)
	{
		tjs_int dotIndex = FindChar(name, TJS_W('.'));
		if (dotIndex == -1)
			return;
		name = SubString(name, 0, dotIndex) +
			SubString(name, dotIndex, name.GetLen() - dotIndex).AsLowerCase();
	}

	void PSBMedia::NormalizePathName(ttstr &name)
	{
	}

	bool PSBMedia::FindResource(const ttstr &name, ttstr *fileName, ttstr *chunkName,
		PSBMediaInfo::PSBMediaItemInfo *item)
	{
		tjs_int slashIndex = FindChar(name, TJS_W('/'));
		if (slashIndex == -1)
			return false;

		ttstr psbName = SubString(name, 0, slashIndex);
		ttstr resourceName = SubString(name, slashIndex + 1, name.GetLen() - slashIndex - 1);
		auto iterFile = resources.find(psbName);
		if (iterFile == resources.end())
		{
			tTJSNI_PSBFile tmp;
			tmp.load(psbName);
			iterFile = resources.find(psbName);
			if (iterFile == resources.end())
				return false;
		}

		auto iterChunk = iterFile->second.resources.find(resourceName);
		if (iterChunk == iterFile->second.resources.end())
			return false;

		if (fileName)
			*fileName = psbName;
		if (chunkName)
			*chunkName = resourceName;
		if (item)
			*item = iterChunk->second;
		return true;
	}

	bool PSBMedia::CheckExistentStorage(const ttstr &name)
	{
		return FindResource(name, nullptr, nullptr, nullptr);
	}

	tTJSBinaryStream *PSBMedia::Open(const ttstr &name, tjs_uint32 flags)
	{
		ttstr psbName;
		PSBMediaInfo::PSBMediaItemInfo item = {};
		if (!FindResource(name, &psbName, nullptr, &item))
			return nullptr;

		tTJSBinaryStream *file = nullptr;
		try
		{
			file = TVPCreateStream(psbName);
		}
		catch (...)
		{
			return nullptr;
		}
		if (!file)
			return nullptr;

		tTVPMemoryStream *memory = new tTVPMemoryStream(nullptr, item.Length);
		file->SetPosition(item.Offset);
		if (item.Length)
			file->ReadBuffer(memory->GetInternalBuffer(), item.Length);
		delete file;
		return memory;
	}

	void PSBMedia::GetListAt(const ttstr &name, iTVPStorageLister *lister)
	{
	}

	void PSBMedia::GetLocallyAccessibleName(ttstr &name)
	{
		name.Clear();
	}
}

static iTJSNativeInstance * TJS_INTF_METHOD Create_NI_PSBFile()
{
	return new tTJSNI_PSBFile();
}

static tjs_int32 ClassID_PSBFile = -1;
#define TJS_NATIVE_CLASSID_NAME ClassID_PSBFile

static void TVPRegisterPSBFileClass()
{
	tTJSNativeClassForPlugin *classobj =
		TJSCreateNativeClassForPlugin(TJS_W("PSBFile"), Create_NI_PSBFile);

#undef TJS_NCM_REG_THIS
#define TJS_NCM_REG_THIS classobj
	tjs_int32 ClassID = -1;

	TJS_BEGIN_NATIVE_MEMBERS(PSBFile)
	ClassID_PSBFile = TJS_NCM_CLASSID;
	TJS_DECL_EMPTY_FINALIZE_METHOD

TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(_this, tTJSNI_PSBFile, PSBFile)
{
	if (numparams == 0)
		return TJS_S_OK;
	if (numparams == 1 && param[0]->Type() == tvtString)
	{
		ttstr path = *param[0];
		bool ok = _this->load(path);
		if (result)
			*result = static_cast<tjs_int>(ok ? 1 : 0);
		return TJS_S_OK;
	}
	return TJS_E_BADPARAMCOUNT;
}
TJS_END_NATIVE_CONSTRUCTOR_DECL(PSBFile)

TJS_BEGIN_NATIVE_METHOD_DECL(load)
{
	TJS_GET_NATIVE_INSTANCE(_this, tTJSNI_PSBFile);
	if (numparams != 1)
		return TJS_E_BADPARAMCOUNT;
	if (param[0]->Type() != tvtString)
		return TJS_E_INVALIDPARAM;

	ttstr path = *param[0];
	bool ok = _this->load(path);
	if (result)
		*result = static_cast<tjs_int>(ok ? 1 : 0);
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(load)

TJS_BEGIN_NATIVE_METHOD_DECL(getData)
{
	TJS_GET_NATIVE_INSTANCE(_this, tTJSNI_PSBFile);
	if (result)
		*result = _this->root();
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(getData)

TJS_BEGIN_NATIVE_METHOD_DECL(setData)
{
	TJS_GET_NATIVE_INSTANCE(_this, tTJSNI_PSBFile);
	if (numparams != 1)
		return TJS_E_BADPARAMCOUNT;
	_this->setRoot(*param[0]);
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(setData)

TJS_BEGIN_NATIVE_METHOD_DECL(save)
{
	if (result)
		*result = static_cast<tjs_int>(1);
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(save)

TJS_BEGIN_NATIVE_PROP_DECL(root)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(_this, tTJSNI_PSBFile);
		if (result)
			*result = _this->root();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER
	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(root)

	TJS_END_NATIVE_MEMBERS

	tTJSVariant val(classobj, classobj);
	classobj->Release();
	iTJSDispatch2 *global = TVPGetScriptDispatch();
	global->PropSet(TJS_MEMBERENSURE, TJS_W("PSBFile"), NULL, &val, global);
	global->Release();
}
#undef TJS_NATIVE_CLASSID_NAME

extern void TVPRegisterPSBGraphicLoader();

void TVPLoadPSBFilePlugin()
{
	static bool registered = false;
	if (registered)
		return;
	registered = true;

	TVPRegisterPSBFileClass();
	if (!gPSBMedia)
	{
		gPSBMedia = new PSB::PSBMedia();
		TVPRegisterStorageMedia(gPSBMedia);
	}
	TVPRegisterPSBGraphicLoader();
}
