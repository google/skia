/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAtlasTextTarget_DEFINED
#define SkAtlasTextTarget_DEFINED

#include <memory>
#include "SkScalar.h"
#include "SkRefCnt.h"
#include "text/GrAtlasTextContext.h"

class SkAtlasTextContext;

class SkAtlasTextOp;

class SkAtlasTextTarget {
public:
    ~SkAtlasTextTarget();

    /**
     * Creates a text drawing target. ‘handle’ is used to identify this rendering
     * surface to SkAtlasTextRenderer::drawSDFGlyphs.
     */
    static std::unique_ptr<SkAtlasTextTarget> Make(sk_sp<SkAtlasTextContext>, int width, int height,
                                                   void* handle);

    void drawText(const void* text, size_t byteLength, SkScalar x, SkScalar y);

    /** Ensures all draws are issued to SkAtlasTextRenderer. */
    void flush();

    int width() const { return fWidth; }
    int height() const { return fHeight; }

private:
    SkAtlasTextTarget() = delete;
    SkAtlasTextTarget(const SkAtlasTextContext&) = delete;
    SkAtlasTextTarget& operator=(const SkAtlasTextContext&) = delete;

    SkAtlasTextTarget(sk_sp<SkAtlasTextContext>, int width, int height, void* handle);

    void addOp(std::unique_ptr<GrAtlasTextOp>);

    SkTArray<std::unique_ptr<GrAtlasTextOp>, true> fOps;
    sk_sp<SkAtlasTextContext> fContext;
    int fWidth;
    int fHeight;
    void* fHandle;
};

#endif
