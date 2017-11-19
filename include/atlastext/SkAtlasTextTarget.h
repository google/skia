/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAtlasTextTarget_DEFINED
#define SkAtlasTextTarget_DEFINED

#include <memory>
#include "SkRefCnt.h"
#include "SkScalar.h"

class SkAtlasTextContext;
class SkAtlasTextFont;

/** Represents a client-created renderable surface and is used to draw text into the surface. */
class SK_API SkAtlasTextTarget {
public:
    virtual ~SkAtlasTextTarget();

    /**
     * Creates a text drawing target. ‘handle’ is used to identify this rendering surface when
     * draws are flushed to the SkAtlasTextContext's SkAtlasTextRenderer.
     */
    static std::unique_ptr<SkAtlasTextTarget> Make(sk_sp<SkAtlasTextContext>, int width, int height,
                                                   void* handle);

    /**
     * Enqueues a text draw in the target. The meaning of 'color' here is interpreted by the
     * client's SkAtlasTextRenderer when it actually renders the text.
     */
    virtual void drawText(const void* text, size_t byteLength, SkScalar x, SkScalar y,
                          uint32_t color, const SkAtlasTextFont& font) = 0;

    /** Issues all queued text draws to SkAtlasTextRenderer. */
    virtual void flush() = 0;

    int width() const { return fWidth; }
    int height() const { return fHeight; }

    void* handle() const { return fHandle; }

    SkAtlasTextContext* context() const { return fContext.get(); }

protected:
    SkAtlasTextTarget(sk_sp<SkAtlasTextContext>, int width, int height, void* handle);

    void* const fHandle;
    const sk_sp<SkAtlasTextContext> fContext;
    const int fWidth;
    const int fHeight;

private:
    SkAtlasTextTarget() = delete;
    SkAtlasTextTarget(const SkAtlasTextContext&) = delete;
    SkAtlasTextTarget& operator=(const SkAtlasTextContext&) = delete;
};

#endif
