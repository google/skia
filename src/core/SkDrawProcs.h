#ifndef SkDrawProcs_DEFINED
#define SkDrawProcs_DEFINED

#include "SkDraw.h"

class SkBlitter;

struct SkDraw1Glyph {
    const SkDraw*   fDraw;
	SkBounder*		fBounder;
	const SkRegion*	fClip;
	SkBlitter*		fBlitter;
	SkGlyphCache*	fCache;
	SkIRect			fClipBounds;
	
	typedef void (*Proc)(const SkDraw1Glyph&, const SkGlyph&, int x, int y);
	
	Proc init(const SkDraw* draw, SkBlitter* blitter, SkGlyphCache* cache);
};

struct SkDrawProcs {
    SkDraw1Glyph::Proc  fD1GProc;
};

#endif

