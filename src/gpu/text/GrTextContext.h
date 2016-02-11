/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextContext_DEFINED
#define GrTextContext_DEFINED

#include "GrClip.h"
#include "GrGlyph.h"
#include "GrPaint.h"
#include "SkSurfaceProps.h"
#include "SkPostConfig.h"

class GrClip;
class GrContext;
class GrDrawContext;
class GrFontScaler;
class SkDrawFilter;
class SkTextBlob;

/*
 * This class wraps the state for a single text render
 */
class GrTextContext {
public:
    virtual ~GrTextContext() {}

    virtual void drawText(GrDrawContext* dc,
                          const GrClip&,  const GrPaint&, const SkPaint&,
                          const SkMatrix& viewMatrix,
                          const SkSurfaceProps& props, const char text[], size_t byteLength,
                          SkScalar x, SkScalar y, const SkIRect& clipBounds) = 0;
    virtual void drawPosText(GrDrawContext* dc,
                             const GrClip&, const GrPaint&, const SkPaint&,
                             const SkMatrix& viewMatrix,
                             const SkSurfaceProps& props,
                             const char text[], size_t byteLength,
                             const SkScalar pos[], int scalarsPerPosition,
                             const SkPoint& offset, const SkIRect& clipBounds) = 0;
    virtual void drawTextBlob(GrDrawContext* dc, const GrClip&,
                              const SkPaint&, const SkMatrix& viewMatrix,
                              const SkSurfaceProps& props, const SkTextBlob*,
                              SkScalar x, SkScalar y,
                              SkDrawFilter*, const SkIRect& clipBounds) = 0;

    static bool ShouldDisableLCD(const SkPaint& paint);

protected:
    GrContext*                     fContext;

    GrTextContext(GrContext*);

    static GrFontScaler* GetGrFontScaler(SkGlyphCache* cache);
    static uint32_t FilterTextFlags(const SkSurfaceProps& surfaceProps, const SkPaint& paint);

    friend class GrAtlasTextBatch;
    friend class GrAtlasTextBlob; // for FilterTextFlags
    friend class GrTextUtils; // for some static functions
};

#endif
