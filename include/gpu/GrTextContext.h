/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextContext_DEFINED
#define GrTextContext_DEFINED

#include "GrContext.h"
#include "GrGlyph.h"
#include "GrPaint.h"

class GrContext;
class GrDrawTarget;
class GrFontScaler;

/*
 * This class wraps the state for a single text render
 */
class GrTextContext {
public:
    virtual void drawPackedGlyph(GrGlyph::PackedID, GrFixed left, GrFixed top,
                                 GrFontScaler*) = 0;

protected:
    GrTextContext(GrContext*, const GrPaint&);
    virtual ~GrTextContext() {}

    GrPaint                fPaint;
    GrContext*             fContext;
    GrDrawTarget*          fDrawTarget;

    SkIRect                fClipRect;

private:
};

#endif
