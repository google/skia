/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAtlasTextTarget_DEFINED
#define SkAtlasTextTarget_DEFINED

#include "include/core/SkDeque.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"

#include <memory>

class SkAtlasTextContext;
class SkAtlasTextFont;
class SkMatrix;
struct SkPoint;

/** Represents a client-created renderable surface and is used to draw text into the surface. */
class SK_API SkAtlasTextTarget {
public:
    virtual ~SkAtlasTextTarget();

    /**
     * Creates a text drawing target. ‘handle’ is used to identify this rendering surface when
     * draws are flushed to the SkAtlasTextContext's SkAtlasTextRenderer.
     */
    static std::unique_ptr<SkAtlasTextTarget> Make(sk_sp<SkAtlasTextContext>,
                                                   int width,
                                                   int height,
                                                   void* handle);

    /**
     * Enqueues a text draw in the target. The caller provides an array of glyphs and their
     * positions. The meaning of 'color' here is interpreted by the client's SkAtlasTextRenderer
     * when it actually renders the text.
     */
    virtual void drawText(const SkGlyphID[], const SkPoint[], int glyphCnt, uint32_t color,
                          const SkAtlasTextFont&) = 0;

    /** Issues all queued text draws to SkAtlasTextRenderer. */
    virtual void flush() = 0;

    int width() const { return fWidth; }
    int height() const { return fHeight; }

    void* handle() const { return fHandle; }

    SkAtlasTextContext* context() const { return fContext.get(); }

    /** Saves the current matrix in a stack. Returns the prior depth of the saved matrix stack. */
    int save();
    /** Pops the top matrix on the stack if the stack is not empty. */
    void restore();
    /**
     * Pops the matrix stack until the stack depth is count. Does nothing if the depth is already
     * less than count.
     */
    void restoreToCount(int count);

    /** Pre-translates the current CTM. */
    void translate(SkScalar dx, SkScalar dy);
    /** Pre-scales the current CTM. */
    void scale(SkScalar sx, SkScalar sy);
    /** Pre-rotates the current CTM about the origin. */
    void rotate(SkScalar degrees);
    /** Pre-rotates the current CTM about the (px, py). */
    void rotate(SkScalar degrees, SkScalar px, SkScalar py);
    /** Pre-skews the current CTM. */
    void skew(SkScalar sx, SkScalar sy);
    /** Pre-concats the current CTM. */
    void concat(const SkMatrix& matrix);

protected:
    SkAtlasTextTarget(sk_sp<SkAtlasTextContext>, int width, int height, void* handle);

    const SkMatrix& ctm() const { return *static_cast<const SkMatrix*>(fMatrixStack.back()); }

    void* const fHandle;
    const sk_sp<SkAtlasTextContext> fContext;
    const int fWidth;
    const int fHeight;

private:
    SkDeque fMatrixStack;
    int fSaveCnt;

    SkMatrix* accessCTM() const {
        return static_cast<SkMatrix*>(const_cast<void*>(fMatrixStack.back()));
    }

    SkAtlasTextTarget() = delete;
    SkAtlasTextTarget(const SkAtlasTextContext&) = delete;
    SkAtlasTextTarget& operator=(const SkAtlasTextContext&) = delete;
};

#endif
