/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSpriteBlitter.h"
#include "SkSpanProcs.h"
#include "SkTemplates.h"
#include "SkXfermode.h"

class Sprite_4f : public SkSpriteBlitter {
public:
    Sprite_4f(const SkPixmap& src, const SkPaint& paint) : INHERITED(src) {
        fLoader = SkLoadSpanProc_Choose(src.info());
        fFilter = SkFilterSpanProc_Choose(paint);
        fBuffer.reset(src.width());
    }
    
protected:
    SkLoadSpanProc          fLoader;
    SkFilterSpanProc        fFilter;
    SkAutoTMalloc<SkPM4f>   fBuffer;
    
private:
    typedef SkSpriteBlitter INHERITED;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

static SkXfermode::Mode get_mode(const SkXfermode* xfer) {
    SkXfermode::Mode mode;
    if (!SkXfermode::AsMode(xfer, &mode)) {
        mode = SkXfermode::kSrcOver_Mode;
    }
    return mode;
}

class Sprite_F16 : public Sprite_4f {
public:
    Sprite_F16(const SkPixmap& src, const SkPaint& paint) : INHERITED(src, paint) {
        fState = { paint.getXfermode(), SkXfermode::kDstIsFloat16_U64Flag };
        if (src.isOpaque()) {
            fState.fFlags |= SkXfermode::kSrcIsOpaque_U64Flag;
        }
        fXfer = SkXfermode::GetU64ProcN(get_mode(fState.fXfer), fState.fFlags);
    }

    void blitRect(int x, int y, int width, int height) override {
        SkASSERT(width > 0 && height > 0);
        uint64_t* SK_RESTRICT dst = fDst.writable_addr64(x, y);
        size_t dstRB = fDst.rowBytes();

        for (int bottom = y + height; y < bottom; ++y) {
            fLoader(fSource, x - fLeft, y - fTop, fBuffer, width);
            fFilter(*fPaint, fBuffer, width);
            fXfer(fState, dst, fBuffer, width, nullptr);
            dst = (uint64_t* SK_RESTRICT)((char*)dst + dstRB);
        }
    }
    
private:
    SkXfermode::U64State    fState;
    SkXfermode::U64ProcN    fXfer;

    typedef Sprite_4f INHERITED;
};


SkSpriteBlitter* SkSpriteBlitter::ChooseF16(const SkPixmap& source, const SkPaint& paint,
                                            SkTBlitterAllocator* allocator) {
    SkASSERT(allocator != nullptr);

    if (paint.getMaskFilter() != nullptr) {
        return nullptr;
    }

    switch (source.colorType()) {
        case kN32_SkColorType:
        case kRGBA_F16_SkColorType:
            return allocator->createT<Sprite_F16>(source, paint);
        default:
            return nullptr;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class Sprite_sRGB : public Sprite_4f {
public:
    Sprite_sRGB(const SkPixmap& src, const SkPaint& paint) : INHERITED(src, paint) {
        fState = { paint.getXfermode(), SkXfermode::kDstIsSRGB_PM4fFlag };
        if (src.isOpaque()) {
            fState.fFlags |= SkXfermode::kSrcIsOpaque_PM4fFlag;
        }
        fXfer = SkXfermode::GetPM4fProcN(get_mode(fState.fXfer), fState.fFlags);
    }
    
    void blitRect(int x, int y, int width, int height) override {
        SkASSERT(width > 0 && height > 0);
        uint32_t* SK_RESTRICT dst = fDst.writable_addr32(x, y);
        size_t dstRB = fDst.rowBytes();
        
        for (int bottom = y + height; y < bottom; ++y) {
            fLoader(fSource, x - fLeft, y - fTop, fBuffer, width);
            fFilter(*fPaint, fBuffer, width);
            fXfer(fState, dst, fBuffer, width, nullptr);
            dst = (uint32_t* SK_RESTRICT)((char*)dst + dstRB);
        }
    }
    
protected:
    SkXfermode::PM4fState   fState;
    SkXfermode::PM4fProcN   fXfer;
    
private:
    typedef Sprite_4f INHERITED;
};


SkSpriteBlitter* SkSpriteBlitter::ChooseS32(const SkPixmap& source, const SkPaint& paint,
                                            SkTBlitterAllocator* allocator) {
    SkASSERT(allocator != nullptr);
    
    if (paint.getMaskFilter() != nullptr) {
        return nullptr;
    }
    
    switch (source.colorType()) {
        case kN32_SkColorType:
        case kRGBA_F16_SkColorType:
            return allocator->createT<Sprite_sRGB>(source, paint);
        default:
            return nullptr;
    }
}
