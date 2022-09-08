/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSpriteBlitter_DEFINED
#define SkSpriteBlitter_DEFINED

#include "include/core/SkPixmap.h"
#include "include/core/SkShader.h"
#include "src/core/SkBlitter.h"

class SkPaint;

// SkSpriteBlitter specializes SkBlitter in a way to move large rectangles of pixels around.
// Because of this use, the main primitive shifts from blitH style things to the more efficient
// blitRect.
class SkSpriteBlitter : public SkBlitter {
public:
    SkSpriteBlitter(const SkPixmap& source);

    virtual bool setup(const SkPixmap& dst, int left, int top, const SkPaint&);

    // blitH, blitAntiH, blitV and blitMask should not be called on an SkSpriteBlitter.
    void blitH(int x, int y, int width) override;
    void blitAntiH(int x, int y, const SkAlpha antialias[], const int16_t runs[]) override;
    void blitV(int x, int y, int height, SkAlpha alpha) override;
    void blitMask(const SkMask&, const SkIRect& clip) override;

    // A SkSpriteBlitter must implement blitRect.
    void blitRect(int x, int y, int width, int height) override = 0;

    static SkSpriteBlitter* ChooseL32(const SkPixmap& source, const SkPaint&, SkArenaAlloc*);

protected:
    SkPixmap        fDst;
    const SkPixmap  fSource;
    int             fLeft, fTop;
    const SkPaint*  fPaint;

private:
    using INHERITED = SkBlitter;
};

#endif
