#define NCB_MODULE_NAME TJS_W("layerExImage.dll")
#include "ncbind.hpp"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <algorithm>

#include "layerExBase.hpp"

typedef unsigned char BYTE;
typedef unsigned short WORD;

struct RGBQUAD {
	BYTE rgbBlue;
	BYTE rgbGreen;
	BYTE rgbRed;
	BYTE rgbReserved;
};

#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

class layerExImage : public layerExBase
{
public:
	layerExImage(DispatchT obj) : layerExBase(obj) {}

	virtual void reset() {
		layerExBase::reset();
		_buffer += _clipTop * _pitch + _clipLeft * 4;
		_width  = _clipWidth;
		_height = _clipHeight;
	}

	void lut(BYTE* pLut) {
		BYTE *src = (BYTE*)_buffer;
		for (int i=0; i < _height; i++){
			BYTE *p = src;
			for (int j=0; j < _width; j++) {
				p[0] = pLut[p[0]]; // B
				p[1] = pLut[p[1]]; // G
				p[2] = pLut[p[2]]; // R
				p += 4;            // skip A
			}
			src += _pitch;
		}
	}

	void light(int brightness, int contrast) {
		float c = (100 + contrast)/100.0f;
		brightness += 128;
		BYTE cTable[256];
		for (int i=0; i<256; i++) {
			cTable[i] = (BYTE)max(0, min(255, (int)((i-128)*c + brightness)));
		}
		lut(cTable);
		redraw();
	}

	void colorize(int hue, int sat, double blend) {
		if (blend < 0.0) blend = 0.0;
		if (blend > 1.0) blend = 1.0;
		int a0 = (int)(256*blend);
		int a1 = 256 - a0;

		bool bFullBlend = (blend > 0.999);

		RGBQUAD color, hsl;

		BYTE *src = (BYTE*)_buffer;
		for (int y=0; y<_height; y++){
			BYTE *p = src;
			for (int x=0; x<_width; x++){
				color.rgbBlue  = p[0];
				color.rgbGreen = p[1];
				color.rgbRed   = p[2];
				if (bFullBlend){
					color = RGBtoHSL(color);
					color.rgbRed = hue;
					color.rgbGreen = sat;
					color = HSLtoRGB(color);
				} else {
					hsl = RGBtoHSL(color);
					hsl.rgbRed = hue;
					hsl.rgbGreen = sat;
					hsl = HSLtoRGB(hsl);
					color.rgbRed   = (BYTE)((hsl.rgbRed   * a0 + color.rgbRed   * a1) >> 8);
					color.rgbBlue  = (BYTE)((hsl.rgbBlue  * a0 + color.rgbBlue  * a1) >> 8);
					color.rgbGreen = (BYTE)((hsl.rgbGreen * a0 + color.rgbGreen * a1) >> 8);
				}
				*p++ = color.rgbBlue;
				*p++ = color.rgbGreen;
				*p++ = color.rgbRed;
				p++;
			}
			src += _pitch;
		}
		redraw();
	}

	void modulate(int hue, int saturation, int luminance) {
		double h = hue / 360.0;
		double s = saturation / 100.0;
		double l = luminance / 100.0;

		BYTE *src = (BYTE*)_buffer;
		for (int y=0; y<_height; y++){
			BYTE *p = src;
			for (int x=0; x<_width; x++){
				int b = p[0];
				int g = p[1];
				int r = p[2];
				modulatePixel(b, g, r, h, s, l);
				*p++ = b;
				*p++ = g;
				*p++ = r;
				p++;
			}
			src += _pitch;
		}
		redraw();
	}

	void noise(int level) {
		BYTE *src = (BYTE*)_buffer;
		for (int y=0; y<_height; y++){
			BYTE *p = src;
			for (int x=0; x<_width; x++){
				int n = (int)((rand()/(float)RAND_MAX - 0.5f)*level);
				p[0] = (BYTE)max(0, min(255, (int)(p[0] + n)));
				n = (int)((rand()/(float)RAND_MAX - 0.5f)*level);
				p[1] = (BYTE)max(0, min(255, (int)(p[1] + n)));
				n = (int)((rand()/(float)RAND_MAX - 0.5f)*level);
				p[2] = (BYTE)max(0, min(255, (int)(p[2] + n)));
				p += 4;
			}
			src += _pitch;
		}
		redraw();
	}

	void generateWhiteNoise() {
		BYTE *src = (BYTE*)_buffer;
		for (int y=0; y<_height; y++){
			BYTE *p = src;
			for (int x=0; x<_width; x++, p+=4){
				BYTE n = (BYTE)(rand() % 256);
				p[2] = p[1] = p[0] = n;
			}
			src += _pitch;
		}
		redraw();
	}

