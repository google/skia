/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextTarget_DEFINED
#define GrTextTarget_DEFINED

#include "include/core/SkPaint.h"
#include "src/gpu/GrColorSpaceInfo.h"

class GrAtlasTextOp;
class GrClip;
class GrPaint;
class GrRecordingContext;
class GrShape;
class SkGlyphRunListPainter;
class SkMatrix;
struct SkIRect;

class GrTextTarget {
public:
    virtual ~GrTextTarget() = default;

    int width() const { return fWidth; }

    int height() const { return fHeight; }

    const GrColorSpaceInfo& colorSpaceInfo() const { return fColorSpaceInfo; }

    virtual void addDrawOp(const GrClip&, std::unique_ptr<GrAtlasTextOp> op) = 0;

    virtual void drawShape(const GrClip&, const SkPaint&,
                           const SkMatrix& viewMatrix, const GrShape&) = 0;

    virtual void makeGrPaint(GrMaskFormat, const SkPaint&, const SkMatrix& viewMatrix,
                             GrPaint*) = 0;

    virtual GrRecordingContext* getContext() = 0;

    virtual SkGlyphRunListPainter* glyphPainter() = 0;

protected:
    GrTextTarget(int width, int height, const GrColorSpaceInfo& colorSpaceInfo)
            : fWidth(width), fHeight(height), fColorSpaceInfo(colorSpaceInfo) {}

private:
    int fWidth;
    int fHeight;
    const GrColorSpaceInfo& fColorSpaceInfo;
};
#endif  // GrTextTarget_DEFINED
