/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkXPS_DEFINED
#define SkXPS_DEFINED

#include <memory>

#include "SkDocument.h"
#include "SkCanvas.h"
#include "SkScalar.h"
#include "SkTextBlob.h"

class SkBitmap;
class SkMatrix;
class SkPaint;
class SkPath;
class SkWStream;
struct SkPoint;
struct IXpsOMObjectFactory;

class SkXPS {
public:
    SkXPS(SkWStream*, IXpsOMObjectFactory*, SkScalar dpi);
    ~SkXPS();
    void newPage(SkSize);
    void endPortfolio();
    void saveLayer(const SkCanvas::SaveLayerRec& rec);
    void restoreLayer();

    void drawText(const SkMatrix& ctm, const SkPath& clip,
                  const void* text, size_t textBytes,
                  SkTextBlob::GlyphPositioning positioning,
                  SkPoint origin, const SkScalar* pos, const SkPaint& paint);
    void drawPoints(const SkMatrix& ctm, const SkPath& clip,
                    SkCanvas::PointMode, size_t, const SkPoint[], const SkPaint&);
    void drawPath(const SkMatrix& ctm, const SkPath& clip,
                  const SkPath& path, const SkPaint& paint);
    void drawBitmap(const SkMatrix& ctm, const SkPath& clip,
                    const SkBitmap&, SkPoint, const SkPaint*);

private:
    struct Impl;
    std::unique_ptr<Impl> fImpl;

    SkXPS(const SkXPS&) = delete;
    SkXPS(SkXPS&&) = delete;
    SkXPS& operator=(const SkXPS&) = delete;
    SkXPS& operator=(SkXPS&&) = delete;

    void endPage();
};

#endif  // SkXPS_DEFINED