	void gaussianBlur(float radius) {
		int tmppitch = _width * 4;
		BYTE *tmpbuf = new BYTE[tmppitch * _height];

		float *cmatrix = NULL;
		int32_t cmatrix_length = gen_convolve_matrix(radius, &cmatrix);
		float *ctable = gen_lookup_table(cmatrix, cmatrix_length);

		int32_t bypp = 4;

		for (int32_t y=0; y<_height; y++) {
			blur_line(ctable, cmatrix, cmatrix_length, _buffer + _pitch * y, tmpbuf + tmppitch * y, _width, bypp);
		}

		BYTE* cur_col  = new BYTE[_height * 4];
		BYTE* dest_col = new BYTE[_height * 4];
		for (int32_t x=0; x<_width; x++) {
			getCol(tmpbuf + x*4, cur_col, _height, tmppitch);
			blur_line(ctable, cmatrix, cmatrix_length, cur_col, dest_col, _height, bypp);
			setCol(_buffer + x*4, dest_col, _height, _pitch);
		}
		delete[] cur_col;
		delete[] dest_col;

		delete[] cmatrix;
		delete[] ctable;
		delete[] tmpbuf;
		redraw();
	}

private:
	// HSL constants
	enum { HSLMAX = 255, RGBMAX = 255, HSLUNDEFINED = HSLMAX*2/3 };

	static RGBQUAD RGBtoHSL(RGBQUAD lRGBColor) {
		BYTE R = lRGBColor.rgbRed;
		BYTE G = lRGBColor.rgbGreen;
		BYTE B = lRGBColor.rgbBlue;

		BYTE cMax = max(max(R,G), B);
		BYTE cMin = min(min(R,G), B);
		BYTE H, L, S;
		L = (BYTE)((((cMax+cMin)*HSLMAX)+RGBMAX)/(2*RGBMAX));

		if (cMax == cMin) {
			S = 0;
			H = HSLUNDEFINED;
		} else {
			if (L <= (HSLMAX/2))
				S = (BYTE)((((cMax-cMin)*HSLMAX)+((cMax+cMin)/2))/(cMax+cMin));
			else
				S = (BYTE)((((cMax-cMin)*HSLMAX)+((2*RGBMAX-cMax-cMin)/2))/(2*RGBMAX-cMax-cMin));

			WORD Rdelta = (WORD)((((cMax-R)*(HSLMAX/6)) + ((cMax-cMin)/2)) / (cMax-cMin));
			WORD Gdelta = (WORD)((((cMax-G)*(HSLMAX/6)) + ((cMax-cMin)/2)) / (cMax-cMin));
			WORD Bdelta = (WORD)((((cMax-B)*(HSLMAX/6)) + ((cMax-cMin)/2)) / (cMax-cMin));

			if (R == cMax)
				H = (BYTE)(Bdelta - Gdelta);
			else if (G == cMax)
				H = (BYTE)((HSLMAX/3) + Rdelta - Bdelta);
			else
				H = (BYTE)(((2*HSLMAX)/3) + Gdelta - Rdelta);

			if (H > HSLMAX) H -= HSLMAX;
		}
		RGBQUAD hsl = {L, S, H, 0};
		return hsl;
	}

	static float HueToRGB(float n1, float n2, float hue) {
		if (hue > 360) hue -= 360;
		else if (hue < 0) hue += 360;

		if (hue < 60)
			return n1 + (n2-n1)*hue/60.0f;
		else if (hue < 180)
			return n2;
		else if (hue < 240)
			return n1 + (n2-n1)*(240-hue)/60.0f;
		else
			return n1;
	}

	static RGBQUAD HSLtoRGB(RGBQUAD lHSLColor) {
		float h = (float)lHSLColor.rgbRed * 360.0f/255.0f;
		float s = (float)lHSLColor.rgbGreen/255.0f;
		float l = (float)lHSLColor.rgbBlue/255.0f;

		float m2 = (l <= 0.5f) ? l * (1+s) : l + s - l*s;
		float m1 = 2 * l - m2;

		BYTE r, g, b;
		if (s == 0) {
			r = g = b = (BYTE)(l*255.0f);
		} else {
			r = (BYTE)(HueToRGB(m1, m2, h+120) * 255.0f);
			g = (BYTE)(HueToRGB(m1, m2, h) * 255.0f);
			b = (BYTE)(HueToRGB(m1, m2, h-120) * 255.0f);
		}

		RGBQUAD rgb = {b, g, r, 0};
		return rgb;
	}

