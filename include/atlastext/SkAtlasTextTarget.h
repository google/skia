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
    virtual ~SkAtlasTextTarget();

    /**
     * Creates a text drawing target. ‘handle’ is used to identify this rendering
     * surface to SkAtlasTextRenderer::drawSDFGlyphs.
     */
    static std::unique_ptr<SkAtlasTextTarget> Make(sk_sp<SkAtlasTextContext>, int width, int height,
                                                   void* handle);

    virtual void drawText(const void* text, size_t byteLength, SkScalar x, SkScalar y) = 0;

    /** Ensures all draws are issued to SkAtlasTextRenderer. */
    virtual void flush() = 0;

    int width() const { return fWidth; }
    int height() const { return fHeight; }

    void* handle() const { return fHandle; }

    SkAtlasTextContext* context() const { return fContext.get(); }

private:
    SkAtlasTextTarget() = delete;
    SkAtlasTextTarget(const SkAtlasTextContext&) = delete;
    SkAtlasTextTarget& operator=(const SkAtlasTextContext&) = delete;

    SkAtlasTextTarget(sk_sp<SkAtlasTextContext>, int width, int height, void* handle);

    sk_sp<SkAtlasTextContext> fContext;

    int fWidth;
    int fHeight;
    void* fHandle;
};

#endif
