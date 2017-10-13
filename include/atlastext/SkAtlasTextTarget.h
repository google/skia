/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAtlasTextTarget_DEFINED
#define SkAtlasTextTarget_DEFINED

#include "SkScalar.h"
#include "SkRefCnt.h"
#include "SkMakeUnique.h"

class SkAtlasTextContext;

class SkAtlasTextTarget {
public:
    /**
     * Creates a text drawing target. ‘handle’ is used to identify this rendering
     * surface to SkAtlasTextRenderer::drawSDFGlyphs.
     */
    static std::unique_ptr<SkAtlasTextTarget> Make(sk_sp<SkAtlasTextContext> context,
                                                   int width, int height, void* handle) {
        return skstd::make_unique<SkAtlasTextTarget>(std::move(context), width, height, handle);
    }

    void drawText(const void* text, size_t byteLength, SkScalar x, SkScalar y);

    /** Ensures all draws are issued to SkAtlasTextRenderer. */
    void flush();

private:
    SkAtlasTextTarget() = delete;
    SkAtlasTextTarget(const SkAtlasTextContext&) = delete;
    SkAtlasTextTarget& operator=(const SkAtlasTextContext&) = delete;

    SkAtlasTextTarget(sk_sp<SkAtlasTextContext> context, int width, int height, void* handle)
            : fContext(std::move(context)), fWidth(width), fHeight(height), fHandle(handle) {}

    sk_sp<SkAtlasTextContext> fContext;
    int fWidth;
    int fHeight;
    void* fHandle;
};

#endif
