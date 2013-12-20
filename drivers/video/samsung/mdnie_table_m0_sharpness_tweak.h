#include "mdnie.h"

static unsigned short tune_dynamic_gallery_sharp_tweak[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x0088,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x0090, 0x0080,	/*DE egth*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1404,	/*CS weight grayTH*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r   0*/
	0x0021, 0x0d93,	/*CC lut r  16 144*/
	0x0022, 0x1aa5,	/*CC lut r  32 160*/
	0x0023, 0x29b7,	/*CC lut r  48 176*/
	0x0024, 0x39c8,	/*CC lut r  64 192*/
	0x0025, 0x4bd8,	/*CC lut r  80 208*/
	0x0026, 0x5de6,	/*CC lut r  96 224*/
	0x0027, 0x6ff4,	/*CC lut r 112 240*/
	0x0028, 0x81ff,	/*CC lut r 128 255*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000,
};

static unsigned short tune_dynamic_ui_sharp_tweak[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x0088,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1804,	/*CS weight grayTH*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r   0*/
	0x0021, 0x0d93,	/*CC lut r  16 144*/
	0x0022, 0x1aa5,	/*CC lut r  32 160*/
	0x0023, 0x29b7,	/*CC lut r  48 176*/
	0x0024, 0x39c8,	/*CC lut r  64 192*/
	0x0025, 0x4bd8,	/*CC lut r  80 208*/
	0x0026, 0x5de6,	/*CC lut r  96 224*/
	0x0027, 0x6ff4,	/*CC lut r 112 240*/
	0x0028, 0x81ff,	/*CC lut r 128 255*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000,
};

static unsigned short tune_dynamic_video_sharp_tweak[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x0088,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1404,	/*CS weight grayTH*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00ff,	/*SCR KgWg*/
	0x00ec, 0x00ff,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r   0*/
	0x0021, 0x0d93,	/*CC lut r  16 144*/
	0x0022, 0x1aa5,	/*CC lut r  32 160*/
	0x0023, 0x29b7,	/*CC lut r  48 176*/
	0x0024, 0x39c8,	/*CC lut r  64 192*/
	0x0025, 0x4bd8,	/*CC lut r  80 208*/
	0x0026, 0x5de6,	/*CC lut r  96 224*/
	0x0027, 0x6ff4,	/*CC lut r 112 240*/
	0x0028, 0x81ff,	/*CC lut r 128 255*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000,
};

static unsigned short tune_dynamic_vt_sharp_tweak[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x008a,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0005,	/*FA cs1 | de8 dnr4 hdr2 fa1*/
	0x0039, 0x0080,	/*FA dnrWeight*/
	0x0080, 0x0fff,	/*DNR dirTh*/
	0x0081, 0x19ff,	/*DNR dirnumTh decon7Th*/
	0x0082, 0xff16,	/*DNR decon5Th maskTh*/
	0x0083, 0x0000,	/*DNR blTh*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1a04,	/*CS weight grayTH*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r  0*/
	0x0021, 0x0d93,	/*CC lut r  16 144*/
	0x0022, 0x1aa5,	/*CC lut r  32 160*/
	0x0023, 0x29b7,	/*CC lut r  48 176*/
	0x0024, 0x39c8,	/*CC lut r  64 192*/
	0x0025, 0x4bd8,	/*CC lut r  80 208*/
	0x0026, 0x5de6,	/*CC lut r  96 224*/
	0x0027, 0x6ff4,	/*CC lut r 112 240*/
	0x0028, 0x81ff,	/*CC lut r 128 255*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000,
};

static unsigned short tune_movie_gallery_sharp_tweak[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x00a0,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x00e1, 0xd6ac,	/*SCR RrCr*/
	0x00e2, 0x32ff,	/*SCR RgCg*/
	0x00e3, 0x2ef0,	/*SCR RbCb*/
	0x00e4, 0xa5fa,	/*SCR GrMr*/
	0x00e5, 0xff4d,	/*SCR GgMg*/
	0x00e6, 0x59ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00fb,	/*SCR BgYg*/
	0x00e9, 0xff61,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00f8,	/*SCR KgWg*/
	0x00ec, 0x00f1,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r   0*/
	0x0021, 0x1090,	/*CC lut r  16 144*/
	0x0022, 0x20a0,	/*CC lut r  32 160*/
	0x0023, 0x30b0,	/*CC lut r  48 176*/
	0x0024, 0x40c0,	/*CC lut r  64 192*/
	0x0025, 0x50d0,	/*CC lut r  80 208*/
	0x0026, 0x60e0,	/*CC lut r  96 224*/
	0x0027, 0x70f0,	/*CC lut r 112 240*/
	0x0028, 0x80ff,	/*CC lut r 128 255*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000,
};

static unsigned short tune_movie_ui_sharp_tweak[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x00a0,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x00e1, 0xd6ac,	/*SCR RrCr*/
	0x00e2, 0x32ff,	/*SCR RgCg*/
	0x00e3, 0x2ef0,	/*SCR RbCb*/
	0x00e4, 0xa5fa,	/*SCR GrMr*/
	0x00e5, 0xff4d,	/*SCR GgMg*/
	0x00e6, 0x59ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00fb,	/*SCR BgYg*/
	0x00e9, 0xff61,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00f8,	/*SCR KgWg*/
	0x00ec, 0x00f1,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r   0*/
	0x0021, 0x1090,	/*CC lut r  16 144*/
	0x0022, 0x20a0,	/*CC lut r  32 160*/
	0x0023, 0x30b0,	/*CC lut r  48 176*/
	0x0024, 0x40c0,	/*CC lut r  64 192*/
	0x0025, 0x50d0,	/*CC lut r  80 208*/
	0x0026, 0x60e0,	/*CC lut r  96 224*/
	0x0027, 0x70f0,	/*CC lut r 112 240*/
	0x0028, 0x80ff,	/*CC lut r 128 255*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000,
};

static unsigned short tune_movie_video_sharp_tweak[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x00a0,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1004,	/*CS weight grayTH*/
	0x00e1, 0xd6ac,	/*SCR RrCr*/
	0x00e2, 0x32ff,	/*SCR RgCg*/
	0x00e3, 0x2ef0,	/*SCR RbCb*/
	0x00e4, 0xa5fa,	/*SCR GrMr*/
	0x00e5, 0xff4d,	/*SCR GgMg*/
	0x00e6, 0x59ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00fb,	/*SCR BgYg*/
	0x00e9, 0xff61,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00f8,	/*SCR KgWg*/
	0x00ec, 0x00f1,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r   0*/
	0x0021, 0x1090,	/*CC lut r  16 144*/
	0x0022, 0x20a0,	/*CC lut r  32 160*/
	0x0023, 0x30b0,	/*CC lut r  48 176*/
	0x0024, 0x40c0,	/*CC lut r  64 192*/
	0x0025, 0x50d0,	/*CC lut r  80 208*/
	0x0026, 0x60e0,	/*CC lut r  96 224*/
	0x0027, 0x70f0,	/*CC lut r 112 240*/
	0x0028, 0x80ff,	/*CC lut r 128 255*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000,
};

static unsigned short tune_movie_vt_sharp_tweak[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x00aa,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0005,	/*FA cs1 | de8 dnr4 hdr2 fa1*/
	0x0039, 0x0080,	/*FA dnrWeight*/
	0x0080, 0x0fff,	/*DNR dirTh*/
	0x0081, 0x19ff,	/*DNR dirnumTh decon7Th*/
	0x0082, 0xff16,	/*DNR decon5Th maskTh*/
	0x0083, 0x0000,	/*DNR blTh*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1204,	/*CS weight grayTH*/
	0x00e1, 0xd6ac,	/*SCR RrCr*/
	0x00e2, 0x32ff,	/*SCR RgCg*/
	0x00e3, 0x2ef0,	/*SCR RbCb*/
	0x00e4, 0xa5fa,	/*SCR GrMr*/
	0x00e5, 0xff4d,	/*SCR GgMg*/
	0x00e6, 0x59ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00fb,	/*SCR BgYg*/
	0x00e9, 0xff61,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00f8,	/*SCR KgWg*/
	0x00ec, 0x00f1,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r   0*/
	0x0021, 0x1090,	/*CC lut r  16 144*/
	0x0022, 0x20a0,	/*CC lut r  32 160*/
	0x0023, 0x30b0,	/*CC lut r  48 176*/
	0x0024, 0x40c0,	/*CC lut r  64 192*/
	0x0025, 0x50d0,	/*CC lut r  80 208*/
	0x0026, 0x60e0,	/*CC lut r  96 224*/
	0x0027, 0x70f0,	/*CC lut r 112 240*/
	0x0028, 0x80ff,	/*CC lut r 128 255*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000,
};

static unsigned short tune_standard_gallery_sharp_tweak[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x0088,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x0090, 0x0080,	/*DE egth*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1204,	/*CS weight grayTH*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r   0*/
	0x0021, 0x1090,	/*CC lut r  16 144*/
	0x0022, 0x20a0,	/*CC lut r  32 160*/
	0x0023, 0x30b0,	/*CC lut r  48 176*/
	0x0024, 0x40c0,	/*CC lut r  64 192*/
	0x0025, 0x50d0,	/*CC lut r  80 208*/
	0x0026, 0x60e0,	/*CC lut r  96 224*/
	0x0027, 0x70f0,	/*CC lut r 112 240*/
	0x0028, 0x80ff,	/*CC lut r 128 255*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000,
};

static unsigned short tune_standard_ui_sharp_tweak[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x0088,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1604,	/*CS weight grayTH*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r   0*/
	0x0021, 0x1090,	/*CC lut r  16 144*/
	0x0022, 0x20a0,	/*CC lut r  32 160*/
	0x0023, 0x30b0,	/*CC lut r  48 176*/
	0x0024, 0x40c0,	/*CC lut r  64 192*/
	0x0025, 0x50d0,	/*CC lut r  80 208*/
	0x0026, 0x60e0,	/*CC lut r  96 224*/
	0x0027, 0x70f0,	/*CC lut r 112 240*/
	0x0028, 0x80ff,	/*CC lut r 128 255*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000,
};

static unsigned short tune_standard_video_sharp_tweak[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x0088,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x0092, 0x0060,	/*DE pe*/
	0x0093, 0x0060,	/*DE pf*/
	0x0094, 0x0060,	/*DE pb*/
	0x0095, 0x0060,	/*DE ne*/
	0x0096, 0x0060,	/*DE nf*/
	0x0097, 0x0060,	/*DE nb*/
	0x0098, 0x1000,	/*DE max ratio*/
	0x0099, 0x0100,	/*DE min ratio*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1204,	/*CS weight grayTH*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00ff,	/*SCR KgWg*/
	0x00ec, 0x00ff,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r   0*/
	0x0021, 0x1090,	/*CC lut r  16 144*/
	0x0022, 0x20a0,	/*CC lut r  32 160*/
	0x0023, 0x30b0,	/*CC lut r  48 176*/
	0x0024, 0x40c0,	/*CC lut r  64 192*/
	0x0025, 0x50d0,	/*CC lut r  80 208*/
	0x0026, 0x60e0,	/*CC lut r  96 224*/
	0x0027, 0x70f0,	/*CC lut r 112 240*/
	0x0028, 0x80ff,	/*CC lut r 128 255*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000,
};

static unsigned short tune_standard_vt_sharp_tweak[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x008a,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0005,	/*FA cs1 | de8 dnr4 hdr2 fa1*/
	0x0039, 0x0080,	/*FA dnrWeight*/
	0x0080, 0x0fff,	/*DNR dirTh*/
	0x0081, 0x19ff,	/*DNR dirnumTh decon7Th*/
	0x0082, 0xff16,	/*DNR decon5Th maskTh*/
	0x0083, 0x0000,	/*DNR blTh*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1804,	/*CS weight grayTH*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r   0*/
	0x0021, 0x1090,	/*CC lut r  16 144*/
	0x0022, 0x20a0,	/*CC lut r  32 160*/
	0x0023, 0x30b0,	/*CC lut r  48 176*/
	0x0024, 0x40c0,	/*CC lut r  64 192*/
	0x0025, 0x50d0,	/*CC lut r  80 208*/
	0x0026, 0x60e0,	/*CC lut r  96 224*/
	0x0027, 0x70f0,	/*CC lut r 112 240*/
	0x0028, 0x80ff,	/*CC lut r 128 255*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000,
};

static unsigned short tune_natural_gallery_sharp_tweak[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x00a8,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x0090, 0x0080,	/*DE egth*/
	0x0099, 0x0100,	/*DE min ratio*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1804,	/*CS weight grayTH*/
	0x00e1, 0xd6ac,	/*SCR RrCr*/
	0x00e2, 0x32ff,	/*SCR RgCg*/
	0x00e3, 0x2ef0,	/*SCR RbCb*/
	0x00e4, 0xa5fa,	/*SCR GrMr*/
	0x00e5, 0xff4d,	/*SCR GgMg*/
	0x00e6, 0x59ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00fb,	/*SCR BgYg*/
	0x00e9, 0xff61,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00fa,	/*SCR KgWg*/
	0x00ec, 0x00f8,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r   0*/
	0x0021, 0x1090,	/*CC lut r  16 144*/
	0x0022, 0x20a0,	/*CC lut r  32 160*/
	0x0023, 0x30b0,	/*CC lut r  48 176*/
	0x0024, 0x40c0,	/*CC lut r  64 192*/
	0x0025, 0x50d0,	/*CC lut r  80 208*/
	0x0026, 0x60e0,	/*CC lut r  96 224*/
	0x0027, 0x70f0,	/*CC lut r 112 240*/
	0x0028, 0x80ff,	/*CC lut r 128 255*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000,
};

static unsigned short tune_natural_ui_sharp_tweak[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x00a8,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1804,	/*CS weight grayTH*/
	0x00e1, 0xd6ac,	/*SCR RrCr*/
	0x00e2, 0x32ff,	/*SCR RgCg*/
	0x00e3, 0x2ef0,	/*SCR RbCb*/
	0x00e4, 0xa5fa,	/*SCR GrMr*/
	0x00e5, 0xff4d,	/*SCR GgMg*/
	0x00e6, 0x59ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00fb,	/*SCR BgYg*/
	0x00e9, 0xff61,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00fa,	/*SCR KgWg*/
	0x00ec, 0x00f8,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r   0*/
	0x0021, 0x1090,	/*CC lut r  16 144*/
	0x0022, 0x20a0,	/*CC lut r  32 160*/
	0x0023, 0x30b0,	/*CC lut r  48 176*/
	0x0024, 0x40c0,	/*CC lut r  64 192*/
	0x0025, 0x50d0,	/*CC lut r  80 208*/
	0x0026, 0x60e0,	/*CC lut r  96 224*/
	0x0027, 0x70f0,	/*CC lut r 112 240*/
	0x0028, 0x80ff,	/*CC lut r 128 255*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000,
};

static unsigned short tune_natural_video_sharp_tweak[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x00a8,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x0090, 0x0080,	/*DE egth*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1804,	/*CS weight grayTH*/
	0x00e1, 0xd6ac,	/*SCR RrCr*/
	0x00e2, 0x32ff,	/*SCR RgCg*/
	0x00e3, 0x2ef0,	/*SCR RbCb*/
	0x00e4, 0xa5fa,	/*SCR GrMr*/
	0x00e5, 0xff4d,	/*SCR GgMg*/
	0x00e6, 0x59ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00fb,	/*SCR BgYg*/
	0x00e9, 0xff61,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00fa,	/*SCR KgWg*/
	0x00ec, 0x00f8,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r   0*/
	0x0021, 0x1090,	/*CC lut r  16 144*/
	0x0022, 0x20a0,	/*CC lut r  32 160*/
	0x0023, 0x30b0,	/*CC lut r  48 176*/
	0x0024, 0x40c0,	/*CC lut r  64 192*/
	0x0025, 0x50d0,	/*CC lut r  80 208*/
	0x0026, 0x60e0,	/*CC lut r  96 224*/
	0x0027, 0x70f0,	/*CC lut r 112 240*/
	0x0028, 0x80ff,	/*CC lut r 128 255*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000,
};

static unsigned short tune_natural_vt_sharp_tweak[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x00aa,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0005,	/*FA cs1 | de8 dnr4 hdr2 fa1*/
	0x0039, 0x0080,	/*FA dnrWeight*/
	0x0080, 0x0fff,	/*DNR dirTh*/
	0x0081, 0x19ff,	/*DNR dirnumTh decon7Th*/
	0x0082, 0xff16,	/*DNR decon5Th maskTh*/
	0x0083, 0x0000,	/*DNR blTh*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1804,	/*CS weight grayTH*/
	0x00e1, 0xd6ac,	/*SCR RrCr*/
	0x00e2, 0x32ff,	/*SCR RgCg*/
	0x00e3, 0x2ef0,	/*SCR RbCb*/
	0x00e4, 0xa5fa,	/*SCR GrMr*/
	0x00e5, 0xff4d,	/*SCR GgMg*/
	0x00e6, 0x59ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00fb,	/*SCR BgYg*/
	0x00e9, 0xff61,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00fa,	/*SCR KgWg*/
	0x00ec, 0x00f8,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r   0*/
	0x0021, 0x1090,	/*CC lut r  16 144*/
	0x0022, 0x20a0,	/*CC lut r  32 160*/
	0x0023, 0x30b0,	/*CC lut r  48 176*/
	0x0024, 0x40c0,	/*CC lut r  64 192*/
	0x0025, 0x50d0,	/*CC lut r  80 208*/
	0x0026, 0x60e0,	/*CC lut r  96 224*/
	0x0027, 0x70f0,	/*CC lut r 112 240*/
	0x0028, 0x80ff,	/*CC lut r 128 255*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000,
};

static unsigned short tune_camera_sharp_tweak[] = {
	0x0000, 0x0000, /*BANK 0*/
	0x0008, 0x002c, /*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000, /*FA cs1 de8 hdr2 fa1*/
	0x0092, 0x0060, /*DE pe*/
	0x0093, 0x0060, /*DE pf*/
	0x0094, 0x0060, /*DE pb*/
	0x0095, 0x0060, /*DE ne*/
	0x0096, 0x0060, /*DE nf*/
	0x0097, 0x0060, /*DE nb*/
	0x0098, 0x1000, /*DE max ratio*/
	0x0099, 0x0100, /*DE min ratio*/
	0x00b0, 0x1010, /*CS hg ry*/
	0x00b1, 0x1010, /*CS hg gc*/
	0x00b2, 0x1010, /*CS hg bm*/
	0x00b3, 0x1204, /*CS weight grayTH*/
	0x00e1, 0xff00, /*SCR RrCr*/
	0x00e2, 0x00ff, /*SCR RgCg*/
	0x00e3, 0x00ff, /*SCR RbCb*/
	0x00e4, 0x00ff, /*SCR GrMr*/
	0x00e5, 0xff00, /*SCR GgMg*/
	0x00e6, 0x00ff, /*SCR GbMb*/
	0x00e7, 0x00ff, /*SCR BrYr*/
	0x00e8, 0x00ff, /*SCR BgYg*/
	0x00e9, 0xff00, /*SCR BbYb*/
	0x00ea, 0x00ff, /*SCR KrWr*/
	0x00eb, 0x00ff, /*SCR KgWg*/
	0x00ec, 0x00ff, /*SCR KbWb*/
	0x00ff, 0x0000, /*Mask Release*/
	END_SEQ, 0x0000
};


