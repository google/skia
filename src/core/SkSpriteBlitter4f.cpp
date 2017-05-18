/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSpriteBlitter.h"
#include "SkArenaAlloc.h"
#include "SkSpanProcs.h"
#include "SkTemplates.h"
#include "SkXfermodePriv.h"

class Sprite_4f : public SkSpriteBlitter {
public:
    Sprite_4f(const SkPixmap& src, const SkPaint& paint) : INHERITED(src) {
        fMode = paint.getBlendMode();
        fLoader = SkLoadSpanProc_Choose(src.info());
        fFilter = SkFilterSpanProc_Choose(paint);
        fBuffer.reset(src.width());
    }

protected:
    SkBlendMode             fMode;
    SkLoadSpanProc          fLoader;
    SkFilterSpanProc        fFilter;
    SkAutoTMalloc<SkPM4f>   fBuffer;

private:
    typedef SkSpriteBlitter INHERITED;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class Sprite_F16 : public Sprite_4f {
public:
    Sprite_F16(const SkPixmap& src, const SkPaint& paint) : INHERITED(src, paint) {
        uint32_t flags = 0;
        if (src.isOpaque()) {
            flags |= SkXfermode::kSrcIsOpaque_F16Flag;
        }
        fWriter = SkXfermode::GetF16Proc(fMode, flags);
    }

    void blitRect(int x, int y, int width, int height) override {
        SkASSERT(width > 0 && height > 0);
        uint64_t* SK_RESTRICT dst = fDst.writable_addr64(x, y);
        size_t dstRB = fDst.rowBytes();

        for (int bottom = y + height; y < bottom; ++y) {
            fLoader(fSource, x - fLeft, y - fTop, fBuffer, width);
            fFilter(*fPaint, fBuffer, width);
            fWriter(fMode, dst, fBuffer, width, nullptr);
            dst = (uint64_t* SK_RESTRICT)((char*)dst + dstRB);
        }
    }

private:
    SkXfermode::F16Proc fWriter;

    typedef Sprite_4f INHERITED;
};


SkSpriteBlitter* SkSpriteBlitter::ChooseF16(const SkPixmap& source, const SkPaint& paint,
                                            SkArenaAlloc* allocator) {
    SkASSERT(allocator != nullptr);

    if (paint.getMaskFilter() != nullptr) {
        return nullptr;
    }

    switch (source.colorType()) {
        case kN32_SkColorType:
        case kRGBA_F16_SkColorType:
            return allocator->make<Sprite_F16>(source, paint);
        default:
            return nullptr;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class Sprite_sRGB : public Sprite_4f {
public:
    Sprite_sRGB(const SkPixmap& src, const SkPaint& paint) : INHERITED(src, paint) {
        uint32_t flags = SkXfermode::kDstIsSRGB_D32Flag;
        if (src.isOpaque()) {
            flags |= SkXfermode::kSrcIsOpaque_D32Flag;
        }
        fWriter = SkXfermode::GetD32Proc(fMode, flags);
    }

    void blitRect(int x, int y, int width, int height) override {
        SkASSERT(width > 0 && height > 0);
        uint32_t* SK_RESTRICT dst = fDst.writable_addr32(x, y);
        size_t dstRB = fDst.rowBytes();

        for (int bottom = y + height; y < bottom; ++y) {
            fLoader(fSource, x - fLeft, y - fTop, fBuffer, width);
            fFilter(*fPaint, fBuffer, width);
            fWriter(fMode, dst, fBuffer, width, nullptr);
            dst = (uint32_t* SK_RESTRICT)((char*)dst + dstRB);
        }
    }

protected:
    SkXfermode::D32Proc fWriter;

private:
    typedef Sprite_4f INHERITED;
};


SkSpriteBlitter* SkSpriteBlitter::ChooseS32(const SkPixmap& source, const SkPaint& paint,
                                            SkArenaAlloc* allocator) {
    SkASSERT(allocator != nullptr);

    if (paint.getMaskFilter() != nullptr) {
        return nullptr;
    }

    switch (source.colorType()) {
        case kN32_SkColorType:
        case kRGBA_F16_SkColorType:
            return allocator->make<Sprite_sRGB>(source, paint);
        default:
            return nullptr;
    }
}
