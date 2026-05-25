//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#if 0
	#pragma inline
#endif

// removed: windows.h
#include "tjsCommHead.h"
#include "TransIntf.h"
#include "tvpgl.h"
#include "MsgIntf.h"
#include <math.h>
#include "ripple.h"
#include "common.h"

//---------------------------------------------------------------------------
/*
	'�g��' �g�����W�V����
	�u���}�b�v�ɂ��A�g�䂪�L�����Ă����悤�Ȋ����̃g�����W�V����
	���̃g�����W�V�����͓]���悪���������Ă����(�v����Ƀg�����W�V�������s��
	���C���� type �� ltOpaque �ȊO�̏ꍇ)�A����ɓ��ߏ��������ł��Ȃ��̂�
	����
*/
//---------------------------------------------------------------------------


// 2003/12/15 W.Dee  M_PI ������`�G���[�ɂȂ�̂��C����SSE���߂�_emit�ɒu������

//---------------------------------------------------------------------------
// #define TVP_DEBUG_RIPPLE_SHOW_UPDATE_COUNT
	// ��`����ƃg�����W�V�������ɉ�ʂ��X�V�����񐔂�\������
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
#define TVP_RIPPLE_DIR_PREC 32
	// �e�[�u�����łP�ی���(90��)�̕����������ɕ������邩
	// (2 �̗ݏ�� 256 �܂ŁB�傫������ƃ�������H��)
#define TVP_RIPPLE_DRIFT_PREC 4
	// drift 1 �s�N�Z���������ɕ������邩
//---------------------------------------------------------------------------
#ifndef M_PI
	#define M_PI (3.14159263589793238462)
#endif
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
/*
	������ �e�[�u�����Ǘ�����N���X/�֐��Q
	�e�[�u���́A���S���W�A�g�����W�V�����摜�̃T�C�Y�A
	�g�̕��A�g��̏c/����A�h��̕����O��ƕς��Ȃ�����Đ����͂���Ȃ��B
	�Đ����ɂ͂��������Ԃ������邽�߁A4�܂ŃL���b�V�����s�����Ƃ��ł���B
*/
//---------------------------------------------------------------------------
class tTVPRippleTable
{
	tjs_int RefCount; // �Q�ƃJ�E���^

	tjs_int Width; // �g�����W�V�����摜�̕�
	tjs_int Height; // �g�����W�V�����摜�̍���

	tjs_int CenterX; // �g��̒��S X ���W
	tjs_int CenterY; // �g��̒��S Y ���W

	tjs_int RippleWidth; // �g��̕�
	float Roundness; // �g��̏c/����
	tjs_int MaxDrift; // �h��̍ő啝

	tjs_int MapWidth; // �u���}�b�v�̕�
	tjs_int MapHeight; // �u���}�b�v�̍���

	tjs_uint16 *DisplaceMap; // [�ʒu]->[����,����] �u���}�b�v
	tjs_uint16 *DriftMap; // [�h��̑傫��,����,����]->[����] �u���}�b�v

public:
	tjs_int GetWidth() const { return Width; }
	tjs_int GetHeight() const { return Height; }

	tjs_int GetCenterX() const { return CenterX; }
	tjs_int GetCenterY() const { return CenterY; }

	tjs_int GetRippleWidth() const { return RippleWidth; }
	float GetRoundness() const { return Roundness; }
	tjs_int GetMaxDrift() const { return MaxDrift; }

	tjs_int GetMapWidth() const { return MapWidth; }
	tjs_int GetMapHeight() const { return MapHeight; }


public:
	tTVPRippleTable(tjs_int width, tjs_int height, tjs_int centerx, tjs_int centery,
		tjs_int ripplewidth, float roundness, tjs_int maxdrift)
	{
		RefCount = 1;

		DisplaceMap = NULL;
		DriftMap = NULL;

		Width = width;
		Height = height;
		CenterX = centerx;
		CenterY = centery;
		RippleWidth = ripplewidth;
		Roundness = roundness;
		MaxDrift = maxdrift;

		MakeTable();
	}

protected:
	~tTVPRippleTable()
	{
		Clear();
	}

public:
	void AddRef()
	{
		RefCount ++;
	}

	void Release()
	{
		if(RefCount == 1)
			delete this;
		else
			RefCount--;
	}

public:
	const tjs_uint16 * GetDisplaceMap(tjs_int x, tjs_int y) const
	{
		return DisplaceMap + x + y*MapWidth;
	}


