
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSoftwarePathRenderer_DEFINED
#define GrSoftwarePathRenderer_DEFINED

#include "GrPathRenderer.h"

#include "SkDraw.h"
#include "SkRasterClip.h"

class GrContext;
class GrAutoScratchTexture;

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

    void toTexture(GrTexture* texture);

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

    typedef GrPathRenderer INHERITED;
};

/**
 * This class uses the software side to render a path to an SkBitmap and
 * then uploads the result to the gpu
 */
class GrSoftwarePathRenderer : public GrPathRenderer {
public:
    GrSoftwarePathRenderer(GrContext* context) 
        : fContext(context) {
    }

    virtual bool canDrawPath(const SkPath& path,
                            GrPathFill fill,
                            const GrDrawTarget* target,
                            bool antiAlias) const SK_OVERRIDE;
protected:
    virtual bool onDrawPath(const SkPath& path,
                            GrPathFill fill,
                            const GrVec* translate,
                            GrDrawTarget* target,
                            GrDrawState::StageMask stageMask,
                            bool antiAlias) SK_OVERRIDE;
 
private:
    GrContext*     fContext;

    typedef GrPathRenderer INHERITED;
};

#endif
