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
#include "SkTLazy.h"

class GrAtlasGlyphCache;
class GrAtlasTextBlob;
class GrAtlasTextOp;
class GrAtlasTextStrike;
class GrClip;
class GrColorSpaceXform;
class GrContext;
class GrPaint;
class GrShaderCaps;
class SkColorSpace;
class SkDrawFilter;
class SkGlyph;
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

    protected:
        Target(int width, int height, const GrColorSpaceInfo& colorSpaceInfo)
                : fWidth(width), fHeight(height), fColorSpaceInfo(colorSpaceInfo) {}

    private:
        int fWidth;
        int fHeight;
        const GrColorSpaceInfo& fColorSpaceInfo;
    };

    /**
     *  This is used to wrap a SkPaint and its post-color filter color. It is also used by RunPaint
     *  (below). This keeps a pointer to the SkPaint it is initialized with and expects it to remain
     *  const. It is also used to transform to GrPaint.
     */
    class Paint {
    public:
        explicit Paint(const SkPaint* paint, const GrColorSpaceInfo* dstColorSpaceInfo)
                : fPaint(paint), fDstColorSpaceInfo(dstColorSpaceInfo) {
            this->initFilteredColor();
        }

        // These expose the paint's color run through its color filter (if any). This is only valid
        // when drawing grayscale/lcd glyph masks and not when drawing color glyphs.
        GrColor filteredPremulColor() const { return fFilteredPremulColor; }
        SkColor luminanceColor() const { return fPaint->computeLuminanceColor(); }

        const SkPaint& skPaint() const { return *fPaint; }
        operator const SkPaint&() const { return this->skPaint(); }

        // Just for RunPaint's constructor
        const GrColorSpaceInfo* dstColorSpaceInfo() const { return fDstColorSpaceInfo; }

    protected:
        void initFilteredColor();
        Paint() = default;
        const SkPaint* fPaint;
        const GrColorSpaceInfo* fDstColorSpaceInfo;
        // This is the paint's color run through its color filter, if present. This color should
        // be used except when rendering bitmap text, in which case the bitmap must be filtered in
        // the fragment shader.
        GrColor fFilteredPremulColor;
    };

    /**
     *  An extension of Paint that incorporated per-run modifications to the paint text settings and
     *  application of a draw filter. It expects its constructor arguments to remain alive and const
     *  during its lifetime.
     */
    class RunPaint : public Paint {
    public:
        RunPaint(const Paint* paint, SkDrawFilter* filter, const SkSurfaceProps& props)
                : fOriginalPaint(paint), fFilter(filter), fProps(props) {
            // Initially we represent the original paint.
            fPaint = &fOriginalPaint->skPaint();
            fDstColorSpaceInfo = fOriginalPaint->dstColorSpaceInfo();
            fFilteredPremulColor = fOriginalPaint->filteredPremulColor();
        }

        bool modifyForRun(const SkTextBlobRunIterator&);

    private:
        SkTLazy<SkPaint> fModifiedPaint;
        const Paint* fOriginalPaint;
        SkDrawFilter* fFilter;
        const SkSurfaceProps& fProps;
    };

    static uint32_t FilterTextFlags(const SkSurfaceProps& surfaceProps, const SkPaint& paint);

    static bool ShouldDisableLCD(const SkPaint& paint);

    // Functions for drawing large text either as paths or (for color emoji) as scaled glyphs
    static void DrawBigText(GrContext*, GrTextUtils::Target*, const GrClip& clip,
                               const SkPaint& paint, const SkMatrix& viewMatrix, const char text[],
                               size_t byteLength, SkScalar x, SkScalar y,
                               const SkIRect& clipBounds);

    static void DrawBigPosText(GrContext* context, GrTextUtils::Target*,
                                  const SkSurfaceProps& props, const GrClip& clip,
                                  const SkPaint& paint, const SkMatrix& viewMatrix,
                                  const char text[], size_t byteLength, const SkScalar pos[],
                                  int scalarsPerPosition, const SkPoint& offset,
                                  const SkIRect& clipBounds);
};

#endif