struct mdnie_tuning_info tuning_table_sharp_tweak[CABC_MAX][MODE_MAX][SCENARIO_MAX] = {
	{
		{
			{"dynamic_ui",		tune_dynamic_ui_sharp_tweak},
			{"dynamic_video",	tune_dynamic_video_sharp_tweak},
			{"dynamic_video",	tune_dynamic_video_sharp_tweak},
			{"dynamic_video",	tune_dynamic_video_sharp_tweak},
			{"camera",		tune_camera_sharp_tweak},
			{"dynamic_ui",		tune_dynamic_ui_sharp_tweak},
			{"dynamic_gallery",	tune_dynamic_gallery_sharp_tweak},
			{"dynamic_vt",		tune_dynamic_vt_sharp_tweak},
			{"dynamic_browser",	tune_dynamic_ui_sharp_tweak},
			{"dynamic_ebook",	tune_dynamic_ui_sharp_tweak},
			{"email",		tune_dynamic_ui_sharp_tweak}
		}, {
			{"standard_ui",		tune_standard_ui_sharp_tweak},
			{"standard_video",	tune_standard_video_sharp_tweak},
			{"standard_video",	tune_standard_video_sharp_tweak},
			{"standard_video",	tune_standard_video_sharp_tweak},
			{"camera",		tune_camera_sharp_tweak},
			{"standard_ui",		tune_standard_ui_sharp_tweak},
			{"standard_gallery",	tune_standard_gallery_sharp_tweak},
			{"standard_vt",		tune_standard_vt_sharp_tweak},
			{"standard_browser",	tune_standard_ui_sharp_tweak},
			{"standard_ebook",	tune_standard_ui_sharp_tweak},
			{"email",		tune_standard_ui_sharp_tweak}
		}, {
			{"natural_ui",		tune_natural_ui_sharp_tweak},
			{"natural_video",	tune_natural_video_sharp_tweak},
			{"natural_video",	tune_natural_video_sharp_tweak},
			{"natural_video",	tune_natural_video_sharp_tweak},
			{"camera",		tune_camera_sharp_tweak},
			{"natural_ui",		tune_natural_ui_sharp_tweak},
			{"natural_gallery",	tune_natural_gallery_sharp_tweak},
			{"natural_vt",		tune_natural_vt_sharp_tweak},
			{"natural_browser",	tune_natural_ui_sharp_tweak},
			{"natural_ebook",	tune_natural_ui_sharp_tweak},
			{"email",		tune_natural_ui_sharp_tweak}
		}, {
			{"movie_ui",		tune_movie_ui_sharp_tweak},
			{"movie_video",		tune_movie_video_sharp_tweak},
			{"movie_video",		tune_movie_video_sharp_tweak},
			{"movie_video",		tune_movie_video_sharp_tweak},
			{"camera",		tune_camera_sharp_tweak},
			{"movie_ui",		tune_movie_ui_sharp_tweak},
			{"movie_gallery",	tune_movie_gallery_sharp_tweak},
			{"movie_vt",		tune_movie_vt_sharp_tweak},
			{"movie_browser",	tune_movie_ui_sharp_tweak},
			{"movie_ebook",		tune_movie_ui_sharp_tweak},
			{"email",		tune_movie_ui_sharp_tweak}
		}, { 
			// with "old" mdnie, auto behaves like standard mode
			{"auto_ui",		tune_standard_ui_sharp_tweak},
			{"auto_video",		tune_standard_video_sharp_tweak},
			{"auto_video",		tune_standard_video_sharp_tweak},
			{"auto_video",		tune_standard_video_sharp_tweak},
			{"auto_camera",		tune_camera_sharp_tweak},
			{"auto_ui",		tune_standard_ui_sharp_tweak},
			{"auto_gallery",	tune_standard_gallery_sharp_tweak},
			{"auto_vt",		tune_standard_vt_sharp_tweak},
			{"auto_browser",	tune_standard_ui_sharp_tweak},
			{"auto_ebook",		tune_standard_ui_sharp_tweak},
			{"email",		tune_standard_ui_sharp_tweak}
		}
	}
};

