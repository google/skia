/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextUtils_DEFINED
#define GrTextUtils_DEFINED

#include "GrColor.h"
#include "GrColorSpaceInfo.h"
#include "SkColorFilter.h"
#include "SkPaint.h"
#include "SkScalar.h"
#include "SkTextToPathIter.h"
#include "SkTLazy.h"

class GrTextBlob;
class GrAtlasTextOp;
class GrTextStrike;
class GrClip;
class GrColorSpaceXform;
class GrContext;
class GrGlyphCache;
class GrPaint;
class GrShaderCaps;
class SkColorSpace;
class SkGlyph;
class SkGlyphRunListDrawer;
class SkMatrix;
struct SkIRect;
struct SkPoint;
class SkGlyphCache;
class SkTextBlobRunIterator;
class SkSurfaceProps;

/**
 * A class to house a bunch of common text utilities.  This class should *ONLY* have static
 * functions.  It is not a namespace only because we wish to friend SkPaint
 */
class GrTextUtils {
public:
    class Target {
    public:
        virtual ~Target() = default;

        int width() const { return fWidth; }

        int height() const { return fHeight; }

        const GrColorSpaceInfo& colorSpaceInfo() const { return fColorSpaceInfo; }

        virtual void addDrawOp(const GrClip&, std::unique_ptr<GrAtlasTextOp> op) = 0;

        virtual void drawPath(const GrClip&, const SkPath&, const SkPaint&,
                              const SkMatrix& viewMatrix, const SkMatrix* pathMatrix,
                              const SkIRect& clipBounds) = 0;

        virtual void makeGrPaint(GrMaskFormat, const SkPaint&, const SkMatrix& viewMatrix,
                                 GrPaint*) = 0;

        virtual GrContext* getContext() = 0;

        virtual SkGlyphRunListDrawer* glyphDrawer() = 0;

    protected:
        Target(int width, int height, const GrColorSpaceInfo& colorSpaceInfo)
                : fWidth(width), fHeight(height), fColorSpaceInfo(colorSpaceInfo) {}

    private:
        int fWidth;
        int fHeight;
        const GrColorSpaceInfo& fColorSpaceInfo;
    };
};
#endif