	const tjs_uint16 * GetDriftMap(tjs_int drift, tjs_int phase)
	{
		return DriftMap + drift * RippleWidth * (2 * TVP_RIPPLE_DIR_PREC) +
			phase * TVP_RIPPLE_DIR_PREC;
	}

private:
	void MakeTable();
	void Clear();

};
//---------------------------------------------------------------------------
float inline TVPRippleWaveForm(float rad)
{
	// �g�𐶐�����֐�
	// �K���ɁBs �͐��ɂ����Ȃ�Ȃ��������ڂ��ǂ��̂ł���ł���
	float s = (sin(rad) + sin(rad*2-2) * 0.2) / 1.19;
	s *= s;
	return s;
}
//---------------------------------------------------------------------------
void tTVPRippleTable::MakeTable()
{
	tjs_int32 *rippleform = NULL;
	tjs_int32 *cos_table = NULL;
	tjs_int32 *sin_table = NULL;

	try
	{
		// MapWidth, MapHeight �̌v�Z
		// Width, Height �� CenterX, CenterY �ŕ�������S�̏ی��̂���
		// �����Ƃ��傫�����̃T�C�Y�� MapWidth, MapHeight �Ƃ���
		MapWidth = CenterX < (Width >> 1) ?
			Width - CenterX : CenterX;
		MapHeight = CenterY < (Height >> 1) ?
			Height - CenterY : CenterY;

		// DisplaceMap �������m��
		DisplaceMap = new tjs_uint16[MapWidth * MapHeight];

		// DisplaceMap �v�Z
		// �u���}�b�v�͂P�ی��ɂ��Ă̂݌v�Z����(���̏ی��͑Ώ̂�����)
		tjs_uint16 *rmp = DisplaceMap;
		tjs_int ripplemask = RippleWidth - 1;
		tjs_int x, y;
		for(y = 0; y < MapHeight; y++)
		{
			float yy = ((float)y + 0.5) * Roundness;
			float fac = 1.0 / yy;
			for(x = 0; x < MapWidth; x++)
			{
				float xx =  (float)x + 0.5;

				tjs_int dir = atan(xx*fac) * ((1.0/(M_PI/2.0)) * TVP_RIPPLE_DIR_PREC);
					// dir = �����R�[�h

				tjs_int dist = (int)sqrt(xx*xx + yy*yy) & ripplemask;
					// dist = ���S����̋���

				*(rmp++) = (tjs_uint16)((dist * TVP_RIPPLE_DIR_PREC) + dir);
			}
		}

		// DriftMap �������m��
		// DriftMap �Ɏg�p���郁�����ʂ�
		// MaxDrift*TVP_RIPPLE_DRIFT_PREC * RippleWidth * 2 * TVP_RIPPLE_DIR_PREC    *sizeof(tjs_uint16)
		// tjs_uint32 [MaxDrift*TVP_RIPPLE_DRIFT_PREC][RippleWidth*2][TVP_RIPPLE_DIR_PREC]
		// *2 �������Ă���̂� �摜���Z���� & �Ń}�X�N��������K�v���Ȃ��悤��
		DriftMap = new tjs_uint16[MaxDrift * TVP_RIPPLE_DRIFT_PREC * RippleWidth *
			2 * TVP_RIPPLE_DIR_PREC];


		// �g�`�̌v�Z
		float rcp_rw = 1.0 / (float)RippleWidth;
		rippleform = new tjs_int32[RippleWidth];
		tjs_int w;
		for(w = 0; w < RippleWidth; w++)
		{
			// �K���ɔg���ۂ�������g�`(�P����sin�g�ł��悢)
			float rad = (float)w * rcp_rw * (M_PI * -2.0);
			
			float s = TVPRippleWaveForm(rad);
			
			if(s < -1.0) s = -1.0;
			if(s > 1.0) s = 1.0;
			s *= 2048.0;
			rippleform[w] = (tjs_int32)(s < 0 ? s - 0.5 : s + 0.5); // 1.11
		}

		// sin/cos �e�[�u���̐���
		cos_table = new tjs_int32[TVP_RIPPLE_DIR_PREC];
		sin_table = new tjs_int32[TVP_RIPPLE_DIR_PREC];
		for(w = 0; w < TVP_RIPPLE_DIR_PREC; w++)
		{
			float fdir = M_PI*0.5 - (((float)w + 0.5) *
				((1.0 / (float)TVP_RIPPLE_DIR_PREC) * (M_PI / 2.0)));
			float v;
			v = cos(fdir) * 2048.0;
			cos_table[w] = (tjs_int32)(v < 0 ? v - 0.5 : v + 0.5); // 1.11
			v = sin(fdir) * 2048.0;
			sin_table[w] = (tjs_int32)(v < 0 ? v - 0.5 : v + 0.5); // 1.11
		}

		// DriftMap �v�Z
		// float �Ōv�Z����ƃG�����x���̂ŌŒ菬���_�Ōv�Z����
		tjs_int drift, dir;
		tjs_int ripplewidth_step = RippleWidth * TVP_RIPPLE_DIR_PREC;
		for(drift = 0; drift < MaxDrift*TVP_RIPPLE_DRIFT_PREC; drift ++)
		{
			tjs_int32 fdrift = (drift << 10) / TVP_RIPPLE_DRIFT_PREC; // 8.10
			tjs_uint16 *dmp = DriftMap + drift * RippleWidth * (2 * TVP_RIPPLE_DIR_PREC);
			for(w = 0; w < RippleWidth; w++)
			{
				tjs_int32 fd = rippleform[w] * fdrift >> 10; // 8.11
				for(dir = 0; dir < TVP_RIPPLE_DIR_PREC; dir++)
				{
					tjs_int32 xd = cos_table[dir] * fd >> 11; // 8.11
					tjs_int32 yd = sin_table[dir] * fd >> 11; // 8.11
					
					tjs_uint16 val = (tjs_uint16)(
						( (int)(char)(int)(xd >>11)<< 8) +
						  (int)(char)(int)(yd >>11) );

					dmp[w * TVP_RIPPLE_DIR_PREC +                    dir] =
					dmp[w * TVP_RIPPLE_DIR_PREC + ripplewidth_step + dir] = val;
				}
			}
		}

	}
	catch(...)
	{
		Clear();
		if(rippleform) delete [] rippleform;
		if(sin_table) delete [] sin_table;
		if(cos_table) delete [] cos_table;
		throw;
	}
	if(rippleform) delete [] rippleform;
	if(sin_table) delete [] sin_table;
	if(cos_table) delete [] cos_table;
}
//---------------------------------------------------------------------------
void tTVPRippleTable::Clear()
{
	if(DisplaceMap) delete [] DisplaceMap, DisplaceMap = NULL;
	if(DriftMap) delete [] DriftMap, DriftMap = NULL;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// �L���b�V���Ǘ�
//---------------------------------------------------------------------------
#define TVP_RIPPLE_TABLE_MAX_CACHE 4
//---------------------------------------------------------------------------
static tTVPRippleTable *TVPRippleTableCache[TVP_RIPPLE_TABLE_MAX_CACHE] =
	{NULL};
//---------------------------------------------------------------------------
static tTVPRippleTable *TVPGetRippleTable
	(tjs_int width, tjs_int height, tjs_int centerx, tjs_int centery,
		tjs_int ripplewidth, float roundness, tjs_int maxdrift)
{
	// �L���b�V���̒�����w�肳�ꂽ�����̃f�[�^������Ă���
	// ����΃L���b�V�����ł̗D�揇�ʂ��ŏ�ʂɂ��ĕԂ��A
	// �����łȂ���΃f�[�^���쐬���ăL���b�V���̍Ō�̃f�[�^���폜���A
	// �D�揇�ʂ̐擪�ɑ}�����ĕԂ�	

	// �L���b�V�����ɂ��邩
	tjs_int i;
	for(i = 0; i < TVP_RIPPLE_TABLE_MAX_CACHE; i++)
	{
		tTVPRippleTable * table = TVPRippleTableCache[i];
		if(!table) continue;

		if(
			table->GetWidth() == width &&
			table->GetHeight() == height &&
			table->GetCenterX() == centerx &&
			table->GetCenterY() == centery &&
			table->GetRippleWidth() == ripplewidth &&
			table->GetRoundness() == roundness &&
			table->GetMaxDrift() == maxdrift)
		{
			// �L���b�V�����Ɍ�������

			// ���X�g�̐擪�ɂ����Ă���
			if(i != 0)
			{
				memmove(TVPRippleTableCache + 1, TVPRippleTableCache,
					i * sizeof(tTVPRippleTable *));
				TVPRippleTableCache[0] = table;
			}

			// �Q�ƃJ�E���^���C���N�������g���ĕԂ�
			table->AddRef();
			return table;
		}
	}

	// �L���b�V�����ɂ͌�����Ȃ�����

	// �Ō�̗v�f���폜
	if(TVPRippleTableCache[TVP_RIPPLE_TABLE_MAX_CACHE -1] != NULL)
	{
		tTVPRippleTable * table =
			TVPRippleTableCache[TVP_RIPPLE_TABLE_MAX_CACHE -1];
		TVPRippleTableCache[TVP_RIPPLE_TABLE_MAX_CACHE -1] = NULL;
		table->Release();
	}

	// �f�[�^���쐬
	tTVPRippleTable * table =
		new tTVPRippleTable
		(width, height, centerx, centery, ripplewidth, roundness, maxdrift);

	// ���X�g�̐擪�ɑ}��
	memmove(TVPRippleTableCache + 1, TVPRippleTableCache,
		(TVP_RIPPLE_TABLE_MAX_CACHE -1) * sizeof(tTVPRippleTable *));
	TVPRippleTableCache[0] = table;
	table->AddRef();

	// �Ԃ�
	return table;
}
//---------------------------------------------------------------------------
static void TVPInitRippleTableCache()
{
	// �L���b�V���̏�����
	tjs_int i;
	for(i = 0; i < TVP_RIPPLE_TABLE_MAX_CACHE; i++)
	{
		TVPRippleTableCache[i] = NULL;
	}
}
//---------------------------------------------------------------------------
static void TVPClearRippleTableCache()
{
	// �L���b�V�����N���A
	tjs_int i;
	for(i = 0; i < TVP_RIPPLE_TABLE_MAX_CACHE; i++)
	{
		tTVPRippleTable * table =
			TVPRippleTableCache[i];
		TVPRippleTableCache[i] = NULL;
		if(table) table->Release();
	}
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// ���Z�֐��Q (TVPRippleTransform_????) �́A
// �E�u���}�b�v�e�[�u���𐳕����Ɍ��Ă������t�����Ɍ��Ă����� (_f _b �T�t�B�b�N�X)
// �EC �o�[�W������ MMX/EMMX �A�Z���u���o�[�W���� (_c _mmx _emmx �T�t�B�b�N�X)
// �� 6 �ƁA�㉺���E��܂�Ԃ��Ȃ����ʊO���Q�Ƃ��Ȃ��悤�ɐT�d��
// �]������ C �֐� (_e �T�t�B�b�N�X) 4 ����Ȃ�
// �E�u���}�b�v�� y �𐳂ɂƂ邩���ɂƂ邩 (_a _d �T�t�B�b�N�X)
//---------------------------------------------------------------------------
#define TVP_RIPPLE_BLEND 	{ \
		tjs_uint32 s1, s2, s1_; \
		s1 = *(const tjs_uint32*)(src1 + ofs); s2 = *(const tjs_uint32*)(src2 + ofs); \
		s1_ = s1 & 0xff00ff; s1_ = (s1_ + (((s2 & 0xff00ff) - s1_) * ratio >> 8)) & 0xff00ff; \
		s2 &= 0xff00; s1 &= 0xff00; \
		dest[i] = s1_ | ((s1 + ((s2 - s1) * ratio >> 8)) & 0xff00); \
	}
//---------------------------------------------------------------------------
static void TVPRippleTransform_c_f(
	const tjs_uint16 *displacemap, const tjs_uint16 *driftmap, tjs_uint32 *dest,
	tjs_int num, tjs_int pitch, const tjs_uint8 * src1, const tjs_uint8 * src2, tjs_int ratio)
{
	for(int i = 0; i < num; i++)
	{
		tjs_int n = driftmap[displacemap[i]];
		tjs_int ofs = (int)((i - (int)(char)(n>>8))*sizeof(tjs_uint32)) +
			(int)(char)(n)*pitch;
		TVP_RIPPLE_BLEND
	}
}
//---------------------------------------------------------------------------
static void TVPRippleTransform_c_b(
	const tjs_uint16 *displacemap, const tjs_uint16 *driftmap, tjs_uint32 *dest,
	tjs_int num, tjs_int pitch, const tjs_uint8 * src1, const tjs_uint8 * src2, tjs_int ratio)
{
	for(int i = 0; i < num; i++)
	{
		tjs_int n = driftmap[*(displacemap--)];
		tjs_int ofs = (int)((i + (int)(char)(n>>8))*sizeof(tjs_uint32)) +
			(int)(char)(n)*pitch;
		TVP_RIPPLE_BLEND
	}
}
//---------------------------------------------------------------------------


#if defined(_MSC_VER) && defined(_M_IX86)
//---------------------------------------------------------------------------
static void TVPRippleTransform_mmx_f(
	const tjs_uint16 *displacemap, const tjs_uint16 *driftmap, tjs_uint32 *dest,
	tjs_int num, tjs_int pitch, const tjs_uint8 * src1, const tjs_uint8 * src2, tjs_int ratio)
{
	_asm
	{
		mov			esi,			driftmap
		mov			edi,			dest
		xor			ebx,			ebx
		mov			ecx,			displacemap
		movd		mm7,			ratio
		psllq		mm7,			7
		punpcklwd	mm7,			mm7
		punpcklwd	mm7,			mm7
		pxor		mm0,			mm0

		sub			num,			1
																		// ���X���b�h
		cmp			num,			ebx
		jng			pexit_mmx_f

	ploop_mmx_f_1:	// ���̃��[�v�ł͂Q�̃X���b�h��K���ɃC���^�[���[�u���Ă���
		movzx		eax,			word ptr [ecx + ebx*2]				// 1
		movzx		eax,			word ptr [esi + eax*2]				// 1
		movsx		edx,			ah									// 1
		movsx		eax,			al									// 1
		neg			edx													// 1
		imul		eax,			pitch								// 1
		lea			edx,			[ebx + edx]							// 1
		mov			ecx,			src1								// 1
		lea			eax,			[eax + edx*4]						// 1

		mov			edx,			src2								// 1
		movd		mm2,			[ecx + eax]							// 1
		movd		mm1,			[edx + eax]							// 1
		mov			ecx,			displacemap							// 2
		punpcklbw	mm1,			mm0									// 1
		movzx		eax,			word ptr [ecx + ebx*2 + 2]			// 2
		punpcklbw	mm2,			mm0									// 1
		movzx		eax,			word ptr [esi + eax*2]				// 2
		psubw		mm1,			mm2									// 1
		movsx		edx,			ah									// 2
		pmulhw		mm1,			mm7									// 1
		neg			edx													// 2
		psllw		mm1,			1									// 1
		movsx		eax,			al									// 2
		paddw		mm2,			mm1									// 1
		imul		eax,			pitch								// 2

		lea			edx,			[ebx + edx + 1]						// 2
		mov			ecx,			src1								// 2
		lea			eax,			[eax + edx*4]						// 2

		mov			edx,			src2								// 2
		movd		mm4,			[ecx + eax]							// 2
		movd		mm3,			[edx + eax]							// 2
		punpcklbw	mm3,			mm0									// 2
		punpcklbw	mm4,			mm0									// 2
		psubw		mm3,			mm4									// 2
		pmulhw		mm3,			mm7									// 2
		psllw		mm3,			1									// 2
		add			ebx,			2
		paddw		mm4,			mm3									// 2
		cmp			num,			ebx
		packuswb	mm2,			mm4									// 1,2
		mov			ecx,			displacemap							// 1
		movq		[edi+ebx*4-8],	mm2									// 1,2

		jg			ploop_mmx_f_1

		add			num,			1

		cmp			num,			ebx
		jng			pexit_mmx_f

	ploop_mmx_f_2:
		movzx		eax,			word ptr [ecx + ebx*2]
		movzx		eax,			word ptr [esi + eax*2]
		movsx		edx,			ah
		neg			edx
		lea			edx,			[ebx + edx]
		movsx		eax,			al
		imul		eax,			pitch
		lea			eax,			[eax + edx*4]

		mov			edx,			src2
		mov			ecx,			src1
		movd		mm1,			[edx + eax]
		movd		mm2,			[ecx + eax]
		punpcklbw	mm1,			mm0
		punpcklbw	mm2,			mm0
		psubw		mm1,			mm2
		pmulhw		mm1,			mm7
		psllw		mm1,			1
		paddw		mm2,			mm1
		packuswb	mm2,			mm0
		mov			ecx,			displacemap
		movd		[edi+ebx*4],	mm2

		inc			ebx

		cmp			num,			ebx
		jg			ploop_mmx_f_2

	pexit_mmx_f:

		emms
	}
}
//---------------------------------------------------------------------------
static void TVPRippleTransform_mmx_b(
	const tjs_uint16 *displacemap, const tjs_uint16 *driftmap, tjs_uint32 *dest,
	tjs_int num, tjs_int pitch, const tjs_uint8 * src1, const tjs_uint8 * src2, tjs_int ratio)
{
	_asm
	{
		mov			esi,			driftmap
		mov			edi,			dest
		xor			ebx,			ebx
		mov			ecx,			displacemap
		movd		mm7,			ratio
		psllq		mm7,			7
		punpcklwd	mm7,			mm7
		punpcklwd	mm7,			mm7
		pxor		mm0,			mm0

		sub			num,			1

		cmp			num,			ebx
		jng			pexit_mmx_b

	ploop_mmx_b_1:
		movzx		eax,			word ptr [ecx]						// 1
		movzx		eax,			word ptr [esi + eax*2]				// 1
		movsx		edx,			ah									// 1
		movsx		eax,			al									// 1
		imul		eax,			pitch								// 1
		lea			edx,			[ebx + edx]							// 1
		mov			esi,			src1								// 1
		lea			eax,			[eax + edx*4]						// 1

		mov			edx,			src2								// 1
		movd		mm2,			[esi + eax]							// 1
		movd		mm1,			[edx + eax]							// 1
		mov			esi,			driftmap							// 2
		punpcklbw	mm1,			mm0									// 1
		movzx		eax,			word ptr [ecx - 2]					// 2
		punpcklbw	mm2,			mm0									// 1
		movzx		eax,			word ptr [esi + eax*2]				// 2
		psubw		mm1,			mm2									// 1
		movsx		edx,			ah									// 2
		pmulhw		mm1,			mm7									// 1
		psllw		mm1,			1									// 1
		movsx		eax,			al									// 2
		paddw		mm2,			mm1									// 1
		imul		eax,			pitch								// 2

		lea			edx,			[ebx + edx + 1]						// 2
		mov			esi,			src1								// 2
		lea			eax,			[eax + edx*4]						// 2

		mov			edx,			src2								// 2
		movd		mm4,			[esi + eax]							// 2
		movd		mm3,			[edx + eax]							// 2
		punpcklbw	mm3,			mm0									// 2
		punpcklbw	mm4,			mm0									// 2
		psubw		mm3,			mm4									// 2
		sub			ecx,			4
		pmulhw		mm3,			mm7									// 2
		psllw		mm3,			1									// 2
		add			ebx,			2
		paddw		mm4,			mm3									// 2
		cmp			num,			ebx
		packuswb	mm2,			mm4									// 1,2
		mov			esi,			driftmap							// 1
		movq		[edi+ebx*4-8],	mm2									// 1,2

		jg			ploop_mmx_b_1

		add			num,			1

		cmp			num,			ebx
		jng			pexit_mmx_b

	ploop_mmx_b_2:
		movzx		eax,			word ptr [ecx]
		movzx		eax,			word ptr [esi + eax*2]
		movsx		edx,			ah
		lea			edx,			[ebx + edx]
		movsx		eax,			al
		imul		eax,			pitch
		lea			eax,			[eax + edx*4]

		mov			edx,			src2
		mov			esi,			src1
		movd		mm1,			[edx + eax]
		movd		mm2,			[esi + eax]
		punpcklbw	mm1,			mm0
		punpcklbw	mm2,			mm0
		psubw		mm1,			mm2
		pmulhw		mm1,			mm7
		psllw		mm1,			1
		paddw		mm2,			mm1
		packuswb	mm2,			mm0
		mov			esi,			driftmap
		movd		[edi+ebx*4],	mm2

		inc			ebx
		sub			ecx,			2

		cmp			num,			ebx
		jg			ploop_mmx_b_2

	pexit_mmx_b:

		emms
	}
}
//---------------------------------------------------------------------------
static void TVPRippleTransform_emmx_f(
	const tjs_uint16 *displacemap, const tjs_uint16 *driftmap, tjs_uint32 *dest,
	tjs_int num, tjs_int pitch, const tjs_uint8 * src1, const tjs_uint8 * src2, tjs_int ratio)
{
	_asm
	{
		mov			esi,			driftmap
		mov			edi,			dest
		xor			ebx,			ebx
		mov			ecx,			displacemap
		movd		mm7,			ratio
		psllq		mm7,			7
		punpcklwd	mm7,			mm7
		punpcklwd	mm7,			mm7
		pxor		mm0,			mm0

		sub			num,			1
																		// ���X���b�h
		cmp			num,			ebx
		jng			pexit_emmx_f

	ploop_emmx_f_1:
		movzx		eax,			word ptr [ecx + ebx*2]				// 1
		movzx		eax,			word ptr [esi + eax*2]				// 1
		movsx		edx,			ah									// 1
		movsx		eax,			al									// 1
		neg			edx													// 1
		imul		eax,			pitch								// 1
		lea			edx,			[ebx + edx]							// 1
		mov			ecx,			src1								// 1
		lea			eax,			[eax + edx*4]						// 1

		mov			edx,			src2								// 1
		movd		mm2,			[ecx + eax]							// 1
		movd		mm1,			[edx + eax]							// 1
		mov			ecx,			displacemap							// 2
		punpcklbw	mm1,			mm0									// 1
		movzx		eax,			word ptr [ecx + ebx*2 + 2]			// 2
		punpcklbw	mm2,			mm0									// 1
		movzx		eax,			word ptr [esi + eax*2]				// 2
#if 0
		} __emit__ (0x0f, 0x18, 0x4c, 0x59, 0x10); _asm{ // prefetcht0	[ecx + ebx*2 + 16]
#else
		} _asm _emit 0x0f _asm _emit 0x18 _asm _emit 0x4c _asm _emit 0x59 _asm _emit 0x10 _asm{ // prefetcht0	[ecx + ebx*2 + 16]
#endif
		psubw		mm1,			mm2									// 1
		movsx		edx,			ah									// 2
		pmulhw		mm1,			mm7									// 1
		neg			edx													// 2
		psllw		mm1,			1									// 1
		movsx		eax,			al									// 2
		paddw		mm2,			mm1									// 1
		imul		eax,			pitch								// 2

		lea			edx,			[ebx + edx + 1]						// 2
		mov			ecx,			src1								// 2
		lea			eax,			[eax + edx*4]						// 2

		mov			edx,			src2								// 2
		movd		mm4,			[ecx + eax]							// 2
		movd		mm3,			[edx + eax]							// 2
		punpcklbw	mm3,			mm0									// 2
		punpcklbw	mm4,			mm0									// 2
		psubw		mm3,			mm4									// 2
		pmulhw		mm3,			mm7									// 2
		psllw		mm3,			1									// 2
		add			ebx,			2
		paddw		mm4,			mm3									// 2
		cmp			num,			ebx
		packuswb	mm2,			mm4									// 1,2
		mov			ecx,			displacemap							// 1
		movq		[edi+ebx*4-8],	mm2									// 1,2

		jg			ploop_emmx_f_1

		add			num,			1

		cmp			num,			ebx
		jng			pexit_emmx_f

	ploop_emmx_f_2:
		movzx		eax,			word ptr [ecx + ebx*2]
		movzx		eax,			word ptr [esi + eax*2]
		movsx		edx,			ah
		neg			edx
		lea			edx,			[ebx + edx]
		movsx		eax,			al
		imul		eax,			pitch
		lea			eax,			[eax + edx*4]

		mov			edx,			src2
		mov			ecx,			src1
		movd		mm1,			[edx + eax]
		movd		mm2,			[ecx + eax]
		punpcklbw	mm1,			mm0
		punpcklbw	mm2,			mm0
		psubw		mm1,			mm2
		pmulhw		mm1,			mm7
		psllw		mm1,			1
		paddw		mm2,			mm1
		packuswb	mm2,			mm0
		mov			ecx,			displacemap
		movd		[edi+ebx*4],	mm2

		inc			ebx

		cmp			num,			ebx
		jg			ploop_emmx_f_2

	pexit_emmx_f:

		emms
	}
}
//---------------------------------------------------------------------------
static void TVPRippleTransform_emmx_b(
	const tjs_uint16 *displacemap, const tjs_uint16 *driftmap, tjs_uint32 *dest,
	tjs_int num, tjs_int pitch, const tjs_uint8 * src1, const tjs_uint8 * src2, tjs_int ratio)
{
	_asm
	{
		mov			esi,			driftmap
		mov			edi,			dest
		xor			ebx,			ebx
		mov			ecx,			displacemap
		movd		mm7,			ratio
		psllq		mm7,			7
		punpcklwd	mm7,			mm7
		punpcklwd	mm7,			mm7
		pxor		mm0,			mm0

		sub			num,			1

		cmp			num,			ebx
		jng			pexit_emmx_b

	ploop_emmx_b_1:
		movzx		eax,			word ptr [ecx]						// 1
		movzx		eax,			word ptr [esi + eax*2]				// 1
		movsx		edx,			ah									// 1
		movsx		eax,			al									// 1
		imul		eax,			pitch								// 1
		lea			edx,			[ebx + edx]							// 1
		mov			esi,			src1								// 1
		lea			eax,			[eax + edx*4]						// 1

		mov			edx,			src2								// 1
		movd		mm2,			[esi + eax]							// 1
		movd		mm1,			[edx + eax]							// 1
		mov			esi,			driftmap							// 2
		punpcklbw	mm1,			mm0									// 1
		movzx		eax,			word ptr [ecx - 2]					// 2
		punpcklbw	mm2,			mm0									// 1
		movzx		eax,			word ptr [esi + eax*2]				// 2
		psubw		mm1,			mm2									// 1
		movsx		edx,			ah									// 2
		pmulhw		mm1,			mm7									// 1
#if 0
		} __emit__ (0x0f, 0x18, 0x49, 0xf4); _asm{ // prefetcht0	[ecx - 12]
#else
		} _asm _emit 0x0f _asm _emit 0x18 _asm _emit 0x49 _asm _emit 0xf4 _asm{ // prefetcht0	[ecx - 12]
#endif
		psllw		mm1,			1									// 1
		movsx		eax,			al									// 2
		paddw		mm2,			mm1									// 1
		imul		eax,			pitch								// 2

		lea			edx,			[ebx + edx + 1]						// 2
		mov			esi,			src1								// 2
		lea			eax,			[eax + edx*4]						// 2

		mov			edx,			src2								// 2
		movd		mm4,			[esi + eax]							// 2
		movd		mm3,			[edx + eax]							// 2
		punpcklbw	mm3,			mm0									// 2
		punpcklbw	mm4,			mm0									// 2
		psubw		mm3,			mm4									// 2
		sub			ecx,			4
		pmulhw		mm3,			mm7									// 2
		psllw		mm3,			1									// 2
		add			ebx,			2
		paddw		mm4,			mm3									// 2
		cmp			num,			ebx
		packuswb	mm2,			mm4									// 1,2
		mov			esi,			driftmap							// 1
		movq		[edi+ebx*4-8],	mm2									// 1,2

		jg			ploop_emmx_b_1

		add			num,			1

		cmp			num,			ebx
		jng			pexit_emmx_b

	ploop_emmx_b_2:
		movzx		eax,			word ptr [ecx]
		movzx		eax,			word ptr [esi + eax*2]
		movsx		edx,			ah
		lea			edx,			[ebx + edx]
		movsx		eax,			al
		imul		eax,			pitch
		lea			eax,			[eax + edx*4]

		mov			edx,			src2
		mov			esi,			src1
		movd		mm1,			[edx + eax]
		movd		mm2,			[esi + eax]
		punpcklbw	mm1,			mm0
		punpcklbw	mm2,			mm0
		psubw		mm1,			mm2
		pmulhw		mm1,			mm7
		psllw		mm1,			1
		paddw		mm2,			mm1
		packuswb	mm2,			mm0
		mov			esi,			driftmap
		movd		[edi+ebx*4],	mm2

		inc			ebx
		sub			ecx,			2

		cmp			num,			ebx
		jg			ploop_emmx_b_2

	pexit_emmx_b:

		emms
	}
}
#endif // _MSC_VER && _M_IX86
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
typedef void (*tTVPRippleTransformFunc)(
	const tjs_uint16 *displacemap, const tjs_uint16 *driftmap, tjs_uint32 *dest,
	tjs_int num, tjs_int pitch, const tjs_uint8 * src1, const tjs_uint8 * src2,
	tjs_int ratio);
static tTVPRippleTransformFunc TVPRippleTransform_f = TVPRippleTransform_c_f;
static tTVPRippleTransformFunc TVPRippleTransform_b = TVPRippleTransform_c_b;
//---------------------------------------------------------------------------
static void TVPInitRippleTransformFuncs()
{
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
#define TVP_RIPPLE_TURN_BORDER { \
		if(x<0) x = -x; \
		if(y<0) y = -y; \
		if(x>=srcwidth) x = srcwidth - 1 - (x - srcwidth); \
		if(y>=srcheight) y = srcheight - 1 - (y - srcheight); \
	}
#define TVP_RIPPLE_CALC_OFS tjs_uint ofs = \
		x*sizeof(tjs_uint32) + y*pitch;
//---------------------------------------------------------------------------
static void TVPRippleTransform_f_a_e(
	const tjs_uint16 *displacemap, const tjs_uint16 *driftmap, tjs_uint32 *dest,
	tjs_int num, tjs_int pitch, const tjs_uint8 * src1, const tjs_uint8 * src2,
	tjs_int srcx, tjs_int srcy, tjs_int srcwidth, tjs_int srcheight, tjs_int ratio)
{
	for(int i = 0; i < num; i++)
	{
		tjs_int n = driftmap[displacemap[i]];
		tjs_int x = srcx + i - (int)(char)(n>>8);
		tjs_int y = srcy + (int)(char)n;
		TVP_RIPPLE_TURN_BORDER
		TVP_RIPPLE_CALC_OFS
		TVP_RIPPLE_BLEND
	}
}
//---------------------------------------------------------------------------
static void TVPRippleTransform_f_d_e(
	const tjs_uint16 *displacemap, const tjs_uint16 *driftmap, tjs_uint32 *dest,
	tjs_int num, tjs_int pitch, const tjs_uint8 * src1, const tjs_uint8 * src2,
	tjs_int srcx, tjs_int srcy, tjs_int srcwidth, tjs_int srcheight, tjs_int ratio)
{
	for(int i = 0; i < num; i++)
	{
		tjs_int n = driftmap[displacemap[i]];
		tjs_int x = srcx + i - (int)(char)(n>>8);
		tjs_int y = srcy - (int)(char)n;
		TVP_RIPPLE_TURN_BORDER
		TVP_RIPPLE_CALC_OFS
		TVP_RIPPLE_BLEND
	}
}
//---------------------------------------------------------------------------
static void TVPRippleTransform_b_a_e(
	const tjs_uint16 *displacemap, const tjs_uint16 *driftmap, tjs_uint32 *dest,
	tjs_int num, tjs_int pitch, const tjs_uint8 * src1, const tjs_uint8 * src2,
	tjs_int srcx, tjs_int srcy, tjs_int srcwidth, tjs_int srcheight, tjs_int ratio)
{
	for(int i = 0; i < num; i++)
	{
		tjs_int n = driftmap[*(displacemap--)];
		tjs_int x = srcx + i + (int)(char)(n>>8);
		tjs_int y = srcy + (int)(char)n;
		TVP_RIPPLE_TURN_BORDER
		TVP_RIPPLE_CALC_OFS
		TVP_RIPPLE_BLEND
	}
}
//---------------------------------------------------------------------------
static void TVPRippleTransform_b_d_e(
	const tjs_uint16 *displacemap, const tjs_uint16 *driftmap, tjs_uint32 *dest,
	tjs_int num, tjs_int pitch, const tjs_uint8 * src1, const tjs_uint8 * src2,
	tjs_int srcx, tjs_int srcy, tjs_int srcwidth, tjs_int srcheight, tjs_int ratio)
{
	for(int i = 0; i < num; i++)
	{
		tjs_int n = driftmap[*(displacemap--)];
		tjs_int x = srcx + i + (int)(char)(n>>8);
		tjs_int y = srcy - (int)(char)n;
		TVP_RIPPLE_TURN_BORDER
		TVP_RIPPLE_CALC_OFS
		TVP_RIPPLE_BLEND
	}
}
//---------------------------------------------------------------------------
#undef TVP_RIPPLE_CALC_OFS
#undef TVP_RIPPLE_TURN_BORDER
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#undef TVP_RIPPLE_BLEND
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
// tTVPRippleTransHandler
//---------------------------------------------------------------------------
class tTVPRippleTransHandler : public iTVPDivisibleTransHandler
{
	//	'�g��' �g�����W�V�����n���h���N���X�̎���

	tjs_int RefCount; // �Q�ƃJ�E���^
		/*
			iTVPDivisibleTransHandler �� �Q�ƃJ�E���^�ɂ��Ǘ����s��
		*/

protected:
	tjs_uint64 StartTick; // �g�����W�V�������J�n���� tick count
	tjs_uint64 Time; // �g�����W�V�����ɗv���鎞��
	tTVPLayerType LayerType; // ���C���̃^�C�v
	tjs_int Width; // ��������摜�̕�
	tjs_int Height; // ��������摜�̍���
	tjs_int64 CurTime; // ���݂� tick count
	tjs_int BlendRatio; // �u�����h��
	tjs_int Phase; // �ʑ�
	tjs_int Drift; // �h��
	bool First; // ��ԍŏ��̌Ăяo�����ǂ���

	tjs_int DriftCarePixels; // ���͂̐܂�Ԃ��ɒ��ӂ��Ȃ���΂Ȃ�Ȃ��s�N�Z����

	tjs_int CenterX; // ���S X ���W
	tjs_int CenterY; // ���S Y ���W
	tjs_int RippleWidth; // �g��̕� (16, 32, 64, 128 �̂����ꂩ)
	float Roundness; // �g��̏c/����
	float Speed; // �g��̓����p���x
	tjs_int MaxDrift; // �h��̍ő啝(�s�N�Z���P��) (127�܂�)

	const tjs_uint16 *CurDriftMap; // ���ݕ`�撆�� DirftMap

	tTVPRippleTable *Table; // �u���}�b�v�Ȃǂ̃e�[�u��

#ifdef TVP_DEBUG_RIPPLE_SHOW_UPDATE_COUNT
	tjs_int UpdateCount;
#endif

public:
	tTVPRippleTransHandler(tjs_uint64 time, tTVPLayerType layertype,
		tjs_int width, tjs_int height,
		tjs_int centerx, tjs_int centery,
		tjs_int ripplewidth,
		float roundness, float speed, tjs_int maxdrift)
	{
		RefCount = 1;

		LayerType = layertype;
		Width = width;
		Height = height;
		Time = time;

		CenterX = centerx;
		CenterY = centery;

		RippleWidth = ripplewidth;

		Roundness = roundness;
		Speed = speed;

		First = true;

		MaxDrift = maxdrift;

		Table =
			TVPGetRippleTable(Width, Height, CenterX, CenterY,
				RippleWidth, Roundness, MaxDrift);
	}

	virtual ~tTVPRippleTransHandler()
	{
#ifdef TVP_DEBUG_RIPPLE_SHOW_UPDATE_COUNT
		TVPAddLog(TJS_W("ripple update count : ") + ttstr(UpdateCount));
#endif
		Table->Release();
	}

	tjs_error TJS_INTF_METHOD AddRef()
	{
		// iTVPBaseTransHandler �� AddRef
		// �Q�ƃJ�E���^���C���N�������g
		RefCount ++;
		return TJS_S_OK;
	}

	tjs_error TJS_INTF_METHOD Release()
	{
		// iTVPBaseTransHandler �� Release
		// �Q�ƃJ�E���^���f�N�������g���A0 �ɂȂ�Ȃ�� delete this
		if(RefCount == 1)
			delete this;
		else
			RefCount--;
		return TJS_S_OK;
	}


	tjs_error TJS_INTF_METHOD SetOption(
			/*in*/iTVPSimpleOptionProvider *options // option provider
		)
	{
		// iTVPBaseTransHandler �� SetOption
		// �Ƃ��ɂ�邱�ƂȂ�
		return TJS_S_OK;
	}

	tjs_error TJS_INTF_METHOD StartProcess(tjs_uint64 tick);

	tjs_error TJS_INTF_METHOD EndProcess();

	tjs_error TJS_INTF_METHOD Process(
			tTVPDivisibleData *data);

	tjs_error TJS_INTF_METHOD MakeFinalImage(
			iTVPScanLineProvider ** dest,
			iTVPScanLineProvider * src1,
			iTVPScanLineProvider * src2)
	{
		*dest = src2; // ��ɍŏI�摜�� src2
		return TJS_S_OK;
	}
};
//---------------------------------------------------------------------------
tjs_error TJS_INTF_METHOD tTVPRippleTransHandler::StartProcess(tjs_uint64 tick)
{
	// �g�����W�V�����̉�ʍX�V��񂲂ƂɌĂ΂��

	// �g�����W�V�����̉�ʍX�V���ɂ��A�܂��ŏ��� StartProcess ���Ă΂��
	// ���̂��� Process ��������Ă΂�� ( �̈�𕪊��������Ă���ꍇ )
	// �Ō�� EndProcess ���Ă΂��

	if(First)
	{
		// �ŏ��̎��s
		First = false;
		StartTick = tick;
#ifdef TVP_DEBUG_RIPPLE_SHOW_UPDATE_COUNT
		UpdateCount = 0;
#endif
	}

	// �摜���Z�ɕK�v�Ȋe�p�����[�^���v�Z
	CurTime = (tick - StartTick);

	// BlendRatio
	BlendRatio = CurTime * 255 / Time;
	if(BlendRatio > 255) BlendRatio = 255;

	// Phase
	// �p���x�� Speed (rad/sec) �ŗ^�����Ă���
	Phase = (int)(Speed * ((1.0/(M_PI*2))*(1.0/1000.0)) * CurTime * RippleWidth) % RippleWidth;
	if(Phase < 0) Phase = 0;
	Phase = RippleWidth - Phase - 1;

	// Drift
	float s = sin(M_PI * CurTime / Time);
	Drift = (int)(s * MaxDrift * TVP_RIPPLE_DRIFT_PREC);
	if(Drift < 0) Drift = 0;
	if(Drift >= MaxDrift * TVP_RIPPLE_DRIFT_PREC) Drift = MaxDrift * TVP_RIPPLE_DRIFT_PREC - 1;

	DriftCarePixels = (int)(Drift / TVP_RIPPLE_DRIFT_PREC) + 1;
	if(DriftCarePixels&1) DriftCarePixels ++; // �ꉞ�����ɃA���C��

	// CurDriftMap
	CurDriftMap = Table->GetDriftMap(Drift, Phase);

#ifdef TVP_DEBUG_RIPPLE_SHOW_UPDATE_COUNT
	UpdateCount ++;
#endif

	return TJS_S_TRUE;
}
//---------------------------------------------------------------------------
tjs_error TJS_INTF_METHOD tTVPRippleTransHandler::EndProcess()
{
	// �g�����W�V�����̉�ʍX�V��񕪂��I��邲�ƂɌĂ΂��

	if(BlendRatio == 255) return TJS_S_FALSE; // �g�����W�V�����I��

	return TJS_S_TRUE;
}
//---------------------------------------------------------------------------
tjs_error TJS_INTF_METHOD tTVPRippleTransHandler::Process(
			tTVPDivisibleData *data)
{
	// �g�����W�V�����̊e�̈悲�ƂɌĂ΂��
	// �g���g���͉�ʂ��X�V����Ƃ��ɂ������̗̈�ɕ������Ȃ��珈�����s���̂�
	// ���̃��\�b�h�͒ʏ�A��ʍX�V���ɂ�������Ă΂��

	// data �ɂ͗̈��摜�Ɋւ����񂪓����Ă���

	// �ϐ��̏���
	tjs_int destxofs = data->DestLeft - data->Left;
//	tjs_int destyofs = data->DestTop - data->Top;

	tjs_uint8 *dest;
	tjs_int destpitch;
	const tjs_uint8 *src1;
	tjs_int src1pitch;
	const tjs_uint8 *src2;
	tjs_int src2pitch;
	if(TJS_FAILED(data->Dest->GetScanLineForWrite(data->DestTop, (void**)&dest)))
		return TJS_E_FAIL;
	if(TJS_FAILED(data->Src1->GetScanLine(0, (const void**)&src1)))
		return TJS_E_FAIL;
	if(TJS_FAILED(data->Src2->GetScanLine(0, (const void**)&src2)))
		return TJS_E_FAIL;
	if(TJS_FAILED(data->Dest->GetPitchBytes(&destpitch)))
		return TJS_E_FAIL;
	if(TJS_FAILED(data->Src1->GetPitchBytes(&src1pitch)))
		return TJS_E_FAIL;
	if(TJS_FAILED(data->Src2->GetPitchBytes(&src2pitch)))
		return TJS_E_FAIL;

	if(src1pitch != src2pitch) return TJS_E_FAIL; // ������pitch����v���Ă��Ȃ��Ƒʖ�

	// ���C�����Ƃɏ���
	tjs_int h = data->Height;
	tjs_int y = data->Top;
	while(h--)
	{
		tjs_int l, r;

		if(y < DriftCarePixels || y >= Height - DriftCarePixels)
		{
			// �㉺�̂��݂ł͂ݏo���\��������̂�
			// �܂�Ԃ��]�����s��

			// ���[ �` CenterX
			l = 0;
			r = CenterX;
			if(Clip(l, r, data->Left, data->Left + data->Width))
			{
				if(y < CenterY)
				{
					TVPRippleTransform_b_a_e(
						Table->GetDisplaceMap(CenterX - l - 1, CenterY - y - 1),
						CurDriftMap,
						(tjs_uint32*)dest + l + destxofs, r - l, src1pitch,
						src1, src2, l, y, Width, Height, BlendRatio);
				}
				else
				{
					TVPRippleTransform_b_d_e(
						Table->GetDisplaceMap(CenterX - l - 1, y - CenterY),
						CurDriftMap,
						(tjs_uint32*)dest + l + destxofs, r - l, src1pitch,
						src1, src2, l, y, Width, Height, BlendRatio);
				}
			}

			// CenterX �` �E�[
			l = CenterX;
			r = Width;
			if(Clip(l, r, data->Left, data->Left + data->Width))
			{
				if(y < CenterY)
				{
					TVPRippleTransform_f_a_e(
						Table->GetDisplaceMap(l - CenterX, CenterY - y - 1),
						CurDriftMap,
						(tjs_uint32*)dest + l + destxofs, r - l, src1pitch,
						src1, src2, l, y, Width, Height, BlendRatio);
				}
				else
				{
					TVPRippleTransform_f_d_e(
						Table->GetDisplaceMap(l - CenterX, y - CenterY),
						CurDriftMap,
						(tjs_uint32*)dest + l + destxofs, r - l, src1pitch,
						src1, src2, l, y, Width, Height, BlendRatio);
				}
			}

		}
		else
		{
			// ���[ �` CenterX
			l = 0;
			r = CenterX;
			if(Clip(l, r, data->Left, data->Left + data->Width))
			{
				int ll, rr;
				ll = 0, rr = DriftCarePixels;
				if(Clip(ll, rr, l, r))
				{
					// ���� ll �` rr �ŕ\����鍶�[�� ���[�ɂ͂ݏo���\��������
					// �̂Ő܂�Ԃ��]����������
					if(y < CenterY)
					{
						TVPRippleTransform_b_a_e(
							Table->GetDisplaceMap(CenterX - ll - 1, CenterY - y - 1),
							CurDriftMap,
							(tjs_uint32*)dest + ll + destxofs, rr - ll, src1pitch,
							src1, src2, ll, y, Width, Height, BlendRatio);
					}
					else
					{
						TVPRippleTransform_b_d_e(
							Table->GetDisplaceMap(CenterX - ll - 1, y - CenterY),
							CurDriftMap,
							(tjs_uint32*)dest + ll + destxofs, rr - ll, src1pitch,
							src1, src2, ll, y, Width, Height, BlendRatio);
					}
				}

				ll = DriftCarePixels; rr = r;
				if(Clip(ll, rr, l, r))
				{
					// �����͂͂ݏo���Ȃ�
					if(y < CenterY)
					{
						TVPRippleTransform_b(
							Table->GetDisplaceMap(CenterX - ll - 1, CenterY - y - 1),
							CurDriftMap,
							(tjs_uint32*)dest + ll + destxofs,
							rr - ll,
							src1pitch,
							(const tjs_uint8 *)((const tjs_uint32*)(src1 + y*src1pitch) + ll),
							(const tjs_uint8 *)((const tjs_uint32*)(src2 + y*src2pitch) + ll),
							BlendRatio);
					}
					else
					{
						TVPRippleTransform_b(
							Table->GetDisplaceMap(CenterX - ll - 1, y - CenterY),
							CurDriftMap,
							(tjs_uint32*)dest + ll + destxofs,
							rr - ll,
							-src1pitch,
							(const tjs_uint8 *)((const tjs_uint32*)(src1 + y*src1pitch) + ll),
							(const tjs_uint8 *)((const tjs_uint32*)(src2 + y*src2pitch) + ll),
							BlendRatio);
					}
				}
			}

			// CenterX �` �E�[
			l = CenterX;
			r = Width;
			if(Clip(l, r, data->Left, data->Left + data->Width))
			{
				int ll, rr;
				ll = l, rr = Width - DriftCarePixels;
				if(Clip(ll, rr, l, r))
				{
					// �����͂͂ݏo���Ȃ�
					if(y < CenterY)
					{
						TVPRippleTransform_f(
							Table->GetDisplaceMap(ll - CenterX, CenterY - y - 1),
							CurDriftMap,
							(tjs_uint32*)dest + ll + destxofs,
							rr - ll,
							src1pitch,
							(const tjs_uint8 *)((const tjs_uint32*)(src1 + y*src1pitch) + ll),
							(const tjs_uint8 *)((const tjs_uint32*)(src2 + y*src2pitch) + ll),
							BlendRatio);
					}
					else
					{
						TVPRippleTransform_f(
							Table->GetDisplaceMap(ll - CenterX, y - CenterY),
							CurDriftMap,
							(tjs_uint32*)dest + ll + destxofs,
							rr - ll,
							-src1pitch,
							(const tjs_uint8 *)((const tjs_uint32*)(src1 + y*src1pitch) + ll),
							(const tjs_uint8 *)((const tjs_uint32*)(src2 + y*src2pitch) + ll),
							BlendRatio);
					}
				}

				ll = Width - DriftCarePixels, rr = r;
				if(Clip(ll, rr, l, r))
				{
					// ���� ll �` rr �ŕ\�����E�[�� �E�[�ɂ͂ݏo���\��������
					// �̂Ő܂�Ԃ��]����������
					if(y < CenterY)
					{
						TVPRippleTransform_f_a_e(
							Table->GetDisplaceMap(ll - CenterX, CenterY - y - 1),
							CurDriftMap,
							(tjs_uint32*)dest + ll + destxofs, rr - ll, src1pitch,
							src1, src2, ll, y, Width, Height, BlendRatio);
					}
					else
					{
						TVPRippleTransform_f_d_e(
							Table->GetDisplaceMap(ll - CenterX, y - CenterY),
							CurDriftMap,
							(tjs_uint32*)dest + ll + destxofs, rr - ll, src1pitch,
							src1, src2, ll, y, Width, Height, BlendRatio);
					}
				}
			}
		}

		dest += destpitch;
		y++;
	}

	return TJS_S_OK;
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
class tTVPRippleTransHandlerProvider : public iTVPTransHandlerProvider
{
	tjs_uint RefCount; // �Q�ƃJ�E���^
public:
	tTVPRippleTransHandlerProvider() { RefCount = 1; }
	~tTVPRippleTransHandlerProvider() {; }

	tjs_error TJS_INTF_METHOD AddRef()
	{
		// iTVPBaseTransHandler �� AddRef
		// �Q�ƃJ�E���^���C���N�������g
		RefCount ++;
		return TJS_S_OK;
	}

	tjs_error TJS_INTF_METHOD Release()
	{
		// iTVPBaseTransHandler �� Release
		// �Q�ƃJ�E���^���f�N�������g���A0 �ɂȂ�Ȃ�� delete this
		if(RefCount == 1)
			delete this;
		else
			RefCount--;
		return TJS_S_OK;
	}

	tjs_error TJS_INTF_METHOD GetName(
			/*out*/const tjs_char ** name)
	{
		// ���̃g�����W�V�����̖��O��Ԃ�
		if(name) *name = TJS_W("ripple");
		return TJS_S_OK;
	}


	tjs_error TJS_INTF_METHOD StartTransition(
			/*in*/iTVPSimpleOptionProvider *options, // option provider
			/*in*/iTVPSimpleImageProvider *imagepro, // image provider
			/*in*/tTVPLayerType layertype, // destination layer type
			/*in*/tjs_uint src1w, tjs_uint src1h, // source 1 size
			/*in*/tjs_uint src2w, tjs_uint src2h, // source 2 size
			/*out*/tTVPTransType *type, // transition type
			/*out*/tTVPTransUpdateType * updatetype, // update typwe
			/*out*/iTVPBaseTransHandler ** handler // transition handler
			)
	{
		if(type) *type = ttExchange; // transition type : exchange
		if(updatetype) *updatetype = tutDivisible;
			// update type : divisible
		if(!handler) return TJS_E_FAIL;
		if(!options) return TJS_E_FAIL;

		if(src1w != src2w || src1h != src2h)
			return TJS_E_FAIL; // src1 �� src2 �̃T�C�Y����v���Ă��Ȃ��Ƒʖ�


		// �I�v�V�����𓾂�
		tTJSVariant tmp;
		tjs_uint64 time;

		tjs_int centerx = src1w >> 1, centery = src1h >> 1;
		tjs_int ripplewidth = 128;
		float roundness = 1.0;
		float speed = 6;
		tjs_int maxdrift = 24;
//		tjs_int rippletype = 0; // �^�C�v


		if(TJS_FAILED(options->GetValue(TJS_W("time"), &tmp)))
			return TJS_E_FAIL; // time �������w�肳��Ă��Ȃ�
		if(tmp.Type() == tvtVoid) return TJS_E_FAIL;
		time = (tjs_int64)tmp;
		if(time < 2) time = 2; // ���܂菬���Ȑ��l���w�肷��Ɩ�肪�N����̂�

		if(TJS_SUCCEEDED(options->GetValue(TJS_W("centerx"), &tmp)))
			if(tmp.Type() != tvtVoid) centerx = (tjs_int)tmp;

		if(TJS_SUCCEEDED(options->GetValue(TJS_W("centery"), &tmp)))
			if(tmp.Type() != tvtVoid) centery = (tjs_int)tmp;

		if(centerx < 0 || centery < 0 ||
			(tjs_uint)centerx >= src1w || (tjs_uint)centery >= src1h)
			TVPThrowExceptionMessage(TJS_W("centerx and centery cannot be out of the image"));


		if(TJS_SUCCEEDED(options->GetValue(TJS_W("rwidth"), &tmp)))
			if(tmp.Type() != tvtVoid) ripplewidth = (tjs_int)tmp;

		if(ripplewidth != 16 && ripplewidth != 32 && ripplewidth != 64 &&
			ripplewidth != 128)
			TVPThrowExceptionMessage(TJS_W("rwidth must be 16, 32, 64 or 128"));


		if(TJS_SUCCEEDED(options->GetValue(TJS_W("roundness"), &tmp)))
			if(tmp.Type() != tvtVoid) roundness = (float)(double)tmp;

		if(roundness <= 0.0)
			TVPThrowExceptionMessage(TJS_W("roundness cannot be nagative or equal to 0"));

		if(TJS_SUCCEEDED(options->GetValue(TJS_W("speed"), &tmp)))
			if(tmp.Type() != tvtVoid) speed = (float)(double)tmp;

		if(TJS_SUCCEEDED(options->GetValue(TJS_W("maxdrift"), &tmp)))
			if(tmp.Type() != tvtVoid) maxdrift = (tjs_int)tmp;
		if(maxdrift < 0 || maxdrift >= 128)
			TVPThrowExceptionMessage(TJS_W("maxdrift cannot be nagative or larger than 127"));

		if((tjs_uint)maxdrift >= src1w || (tjs_uint)maxdrift >= src1h)
			TVPThrowExceptionMessage(TJS_W("maxdrift must be lesser than both image width and height"));

		// �I�u�W�F�N�g���쐬
		*handler = new tTVPRippleTransHandler(time, layertype,
			src1w, src1h, centerx, centery,
			ripplewidth, roundness, speed, maxdrift);

		return TJS_S_OK;
	}

} static * RippleTransHandlerProvider;
//---------------------------------------------------------------------------
void RegisterRippleTransHandlerProvider()
{
	TVPInitRippleTableCache(); // �e�[�u���̃L���b�V���̏�����

	TVPInitRippleTransformFuncs(); // ���Z�֐��̏�����

	// TVPAddTransHandlerProvider ���g���ăg�����W�V�����n���h���v���o�C�_��
	// �o�^����
	RippleTransHandlerProvider = new tTVPRippleTransHandlerProvider();
	TVPAddTransHandlerProvider(RippleTransHandlerProvider);
}
//---------------------------------------------------------------------------
void UnregisterRippleTransHandlerProvider()
{
	// TVPRemoveTransHandlerProvider ���g���ăg�����W�V�����n���h���v���o�C�_��
	// �o�^��������
	TVPRemoveTransHandlerProvider(RippleTransHandlerProvider);
	RippleTransHandlerProvider->Release();

	TVPClearRippleTableCache(); // �u���}�b�v�Ȃǂ̃e�[�u���̃L���b�V���̃N���A
}
//---------------------------------------------------------------------------
