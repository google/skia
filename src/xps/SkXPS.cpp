/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkXPS.h"

#if 1 // #ifndef SK_BUILD_FOR_WIN

struct SkXPS::Impl {};

SkXPS::SkXPS(SkWStream*, SkScalar dpi) {}

SkXPS::~SkXPS() {}

void SkXPS::newPage(SkISize) {}

void SkXPS::endPortfolio() {}

void SkXPS::saveLayer(const SkCanvas::SaveLayerRec& rec) {}

void SkXPS::restoreLayer() {}

void SkXPS::drawText(const SkMatrix& ctm, const SkPath& clip,
                     const void* text, size_t textBytes,
                     SkTextBlob::GlyphPositioning positioning,
                     SkPoint origin, const SkScalar* pos, const SkPaint& paint) {}

void SkXPS::drawPoints(const SkMatrix& ctm, const SkPath& clip,
                       SkCanvas::PointMode, size_t, const SkPoint[], const SkPaint&) {}

void SkXPS::drawPath(const SkMatrix& ctm, const SkPath& clip,
                     const SkPath& path, const SkPaint& paint) {}

void SkXPS::drawBitmap(const SkMatrix& ctm, const SkPath& clip,
                       const SkBitmap&, SkPoint, const SkPaint*) {}
#endif  // SK_BUILD_FOR_WIN
