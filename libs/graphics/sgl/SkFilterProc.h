#ifndef SkFilter_DEFINED
#define SkFilter_DEFINED

#include "SkMath.h"

typedef unsigned (*SkFilterProc)(unsigned x00, unsigned x01, unsigned x10, unsigned x11);

const SkFilterProc* SkGetBilinearFilterProcTable();

inline SkFilterProc SkGetBilinearFilterProc(const SkFilterProc* table, SkFixed x, SkFixed y)
{
	SkASSERT(table);

	// convert to dot 2
	x = (unsigned)(x << 16) >> 30;
	y = (unsigned)(y << 16) >> 30;
	return table[(y << 2) | x];
}

#endif


