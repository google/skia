
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkSpriteBlitter.h"
#include "SkBlitRow.h"
#include "SkColorFilter.h"
#include "SkColorPriv.h"
#include "SkTemplates.h"
#include "SkUtils.h"
#include "SkXfermode.h"

///////////////////////////////////////////////////////////////////////////////

class Sprite_D32_S32 : public SkSpriteBlitter {
public:
    Sprite_D32_S32(const SkBitmap& src, U8CPU alpha)  : INHERITED(src) {
        SkASSERT(src.config() == SkBitmap::kARGB_8888_Config);

        unsigned flags32 = 0;
        if (255 != alpha) {
            flags32 |= SkBlitRow::kGlobalAlpha_Flag32;
        }
        if (!src.isOpaque()) {
            flags32 |= SkBlitRow::kSrcPixelAlpha_Flag32;
        }

        fProc32 = SkBlitRow::Factory32(flags32);
        fAlpha = alpha;
    }

    virtual void blitRect(int x, int y, int width, int height) {
        SkASSERT(width > 0 && height > 0);
        uint32_t* SK_RESTRICT dst = fDevice->getAddr32(x, y);
        const uint32_t* SK_RESTRICT src = fSource->getAddr32(x - fLeft,
                                                             y - fTop);
        size_t dstRB = fDevice->rowBytes();
        size_t srcRB = fSource->rowBytes();
        SkBlitRow::Proc32 proc = fProc32;
        U8CPU             alpha = fAlpha;

        do {
            proc(dst, src, width, alpha);
            dst = (uint32_t* SK_RESTRICT)((char*)dst + dstRB);
            src = (const uint32_t* SK_RESTRICT)((const char*)src + srcRB);
        } while (--height != 0);
    }

private:
    SkBlitRow::Proc32   fProc32;
    U8CPU               fAlpha;

