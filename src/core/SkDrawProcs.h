
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkDrawProcs_DEFINED
#define SkDrawProcs_DEFINED

#include "SkDraw.h"

class SkAAClip;
class SkBlitter;

struct SkDraw1Glyph {
    const SkDraw*   fDraw;
	SkBounder*		fBounder;
	const SkRegion*	fClip;
	const SkAAClip*	fAAClip;
	SkBlitter*		fBlitter;
	SkGlyphCache*	fCache;
	SkIRect			fClipBounds;
	
    // The fixed x,y have been pre-rounded (i.e. 1/2 has already been added),
    // so the impls need just trunc down to an int
	typedef void (*Proc)(const SkDraw1Glyph&, SkFixed x, SkFixed y, const SkGlyph&);
	
	Proc init(const SkDraw* draw, SkBlitter* blitter, SkGlyphCache* cache);
};

struct SkDrawProcs {
    SkDraw1Glyph::Proc  fD1GProc;
};

/**
 *  If the current paint is set to stroke, has a compatible xfermode, and the
 *  stroke-width when applied to the matrix is <= 1.0, then this returns true,
 *  and sets newAlpha (simulating a stroke by drawing a hairline + newAlpha).
 *  If any of these conditions are false, then this returns false and modulate
 *  is ignored.
 */
bool SkDrawTreatAsHairline(const SkPaint&, const SkMatrix&, SkAlpha* newAlpha);

#endif

