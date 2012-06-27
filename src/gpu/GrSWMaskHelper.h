/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSWMaskHelper_DEFINED
#define GrSWMaskHelper_DEFINED

#include "GrColor.h"
#include "GrMatrix.h"
#include "GrNoncopyable.h"
#include "SkBitmap.h"
#include "SkDraw.h"
#include "SkRasterClip.h"
#include "SkRegion.h"

class GrAutoScratchTexture;
class GrContext;
class GrTexture;
class SkPath;

/**
 * The GrSWMaskHelper helps generate clip masks using the software rendering
 * path.
 */
class GrSWMaskHelper : public GrNoncopyable {
public:
    GrSWMaskHelper(GrContext* context) 
    : fContext(context) {
    }

    void draw(const GrRect& clientRect, SkRegion::Op op, 
              bool antiAlias, GrColor color);

    void draw(const SkPath& clientPath, SkRegion::Op op, 
              GrPathFill fill, bool antiAlias, GrColor color);

    bool init(const GrIRect& pathDevBounds, 
              const GrPoint* translate,
              bool useMatrix);

    bool getTexture(GrAutoScratchTexture* tex);

    void toTexture(GrTexture* texture, bool clearToWhite);

    void clear(GrColor color) {
        fBM.eraseColor(color);
    }

protected:
private:
    GrContext*      fContext;
    GrMatrix        fMatrix;
    SkBitmap        fBM;
    SkDraw          fDraw;
    SkRasterClip    fRasterClip;

    typedef GrNoncopyable INHERITED;
};

#endif GrSWMaskHelper_DEFINED
