
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkSpriteBlitter_DEFINED
#define SkSpriteBlitter_DEFINED

#include "SkBitmap.h"
#include "SkBitmapProcShader.h"
#include "SkBlitter.h"
#include "SkShader.h"
#include "SkSmallAllocator.h"

class SkPaint;

class SkSpriteBlitter : public SkBlitter {
public:
            SkSpriteBlitter(const SkBitmap& source);
    virtual ~SkSpriteBlitter();

    virtual void setup(const SkBitmap& device, int left, int top,
                       const SkPaint& paint);

    // overrides
#ifdef SK_DEBUG
    virtual void    blitH(int x, int y, int width);
    virtual void    blitAntiH(int x, int y, const SkAlpha antialias[], const int16_t runs[]);
    virtual void    blitV(int x, int y, int height, SkAlpha alpha);
    virtual void    blitMask(const SkMask&, const SkIRect& clip);
#endif

    static SkSpriteBlitter* ChooseD16(const SkBitmap& source, const SkPaint&,
                                      SkTBlitterAllocator*);
    static SkSpriteBlitter* ChooseD32(const SkBitmap& source, const SkPaint&,
                                      SkTBlitterAllocator*);

protected:
    const SkBitmap* fDevice;
    const SkBitmap* fSource;
    int             fLeft, fTop;
    const SkPaint*  fPaint;
};

#endif