	static int hue2rgb(double n1, double n2, double hue) {
		double color;
		if (hue < 0) hue += 1.0;
		else if (hue > 1.0) hue -= 1.0;
		if (hue < 1.0/6.0)
			color = n1 + (n2-n1)*hue*6.0;
		else if (hue < 1.0/2.0)
			color = n2;
		else if (hue < 2.0/3.0)
			color = n1 + (n2-n1)*(2.0/3.0-hue)*6.0;
		else
			color = n1;
		return (int)(color * 255.0);
	}

	static void modulatePixel(int &b, int &g, int &r, double h, double s, double l) {
		double red   = r / 255.0;
		double green = g / 255.0;
		double blue  = b / 255.0;

		double cMax = max(max(red,green), blue);
		double cMin = min(min(red,green), blue);
		double delta = cMax - cMin;
		double add   = cMax + cMin;
		double luminance = add/2.0;
		double hue, saturation;

		if (delta == 0) {
			saturation = 0;
			hue = 0;
		} else {
			saturation = (luminance < 0.5) ? delta/add : delta/(2.0-add);
			if (red == cMax)
				hue = (green - blue)/delta;
			else if (green == cMax)
				hue = 2.0 + (blue - red)/delta;
			else
				hue = 4.0 + (red - green)/delta;
			hue /= 6.0;
		}

		hue += h;
		while (hue < 0) hue += 1.0;
		while (hue > 1.0) hue -= 1.0;
		if (s > 0)
			saturation += (1.0 - saturation) * s;
		else
			saturation += saturation * s;
		if (l > 0)
			luminance += (1.0 - luminance) * l;
		else
			luminance += luminance * l;

		if (saturation == 0.0) {
			r = g = b = (int)(luminance * 255.0);
		} else {
			double m2 = (luminance <= 0.5) ? luminance * (1+saturation) : luminance + saturation - luminance * saturation;
			double m1 = 2.0 * luminance - m2;
			r = hue2rgb(m1, m2, hue+1.0/3.0);
			g = hue2rgb(m1, m2, hue);
			b = hue2rgb(m1, m2, hue-1.0/3.0);
		}
	}

	// Gaussian blur helpers
	static int32_t gen_convolve_matrix(float radius, float **cmatrix_p) {
		radius = (float)fabs(0.5*radius) + 0.25f;
		float std_dev = radius;
		radius = std_dev * 2;

		int32_t matrix_length = (int32_t)(2 * ceil(radius-0.5) + 1);
		if (matrix_length <= 0) matrix_length = 1;
		*cmatrix_p = new float[matrix_length];
		float *cmatrix = *cmatrix_p;

		for (int32_t i = matrix_length/2 + 1; i < matrix_length; i++) {
			float base_x = i - (float)floor((float)(matrix_length/2)) - 0.5f;
			float sum = 0;
			for (int32_t j = 1; j <= 50; j++) {
				if (base_x+0.02f*j <= radius)
					sum += (float)exp(-(base_x+0.02f*j)*(base_x+0.02f*j) / (2*std_dev*std_dev));
			}
			cmatrix[i] = sum/50;
		}

		for (int32_t i=0; i<=matrix_length/2; i++)
			cmatrix[i] = cmatrix[matrix_length-1-i];

		float sum = 0;
		for (int32_t j=0; j<=50; j++)
			sum += (float)exp(-(0.5f+0.02f*j)*(0.5f+0.02f*j) / (2*std_dev*std_dev));
		cmatrix[matrix_length/2] = sum/51;

		sum = 0;
		for (int32_t i=0; i<matrix_length; i++) sum += cmatrix[i];
		for (int32_t i=0; i<matrix_length; i++) cmatrix[i] = cmatrix[i] / sum;

		return matrix_length;
	}

	static float* gen_lookup_table(float *cmatrix, int32_t cmatrix_length) {
		float* lookup_table = new float[cmatrix_length * 256];
		float* lookup_table_p = lookup_table;
		float* cmatrix_p = cmatrix;
		for (int32_t i=0; i<cmatrix_length; i++) {
			for (int32_t j=0; j<256; j++)
				*(lookup_table_p++) = *cmatrix_p * (float)j;
			cmatrix_p++;
		}
		return lookup_table;
	}

