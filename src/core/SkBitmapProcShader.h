
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkBitmapProcShader_DEFINED
#define SkBitmapProcShader_DEFINED

#include "SkShader.h"
#include "SkBitmapProcState.h"

class SkBitmapProcShader : public SkShader {
public:
    SkBitmapProcShader(const SkBitmap& src, TileMode tx, TileMode ty);

    // overrides from SkShader
    virtual bool setContext(const SkBitmap&, const SkPaint&, const SkMatrix&);
    virtual uint32_t getFlags() { return fFlags; }
    virtual void shadeSpan(int x, int y, SkPMColor dstC[], int count);
    virtual void shadeSpan16(int x, int y, uint16_t dstC[], int count);
    virtual void beginSession();
    virtual void endSession();
    virtual BitmapType asABitmap(SkBitmap*, SkMatrix*, TileMode*,
                                 SkScalar* twoPointRadialParams) const;

    static bool CanDo(const SkBitmap&, TileMode tx, TileMode ty);

    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer) {
        return SkNEW_ARGS(SkBitmapProcShader, (buffer));
    }

    // override from flattenable
    virtual bool toDumpString(SkString* str) const;

protected:
    SkBitmapProcShader(SkFlattenableReadBuffer& );
    virtual void flatten(SkFlattenableWriteBuffer& );
    virtual Factory getFactory() { return CreateProc; }

    SkBitmap          fRawBitmap;   // experimental for RLE encoding
    SkBitmapProcState fState;
    uint32_t          fFlags;

private:
    typedef SkShader INHERITED;
};

#endif
