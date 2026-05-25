#ifndef commonH
#define commonH

#include "tjsTypes.h"
#include "tvpgl.h"

//---------------------------------------------------------------------------
static inline bool Clip(tjs_int &l, tjs_int &r, tjs_int cl, tjs_int cr)
{
	// ���� l <-> r (l<r) ���A cl <-> cr (cl<cr) �ŃN���b�s���O���A���ʂ� l r �ɕԂ�
	// �N���b�s���O�������ʁA���� l - r ���c��ΐ^�A���ł���΋U��Ԃ�
	if(l < cl) l = cl;
	if(r > cr) r = cr;
	if(l >= r) return false;
	return true;
}
//---------------------------------------------------------------------------
static inline tjs_uint32 Blend(tjs_uint32 a, tjs_uint32 b, tjs_int opa)
{
	// a �� b �������� opa �ō������ĕԂ� ( opa = 0 �` 255, 0 = a, 255 = b )
	tjs_uint32 ret;
	tjs_uint32 tmp;

	tmp = a & 0x000000ff;  ret   = 0x000000ff & (tmp + (( (b & 0x000000ff) - tmp ) * opa >> 8));
	tmp = a & 0x0000ff00;  ret  |= 0x0000ff00 & (tmp + (( (b & 0x0000ff00) - tmp ) * opa >> 8));
	tmp = a & 0x00ff0000;  ret  |= 0x00ff0000 & (tmp + (( (b & 0x00ff0000) - tmp ) * opa >> 8));
	tmp = a >> 24;
	ret  |= (0x000000ff & (tmp + (( (b >> 24) - tmp ) * opa >> 8))) << 24;

	return ret;
}
//---------------------------------------------------------------------------
static inline void Swap_tjs_int(tjs_int &a, tjs_int &b)
{
	// a �� b �����ւ���
	tjs_int tmp = a;
	a = b;
	b = tmp;
}
//---------------------------------------------------------------------------

#endif