	static void blur_line(float *ctable, float *cmatrix, int32_t cmatrix_length,
						  uint8_t* cur_col, uint8_t* dest_col, int32_t y, int32_t bytes) {
		float scale, sum;
		int32_t i, j, row;
		int32_t cmatrix_middle = cmatrix_length/2;

		uint8_t *cur_col_p, *cur_col_p1, *dest_col_p;
		float *ctable_p;

		if (cmatrix_length > y) {
			for (row = 0; row < y; row++) {
				scale = 0;
				for (j = 0; j < y; j++) {
					if ((j + cmatrix_middle - row >= 0) &&
						(j + cmatrix_middle - row < cmatrix_length))
						scale += cmatrix[j + cmatrix_middle - row];
				}
				for (i = 0; i<bytes; i++) {
					sum = 0;
					for (j = 0; j < y; j++) {
						if ((j >= row - cmatrix_middle) &&
							(j <= row + cmatrix_middle))
							sum += cur_col[j*bytes + i] * cmatrix[j];
					}
					dest_col[row*bytes + i] = (uint8_t)(0.5f + sum / scale);
				}
			}
		} else {
			for (row = 0; row < cmatrix_middle; row++) {
				scale = 0;
				for (j = cmatrix_middle - row; j<cmatrix_length; j++)
					scale += cmatrix[j];
				for (i = 0; i<bytes; i++) {
					sum = 0;
					for (j = cmatrix_middle - row; j<cmatrix_length; j++)
						sum += cur_col[(row + j-cmatrix_middle)*bytes + i] * cmatrix[j];
					dest_col[row*bytes + i] = (uint8_t)(0.5f + sum / scale);
				}
			}
			dest_col_p = dest_col + row*bytes;
			for (; row < y-cmatrix_middle; row++) {
				cur_col_p = (row - cmatrix_middle) * bytes + cur_col;
				for (i = 0; i<bytes; i++) {
					sum = 0;
					cur_col_p1 = cur_col_p;
					ctable_p = ctable;
					for (j = cmatrix_length; j>0; j--) {
						sum += *(ctable_p + *cur_col_p1);
						cur_col_p1 += bytes;
						ctable_p += 256;
					}
					cur_col_p++;
					*(dest_col_p++) = (uint8_t)(0.5f + sum);
				}
			}
			for (; row < y; row++) {
				scale = 0;
				for (j = 0; j < y-row + cmatrix_middle; j++)
					scale += cmatrix[j];
				for (i = 0; i<bytes; i++) {
					sum = 0;
					for (j = 0; j < y-row + cmatrix_middle; j++)
						sum += cur_col[(row + j-cmatrix_middle)*bytes + i] * cmatrix[j];
					dest_col[row*bytes + i] = (uint8_t)(0.5f + sum / scale);
				}
			}
		}
	}

	static void getCol(BYTE *src, BYTE *dest, int height, int pitch) {
		pitch -= 4;
		for (int i=0; i<height; i++) {
			*dest++ = *src++;
			*dest++ = *src++;
			*dest++ = *src++;
			*dest++ = *src++;
			src += pitch;
		}
	}

	static void setCol(BYTE *src, BYTE *dest, int height, int pitch) {
		pitch -= 4;
		for (int i=0; i<height; i++) {
			*src++ = *dest++;
			*src++ = *dest++;
			*src++ = *dest++;
			*src++ = *dest++;
			src += pitch;
		}
	}
};

static const char *copyright =
"----- CxImage Copyright START -----\n"
"CxImage version 7.0.2 07/Feb/2011\n"
"CxImage : Copyright (C) 2001 - 2011, Davide Pizzolato\n"
"Original CImage and CImageIterator implementation are:\n"
"Copyright (C) 1995, Alejandro Aguilar Sierra\n"
"----- CxImage Copyright END -----\n";

NCB_GET_INSTANCE_HOOK(layerExImage)
{
	NCB_INSTANCE_GETTER(objthis) {
		ClassT* obj = GetNativeInstance(objthis);
		if (!obj) {
			obj = new ClassT(objthis);
			SetNativeInstance(objthis, obj);
		}
		obj->reset();
		return obj;
	}
	~NCB_GET_INSTANCE_HOOK_CLASS () {
	}
};

NCB_ATTACH_CLASS_WITH_HOOK(layerExImage, Layer) {
	NCB_METHOD(light);
	NCB_METHOD(colorize);
	NCB_METHOD(modulate);
	NCB_METHOD(noise);
	NCB_METHOD(generateWhiteNoise);
	NCB_METHOD(gaussianBlur);
}

void layerExImage_init()
{
	TVPAddImportantLog(ttstr(copyright));
}

NCB_PRE_REGIST_CALLBACK(layerExImage_init);
