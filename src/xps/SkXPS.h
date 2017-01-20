/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkXPS_DEFINED
#define SkXPS_DEFINED

#include "SkLeanWindows.h"

#ifdef SK_BUILD_FOR_WIN

#include <XpsObjectModel.h>

#include "SkCanvas.h"
#include "SkTScopedComPtr.h"
#include "SkTextBlob.h"
#include "SkIStream.h"

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
    void drawPath(const SkMatrix& ctm, const SkPath& clip,
                  const SkPath& path, const SkPaint& paint);
    void drawBitmap(const SkMatrix& ctm, const SkPath& clip,
                    const SkBitmap&, SkPoint, const SkPaint*);

    struct Layer;
    struct TypefaceUse;

private:
    friend struct Layer;
    SkTArray<Layer, true> fLayers;
    SkTArray<TypefaceUse, true> fTypefaces;
    SkTScopedComPtr<IXpsOMObjectFactory> fXpsFactory;
    SkTScopedComPtr<IStream> fOutputStream;
    SkTScopedComPtr<IXpsOMPackageWriter> fPackageWriter;
    SkSize fCurrentPageSize;
    unsigned fCurrentPage = 0;
    SkScalar fDpi;
    #ifdef SK_XPS_USE_DETERMINISTIC_IDS
    uint32_t fNextId = 0;
    #endif

    SkXPS(const SkXPS&) = delete;
    SkXPS(SkXPS&&) = delete;
    SkXPS& operator=(const SkXPS&) = delete;
    SkXPS& operator=(SkXPS&&) = delete;

    void endPage();
    void createId(wchar_t* buffer, size_t bufferSize, wchar_t sep = '-');
};
#endif  // SK_BUILD_FOR_WINDOWS
#endif  // SkXPS_DEFINED