    typedef SkSpriteBlitter INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class Sprite_D32_XferFilter : public SkSpriteBlitter {
public:
    Sprite_D32_XferFilter(const SkBitmap& source, const SkPaint& paint)
        : SkSpriteBlitter(source) {
        fColorFilter = paint.getColorFilter();
        SkSafeRef(fColorFilter);

        fXfermode = paint.getXfermode();
        SkSafeRef(fXfermode);

        fBufferSize = 0;
        fBuffer = NULL;

        unsigned flags32 = 0;
        if (255 != paint.getAlpha()) {
            flags32 |= SkBlitRow::kGlobalAlpha_Flag32;
        }
        if (!source.isOpaque()) {
            flags32 |= SkBlitRow::kSrcPixelAlpha_Flag32;
        }

        fProc32 = SkBlitRow::Factory32(flags32);
        fAlpha = paint.getAlpha();
    }

    virtual ~Sprite_D32_XferFilter() {
        delete[] fBuffer;
        SkSafeUnref(fXfermode);
        SkSafeUnref(fColorFilter);
    }

    virtual void setup(const SkBitmap& device, int left, int top,
                       const SkPaint& paint) {
        this->INHERITED::setup(device, left, top, paint);

        int width = device.width();
        if (width > fBufferSize) {
            fBufferSize = width;
            delete[] fBuffer;
            fBuffer = new SkPMColor[width];
        }
    }

protected:
    SkColorFilter*      fColorFilter;
    SkXfermode*         fXfermode;
    int                 fBufferSize;
    SkPMColor*          fBuffer;
    SkBlitRow::Proc32   fProc32;
    U8CPU               fAlpha;

private:
    typedef SkSpriteBlitter INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class Sprite_D32_S32A_XferFilter : public Sprite_D32_XferFilter {
public:
    Sprite_D32_S32A_XferFilter(const SkBitmap& source, const SkPaint& paint)
        : Sprite_D32_XferFilter(source, paint) {}

    virtual void blitRect(int x, int y, int width, int height) {
        SkASSERT(width > 0 && height > 0);
        uint32_t* SK_RESTRICT dst = fDevice->getAddr32(x, y);
        const uint32_t* SK_RESTRICT src = fSource->getAddr32(x - fLeft,
                                                             y - fTop);
        size_t dstRB = fDevice->rowBytes();
        size_t srcRB = fSource->rowBytes();
        SkColorFilter* colorFilter = fColorFilter;
        SkXfermode* xfermode = fXfermode;

        do {
            const SkPMColor* tmp = src;

            if (NULL != colorFilter) {
                colorFilter->filterSpan(src, width, fBuffer);
                tmp = fBuffer;
            }

            if (NULL != xfermode) {
                xfermode->xfer32(dst, tmp, width, NULL);
            } else {
                fProc32(dst, tmp, width, fAlpha);
            }

            dst = (uint32_t* SK_RESTRICT)((char*)dst + dstRB);
            src = (const uint32_t* SK_RESTRICT)((const char*)src + srcRB);
        } while (--height != 0);
    }

private:
    typedef Sprite_D32_XferFilter INHERITED;
};

static void fillbuffer(SkPMColor* SK_RESTRICT dst,
                       const SkPMColor16* SK_RESTRICT src, int count) {
    SkASSERT(count > 0);

    do {
        *dst++ = SkPixel4444ToPixel32(*src++);
    } while (--count != 0);
}

class Sprite_D32_S4444_XferFilter : public Sprite_D32_XferFilter {
public:
    Sprite_D32_S4444_XferFilter(const SkBitmap& source, const SkPaint& paint)
        : Sprite_D32_XferFilter(source, paint) {}

    virtual void blitRect(int x, int y, int width, int height) {
        SkASSERT(width > 0 && height > 0);
        SkPMColor* SK_RESTRICT dst = fDevice->getAddr32(x, y);
        const SkPMColor16* SK_RESTRICT src = fSource->getAddr16(x - fLeft,
                                                                y - fTop);
        size_t dstRB = fDevice->rowBytes();
        size_t srcRB = fSource->rowBytes();
        SkPMColor* SK_RESTRICT buffer = fBuffer;
        SkColorFilter* colorFilter = fColorFilter;
        SkXfermode* xfermode = fXfermode;

        do {
            fillbuffer(buffer, src, width);

            if (NULL != colorFilter) {
                colorFilter->filterSpan(buffer, width, buffer);
            }
            if (NULL != xfermode) {
                xfermode->xfer32(dst, buffer, width, NULL);
            } else {
                fProc32(dst, buffer, width, fAlpha);
            }

            dst = (SkPMColor* SK_RESTRICT)((char*)dst + dstRB);
            src = (const SkPMColor16* SK_RESTRICT)((const char*)src + srcRB);
        } while (--height != 0);
    }

private:
    typedef Sprite_D32_XferFilter INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static void src_row(SkPMColor* SK_RESTRICT dst,
                    const SkPMColor16* SK_RESTRICT src, int count) {
    do {
        *dst = SkPixel4444ToPixel32(*src);
        src += 1;
        dst += 1;
    } while (--count != 0);
}

class Sprite_D32_S4444_Opaque : public SkSpriteBlitter {
public:
    Sprite_D32_S4444_Opaque(const SkBitmap& source) : SkSpriteBlitter(source) {}

    virtual void blitRect(int x, int y, int width, int height) {
        SkASSERT(width > 0 && height > 0);
        SkPMColor* SK_RESTRICT dst = fDevice->getAddr32(x, y);
        const SkPMColor16* SK_RESTRICT src = fSource->getAddr16(x - fLeft,
                                                                y - fTop);
        size_t dstRB = fDevice->rowBytes();
        size_t srcRB = fSource->rowBytes();

        do {
            src_row(dst, src, width);
            dst = (SkPMColor* SK_RESTRICT)((char*)dst + dstRB);
            src = (const SkPMColor16* SK_RESTRICT)((const char*)src + srcRB);
        } while (--height != 0);
    }
};

static void srcover_row(SkPMColor* SK_RESTRICT dst,
                        const SkPMColor16* SK_RESTRICT src, int count) {
    do {
        *dst = SkPMSrcOver(SkPixel4444ToPixel32(*src), *dst);
        src += 1;
        dst += 1;
    } while (--count != 0);
}

class Sprite_D32_S4444 : public SkSpriteBlitter {
public:
    Sprite_D32_S4444(const SkBitmap& source) : SkSpriteBlitter(source) {}

    virtual void blitRect(int x, int y, int width, int height) {
        SkASSERT(width > 0 && height > 0);
        SkPMColor* SK_RESTRICT dst = fDevice->getAddr32(x, y);
        const SkPMColor16* SK_RESTRICT src = fSource->getAddr16(x - fLeft,
                                                                y - fTop);
        size_t dstRB = fDevice->rowBytes();
        size_t srcRB = fSource->rowBytes();

        do {
            srcover_row(dst, src, width);
            dst = (SkPMColor* SK_RESTRICT)((char*)dst + dstRB);
            src = (const SkPMColor16* SK_RESTRICT)((const char*)src + srcRB);
        } while (--height != 0);
    }
};

///////////////////////////////////////////////////////////////////////////////

#include "SkTemplatesPriv.h"

SkSpriteBlitter* SkSpriteBlitter::ChooseD32(const SkBitmap& source,
                                            const SkPaint& paint,
                                            void* storage, size_t storageSize) {
    if (paint.getMaskFilter() != NULL) {
        return NULL;
    }

    U8CPU       alpha = paint.getAlpha();
    SkXfermode* xfermode = paint.getXfermode();
    SkColorFilter* filter = paint.getColorFilter();
    SkSpriteBlitter* blitter = NULL;

    switch (source.config()) {
        case SkBitmap::kARGB_4444_Config:
            if (alpha != 0xFF) {
                return NULL;    // we only have opaque sprites
            }
            if (xfermode || filter) {
                SK_PLACEMENT_NEW_ARGS(blitter, Sprite_D32_S4444_XferFilter,
                                      storage, storageSize, (source, paint));
            } else if (source.isOpaque()) {
                SK_PLACEMENT_NEW_ARGS(blitter, Sprite_D32_S4444_Opaque,
                                      storage, storageSize, (source));
            } else {
                SK_PLACEMENT_NEW_ARGS(blitter, Sprite_D32_S4444,
                                      storage, storageSize, (source));
            }
            break;
        case SkBitmap::kARGB_8888_Config:
            if (xfermode || filter) {
                if (255 == alpha) {
                    // this can handle xfermode or filter, but not alpha
                    SK_PLACEMENT_NEW_ARGS(blitter, Sprite_D32_S32A_XferFilter,
                                      storage, storageSize, (source, paint));
                }
            } else {
                // this can handle alpha, but not xfermode or filter
                SK_PLACEMENT_NEW_ARGS(blitter, Sprite_D32_S32,
                              storage, storageSize, (source, alpha));
            }
            break;
        default:
            break;
    }
    return blitter;
}
