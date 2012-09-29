
/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkBitmap.h"
#include "SkColorPriv.h"
#include "SkDither.h"
#include "SkFlattenable.h"
#include "SkMallocPixelRef.h"
#include "SkMask.h"
#include "SkOrderedReadBuffer.h"
#include "SkOrderedWriteBuffer.h"
#include "SkPixelRef.h"
#include "SkThread.h"
#include "SkUnPreMultiply.h"
#include "SkUtils.h"
#include "SkPackBits.h"
#include <new>

SK_DEFINE_INST_COUNT(SkBitmap::Allocator)

static bool isPos32Bits(const Sk64& value) {
    return !value.isNeg() && value.is32();
}

struct MipLevel {
    void*       fPixels;
    uint32_t    fRowBytes;
    uint32_t    fWidth, fHeight;
};

struct SkBitmap::MipMap : SkNoncopyable {
    int32_t fRefCnt;
    int     fLevelCount;
//  MipLevel    fLevel[fLevelCount];
//  Pixels[]

    static MipMap* Alloc(int levelCount, size_t pixelSize) {
        if (levelCount < 0) {
            return NULL;
        }
        Sk64 size;
        size.setMul(levelCount + 1, sizeof(MipLevel));
        size.add(sizeof(MipMap));
        size.add(pixelSize);
        if (!isPos32Bits(size)) {
            return NULL;
        }
        MipMap* mm = (MipMap*)sk_malloc_throw(size.get32());
        mm->fRefCnt = 1;
        mm->fLevelCount = levelCount;
        return mm;
    }

    const MipLevel* levels() const { return (const MipLevel*)(this + 1); }
    MipLevel* levels() { return (MipLevel*)(this + 1); }

    const void* pixels() const { return levels() + fLevelCount; }
    void* pixels() { return levels() + fLevelCount; }

    void ref() {
        if (SK_MaxS32 == sk_atomic_inc(&fRefCnt)) {
            sk_throw();
        }
    }
    void unref() {
        SkASSERT(fRefCnt > 0);
        if (sk_atomic_dec(&fRefCnt) == 1) {
            sk_free(this);
        }
    }
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

SkBitmap::SkBitmap() {
    sk_bzero(this, sizeof(*this));
}

SkBitmap::SkBitmap(const SkBitmap& src) {
    SkDEBUGCODE(src.validate();)
    sk_bzero(this, sizeof(*this));
    *this = src;
    SkDEBUGCODE(this->validate();)
}

SkBitmap::~SkBitmap() {
    SkDEBUGCODE(this->validate();)
    this->freePixels();
}

SkBitmap& SkBitmap::operator=(const SkBitmap& src) {
    if (this != &src) {
        this->freePixels();
        memcpy(this, &src, sizeof(src));

        // inc src reference counts
        SkSafeRef(src.fPixelRef);
        SkSafeRef(src.fMipMap);

        // we reset our locks if we get blown away
        fPixelLockCount = 0;

        /*  The src could be in 3 states
            1. no pixelref, in which case we just copy/ref the pixels/ctable
            2. unlocked pixelref, pixels/ctable should be null
            3. locked pixelref, we should lock the ref again ourselves
        */
        if (NULL == fPixelRef) {
            // leave fPixels as it is
            SkSafeRef(fColorTable); // ref the user's ctable if present
        } else {    // we have a pixelref, so pixels/ctable reflect it
            // ignore the values from the memcpy
            fPixels = NULL;
            fColorTable = NULL;
            // Note that what to for genID is somewhat arbitrary. We have no
            // way to track changes to raw pixels across multiple SkBitmaps.
            // Would benefit from an SkRawPixelRef type created by
            // setPixels.
            // Just leave the memcpy'ed one but they'll get out of sync
            // as soon either is modified.
        }
    }

    SkDEBUGCODE(this->validate();)
    return *this;
}

void SkBitmap::swap(SkBitmap& other) {
    SkTSwap(fColorTable, other.fColorTable);
    SkTSwap(fPixelRef, other.fPixelRef);
    SkTSwap(fPixelRefOffset, other.fPixelRefOffset);
    SkTSwap(fPixelLockCount, other.fPixelLockCount);
    SkTSwap(fMipMap, other.fMipMap);
    SkTSwap(fPixels, other.fPixels);
    SkTSwap(fRowBytes, other.fRowBytes);
    SkTSwap(fWidth, other.fWidth);
    SkTSwap(fHeight, other.fHeight);
    SkTSwap(fConfig, other.fConfig);
    SkTSwap(fFlags, other.fFlags);
    SkTSwap(fBytesPerPixel, other.fBytesPerPixel);

    SkDEBUGCODE(this->validate();)
}

void SkBitmap::reset() {
    this->freePixels();
    sk_bzero(this, sizeof(*this));
}

int SkBitmap::ComputeBytesPerPixel(SkBitmap::Config config) {
    int bpp;
    switch (config) {
        case kNo_Config:
        case kA1_Config:
            bpp = 0;   // not applicable
            break;
        case kRLE_Index8_Config:
        case kA8_Config:
        case kIndex8_Config:
            bpp = 1;
            break;
        case kRGB_565_Config:
        case kARGB_4444_Config:
            bpp = 2;
            break;
        case kARGB_8888_Config:
            bpp = 4;
            break;
        default:
            SkDEBUGFAIL("unknown config");
            bpp = 0;   // error
            break;
    }
    return bpp;
}

int SkBitmap::ComputeRowBytes(Config c, int width) {
    if (width < 0) {
        return 0;
    }

    Sk64 rowBytes;
    rowBytes.setZero();

    switch (c) {
        case kNo_Config:
        case kRLE_Index8_Config:
            break;
        case kA1_Config:
            rowBytes.set(width);
            rowBytes.add(7);
            rowBytes.shiftRight(3);
            break;
        case kA8_Config:
        case kIndex8_Config:
            rowBytes.set(width);
            break;
        case kRGB_565_Config:
        case kARGB_4444_Config:
            rowBytes.set(width);
            rowBytes.shiftLeft(1);
            break;
        case kARGB_8888_Config:
            rowBytes.set(width);
            rowBytes.shiftLeft(2);
            break;
        default:
            SkDEBUGFAIL("unknown config");
            break;
    }
    return isPos32Bits(rowBytes) ? rowBytes.get32() : 0;
}

Sk64 SkBitmap::ComputeSize64(Config c, int width, int height) {
    Sk64 size;
    size.setMul(SkBitmap::ComputeRowBytes(c, width), height);
    return size;
}

size_t SkBitmap::ComputeSize(Config c, int width, int height) {
    Sk64 size = SkBitmap::ComputeSize64(c, width, height);
    return isPos32Bits(size) ? size.get32() : 0;
}

Sk64 SkBitmap::ComputeSafeSize64(Config config,
                                 uint32_t width,
                                 uint32_t height,
                                 uint32_t rowBytes) {
    Sk64 safeSize;
    safeSize.setZero();
    if (height > 0) {
        safeSize.set(ComputeRowBytes(config, width));
        Sk64 sizeAllButLastRow;
        sizeAllButLastRow.setMul(height - 1, rowBytes);
        safeSize.add(sizeAllButLastRow);
    }
    SkASSERT(!safeSize.isNeg());
    return safeSize;
}

size_t SkBitmap::ComputeSafeSize(Config config,
                                 uint32_t width,
                                 uint32_t height,
                                 uint32_t rowBytes) {
    Sk64 safeSize = ComputeSafeSize64(config, width, height, rowBytes);
    return (safeSize.is32() ? safeSize.get32() : 0);
}

void SkBitmap::getBounds(SkRect* bounds) const {
    SkASSERT(bounds);
    bounds->set(0, 0,
                SkIntToScalar(fWidth), SkIntToScalar(fHeight));
}

void SkBitmap::getBounds(SkIRect* bounds) const {
    SkASSERT(bounds);
    bounds->set(0, 0, fWidth, fHeight);
}

///////////////////////////////////////////////////////////////////////////////

void SkBitmap::setConfig(Config c, int width, int height, int rowBytes) {
    this->freePixels();

    if ((width | height | rowBytes) < 0) {
        goto err;
    }

    if (rowBytes == 0) {
        rowBytes = SkBitmap::ComputeRowBytes(c, width);
        if (0 == rowBytes && kNo_Config != c) {
            goto err;
        }
    }

    fConfig     = SkToU8(c);
    fWidth      = width;
    fHeight     = height;
    fRowBytes   = rowBytes;

    fBytesPerPixel = (uint8_t)ComputeBytesPerPixel(c);

    SkDEBUGCODE(this->validate();)
    return;

    // if we got here, we had an error, so we reset the bitmap to empty
err:
    this->reset();
}

void SkBitmap::updatePixelsFromRef() const {
    if (NULL != fPixelRef) {
        if (fPixelLockCount > 0) {
            SkASSERT(fPixelRef->isLocked());

            void* p = fPixelRef->pixels();
            if (NULL != p) {
                p = (char*)p + fPixelRefOffset;
            }
            fPixels = p;
            SkRefCnt_SafeAssign(fColorTable, fPixelRef->colorTable());
        } else {
            SkASSERT(0 == fPixelLockCount);
            fPixels = NULL;
            if (fColorTable) {
                fColorTable->unref();
                fColorTable = NULL;
            }
        }
    }
}

SkPixelRef* SkBitmap::setPixelRef(SkPixelRef* pr, size_t offset) {
    // do this first, we that we never have a non-zero offset with a null ref
    if (NULL == pr) {
        offset = 0;
    }

    if (fPixelRef != pr || fPixelRefOffset != offset) {
        if (fPixelRef != pr) {
            this->freePixels();
            SkASSERT(NULL == fPixelRef);

            SkSafeRef(pr);
            fPixelRef = pr;
        }
        fPixelRefOffset = offset;
        this->updatePixelsFromRef();
    }

    SkDEBUGCODE(this->validate();)
    return pr;
}

void SkBitmap::lockPixels() const {
    if (NULL != fPixelRef && 1 == ++fPixelLockCount) {
        fPixelRef->lockPixels();
        this->updatePixelsFromRef();
    }
    SkDEBUGCODE(this->validate();)
}

void SkBitmap::unlockPixels() const {
    SkASSERT(NULL == fPixelRef || fPixelLockCount > 0);

    if (NULL != fPixelRef && 0 == --fPixelLockCount) {
        fPixelRef->unlockPixels();
        this->updatePixelsFromRef();
    }
    SkDEBUGCODE(this->validate();)
}

bool SkBitmap::lockPixelsAreWritable() const {
    return (fPixelRef) ? fPixelRef->lockPixelsAreWritable() : false;
}

void SkBitmap::setPixels(void* p, SkColorTable* ctable) {
    if (NULL == p) {
        this->setPixelRef(NULL, 0);
        return;
    }

    Sk64 size = this->getSize64();
    SkASSERT(!size.isNeg() && size.is32());

    this->setPixelRef(new SkMallocPixelRef(p, size.get32(), ctable, false))->unref();
    // since we're already allocated, we lockPixels right away
    this->lockPixels();
    SkDEBUGCODE(this->validate();)
}

bool SkBitmap::allocPixels(Allocator* allocator, SkColorTable* ctable) {
    HeapAllocator stdalloc;

    if (NULL == allocator) {
        allocator = &stdalloc;
    }
    return allocator->allocPixelRef(this, ctable);
}

void SkBitmap::freePixels() {
    // if we're gonna free the pixels, we certainly need to free the mipmap
    this->freeMipMap();

    if (fColorTable) {
        fColorTable->unref();
        fColorTable = NULL;
    }

    if (NULL != fPixelRef) {
        if (fPixelLockCount > 0) {
            fPixelRef->unlockPixels();
        }
        fPixelRef->unref();
        fPixelRef = NULL;
        fPixelRefOffset = 0;
    }
    fPixelLockCount = 0;
    fPixels = NULL;
}

void SkBitmap::freeMipMap() {
    if (fMipMap) {
        fMipMap->unref();
        fMipMap = NULL;
    }
}

uint32_t SkBitmap::getGenerationID() const {
    return (fPixelRef) ? fPixelRef->getGenerationID() : 0;
}

void SkBitmap::notifyPixelsChanged() const {
    SkASSERT(!this->isImmutable());
    if (fPixelRef) {
        fPixelRef->notifyPixelsChanged();
    }
}

SkGpuTexture* SkBitmap::getTexture() const {
    return fPixelRef ? fPixelRef->getTexture() : NULL;
}

///////////////////////////////////////////////////////////////////////////////

/** We explicitly use the same allocator for our pixels that SkMask does,
 so that we can freely assign memory allocated by one class to the other.
 */
bool SkBitmap::HeapAllocator::allocPixelRef(SkBitmap* dst,
                                            SkColorTable* ctable) {
    Sk64 size = dst->getSize64();
    if (size.isNeg() || !size.is32()) {
        return false;
    }

    void* addr = sk_malloc_flags(size.get32(), 0);  // returns NULL on failure
    if (NULL == addr) {
        return false;
    }

    dst->setPixelRef(new SkMallocPixelRef(addr, size.get32(), ctable))->unref();
    // since we're already allocated, we lockPixels right away
    dst->lockPixels();
    return true;
}

///////////////////////////////////////////////////////////////////////////////

size_t SkBitmap::getSafeSize() const {
    // This is intended to be a size_t version of ComputeSafeSize64(), just
    // faster. The computation is meant to be identical.
    return (fHeight ? ((fHeight - 1) * fRowBytes) +
            ComputeRowBytes(getConfig(), fWidth): 0);
}

Sk64 SkBitmap::getSafeSize64() const {
    return ComputeSafeSize64(getConfig(), fWidth, fHeight, fRowBytes);
}

bool SkBitmap::copyPixelsTo(void* const dst, size_t dstSize,
                            int dstRowBytes, bool preserveDstPad) const {

    if (dstRowBytes == -1)
        dstRowBytes = fRowBytes;
    SkASSERT(dstRowBytes >= 0);

    if (getConfig() == kRLE_Index8_Config ||
        dstRowBytes < ComputeRowBytes(getConfig(), fWidth) ||
        dst == NULL || (getPixels() == NULL && pixelRef() == NULL))
        return false;

    if (!preserveDstPad && static_cast<uint32_t>(dstRowBytes) == fRowBytes) {
        size_t safeSize = getSafeSize();
        if (safeSize > dstSize || safeSize == 0)
            return false;
        else {
            SkAutoLockPixels lock(*this);
            // This implementation will write bytes beyond the end of each row,
            // excluding the last row, if the bitmap's stride is greater than
            // strictly required by the current config.
            memcpy(dst, getPixels(), safeSize);

            return true;
        }
    } else {
        // If destination has different stride than us, then copy line by line.
        if (ComputeSafeSize(getConfig(), fWidth, fHeight, dstRowBytes) >
            dstSize)
            return false;
        else {
            // Just copy what we need on each line.
            uint32_t rowBytes = ComputeRowBytes(getConfig(), fWidth);
            SkAutoLockPixels lock(*this);
            const uint8_t* srcP = reinterpret_cast<const uint8_t*>(getPixels());
            uint8_t* dstP = reinterpret_cast<uint8_t*>(dst);
            for (uint32_t row = 0; row < fHeight;
                 row++, srcP += fRowBytes, dstP += dstRowBytes) {
                memcpy(dstP, srcP, rowBytes);
            }

            return true;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

bool SkBitmap::isImmutable() const {
    return fPixelRef ? fPixelRef->isImmutable() :
        fFlags & kImageIsImmutable_Flag;
}

void SkBitmap::setImmutable() {
    if (fPixelRef) {
        fPixelRef->setImmutable();
    } else {
        fFlags |= kImageIsImmutable_Flag;
    }
}

bool SkBitmap::isOpaque() const {
    switch (fConfig) {
        case kNo_Config:
            return true;

        case kA1_Config:
        case kA8_Config:
        case kARGB_4444_Config:
        case kARGB_8888_Config:
            return (fFlags & kImageIsOpaque_Flag) != 0;

        case kIndex8_Config:
        case kRLE_Index8_Config: {
                uint32_t flags = 0;

                this->lockPixels();
                // if lockPixels failed, we may not have a ctable ptr
                if (fColorTable) {
                    flags = fColorTable->getFlags();
                }
                this->unlockPixels();

                return (flags & SkColorTable::kColorsAreOpaque_Flag) != 0;
            }

        case kRGB_565_Config:
            return true;

        default:
            SkDEBUGFAIL("unknown bitmap config pased to isOpaque");
            return false;
    }
}

void SkBitmap::setIsOpaque(bool isOpaque) {
    /*  we record this regardless of fConfig, though it is ignored in
        isOpaque() for configs that can't support per-pixel alpha.
    */
    if (isOpaque) {
        fFlags |= kImageIsOpaque_Flag;
    } else {
        fFlags &= ~kImageIsOpaque_Flag;
    }
}

bool SkBitmap::isVolatile() const {
    return (fFlags & kImageIsVolatile_Flag) != 0;
}

void SkBitmap::setIsVolatile(bool isVolatile) {
    if (isVolatile) {
        fFlags |= kImageIsVolatile_Flag;
    } else {
        fFlags &= ~kImageIsVolatile_Flag;
    }
}

void* SkBitmap::getAddr(int x, int y) const {
    SkASSERT((unsigned)x < (unsigned)this->width());
    SkASSERT((unsigned)y < (unsigned)this->height());

    char* base = (char*)this->getPixels();
    if (base) {
        base += y * this->rowBytes();
        switch (this->config()) {
            case SkBitmap::kARGB_8888_Config:
                base += x << 2;
                break;
            case SkBitmap::kARGB_4444_Config:
            case SkBitmap::kRGB_565_Config:
                base += x << 1;
                break;
            case SkBitmap::kA8_Config:
            case SkBitmap::kIndex8_Config:
                base += x;
                break;
            case SkBitmap::kA1_Config:
                base += x >> 3;
                break;
            case kRLE_Index8_Config:
                SkDEBUGFAIL("Can't return addr for kRLE_Index8_Config");
                base = NULL;
                break;
            default:
                SkDEBUGFAIL("Can't return addr for config");
                base = NULL;
                break;
        }
    }
    return base;
}

SkColor SkBitmap::getColor(int x, int y) const {
    SkASSERT((unsigned)x < (unsigned)this->width());
    SkASSERT((unsigned)y < (unsigned)this->height());

    switch (this->config()) {
        case SkBitmap::kA1_Config: {
            uint8_t* addr = this->getAddr1(x, y);
            uint8_t mask = 1 << (7  - (x % 8));
            if (addr[0] & mask) {
                return SK_ColorBLACK;
            } else {
                return 0;
            }
        }
        case SkBitmap::kA8_Config: {
            uint8_t* addr = this->getAddr8(x, y);
            return SkColorSetA(0, addr[0]);
        }
        case SkBitmap::kIndex8_Config: {
            SkPMColor c = this->getIndex8Color(x, y);
            return SkUnPreMultiply::PMColorToColor(c);
        }
        case SkBitmap::kRGB_565_Config: {
            uint16_t* addr = this->getAddr16(x, y);
            return SkPixel16ToColor(addr[0]);
        }
        case SkBitmap::kARGB_4444_Config: {
            uint16_t* addr = this->getAddr16(x, y);
            SkPMColor c = SkPixel4444ToPixel32(addr[0]);
            return SkUnPreMultiply::PMColorToColor(c);
        }
        case SkBitmap::kARGB_8888_Config: {
            uint32_t* addr = this->getAddr32(x, y);
            return SkUnPreMultiply::PMColorToColor(addr[0]);
        }
        case kRLE_Index8_Config: {
            uint8_t dst;
            const SkBitmap::RLEPixels* rle =
                (const SkBitmap::RLEPixels*)this->getPixels();
            SkPackBits::Unpack8(&dst, x, 1, rle->packedAtY(y));
            return SkUnPreMultiply::PMColorToColor((*fColorTable)[dst]);
        }
        case kNo_Config:
        case kConfigCount:
            SkASSERT(false);
            return 0;
    }
    SkASSERT(false);  // Not reached.
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SkBitmap::eraseARGB(U8CPU a, U8CPU r, U8CPU g, U8CPU b) const {
    SkDEBUGCODE(this->validate();)

    if (0 == fWidth || 0 == fHeight ||
            kNo_Config == fConfig || kIndex8_Config == fConfig) {
        return;
    }

    SkAutoLockPixels alp(*this);
    // perform this check after the lock call
    if (!this->readyToDraw()) {
        return;
    }

    int height = fHeight;
    const int width = fWidth;
    const int rowBytes = fRowBytes;

    // make rgb premultiplied
    if (255 != a) {
        r = SkAlphaMul(r, a);
        g = SkAlphaMul(g, a);
        b = SkAlphaMul(b, a);
    }

    switch (fConfig) {
        case kA1_Config: {
            uint8_t* p = (uint8_t*)fPixels;
            const int count = (width + 7) >> 3;
            a = (a >> 7) ? 0xFF : 0;
            SkASSERT(count <= rowBytes);
            while (--height >= 0) {
                memset(p, a, count);
                p += rowBytes;
            }
            break;
        }
        case kA8_Config: {
            uint8_t* p = (uint8_t*)fPixels;
            while (--height >= 0) {
                memset(p, a, width);
                p += rowBytes;
            }
            break;
        }
        case kARGB_4444_Config:
        case kRGB_565_Config: {
            uint16_t* p = (uint16_t*)fPixels;
            uint16_t v;

            if (kARGB_4444_Config == fConfig) {
                v = SkPackARGB4444(a >> 4, r >> 4, g >> 4, b >> 4);
            } else {    // kRGB_565_Config
                v = SkPackRGB16(r >> (8 - SK_R16_BITS), g >> (8 - SK_G16_BITS),
                                b >> (8 - SK_B16_BITS));
            }
            while (--height >= 0) {
                sk_memset16(p, v, width);
                p = (uint16_t*)((char*)p + rowBytes);
            }
            break;
        }
        case kARGB_8888_Config: {
            uint32_t* p = (uint32_t*)fPixels;
            uint32_t  v = SkPackARGB32(a, r, g, b);

            while (--height >= 0) {
                sk_memset32(p, v, width);
                p = (uint32_t*)((char*)p + rowBytes);
            }
            break;
        }
    }

    this->notifyPixelsChanged();
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

#define SUB_OFFSET_FAILURE  ((size_t)-1)

static size_t getSubOffset(const SkBitmap& bm, int x, int y) {
    SkASSERT((unsigned)x < (unsigned)bm.width());
    SkASSERT((unsigned)y < (unsigned)bm.height());

    switch (bm.getConfig()) {
        case SkBitmap::kA8_Config:
        case SkBitmap:: kIndex8_Config:
            // x is fine as is for the calculation
            break;

        case SkBitmap::kRGB_565_Config:
        case SkBitmap::kARGB_4444_Config:
            x <<= 1;
            break;

        case SkBitmap::kARGB_8888_Config:
            x <<= 2;
            break;

        case SkBitmap::kNo_Config:
        case SkBitmap::kA1_Config:
        default:
            return SUB_OFFSET_FAILURE;
    }
    return y * bm.rowBytes() + x;
}

bool SkBitmap::extractSubset(SkBitmap* result, const SkIRect& subset) const {
    SkDEBUGCODE(this->validate();)

    if (NULL == result || NULL == fPixelRef) {
        return false;   // no src pixels
    }

    SkIRect srcRect, r;
    srcRect.set(0, 0, this->width(), this->height());
    if (!r.intersect(srcRect, subset)) {
        return false;   // r is empty (i.e. no intersection)
    }

    if (kRLE_Index8_Config == fConfig) {
        SkAutoLockPixels alp(*this);
        // don't call readyToDraw(), since we can operate w/o a colortable
        // at this stage
        if (this->getPixels() == NULL) {
            return false;
        }
        SkBitmap bm;

        bm.setConfig(kIndex8_Config, r.width(), r.height());
        bm.allocPixels(this->getColorTable());
        if (NULL == bm.getPixels()) {
            return false;
        }

        const RLEPixels* rle = (const RLEPixels*)this->getPixels();
        uint8_t* dst = bm.getAddr8(0, 0);
        const int width = bm.width();
        const int rowBytes = bm.rowBytes();

        for (int y = r.fTop; y < r.fBottom; y++) {
            SkPackBits::Unpack8(dst, r.fLeft, width, rle->packedAtY(y));
            dst += rowBytes;
        }
        result->swap(bm);
        return true;
    }

    size_t offset = getSubOffset(*this, r.fLeft, r.fTop);
    if (SUB_OFFSET_FAILURE == offset) {
        return false;   // config not supported
    }

    SkBitmap dst;
    dst.setConfig(this->config(), r.width(), r.height(), this->rowBytes());
    dst.setIsVolatile(this->isVolatile());

    if (fPixelRef) {
        // share the pixelref with a custom offset
        dst.setPixelRef(fPixelRef, fPixelRefOffset + offset);
    }
    SkDEBUGCODE(dst.validate();)

    // we know we're good, so commit to result
    result->swap(dst);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

#include "SkCanvas.h"
#include "SkPaint.h"

bool SkBitmap::canCopyTo(Config dstConfig) const {
    if (this->getConfig() == kNo_Config) {
        return false;
    }

    bool sameConfigs = (this->config() == dstConfig);
    switch (dstConfig) {
        case kA8_Config:
        case kARGB_4444_Config:
        case kRGB_565_Config:
        case kARGB_8888_Config:
            break;
        case kA1_Config:
        case kIndex8_Config:
            if (!sameConfigs) {
                return false;
            }
            break;
        default:
            return false;
    }

    // do not copy src if srcConfig == kA1_Config while dstConfig != kA1_Config
    if (this->getConfig() == kA1_Config && !sameConfigs) {
        return false;
    }

    return true;
}

bool SkBitmap::copyTo(SkBitmap* dst, Config dstConfig, Allocator* alloc) const {
    if (!this->canCopyTo(dstConfig)) {
        return false;
    }

    // if we have a texture, first get those pixels
    SkBitmap tmpSrc;
    const SkBitmap* src = this;

    if (fPixelRef && fPixelRef->readPixels(&tmpSrc)) {
        SkASSERT(tmpSrc.width() == this->width());
        SkASSERT(tmpSrc.height() == this->height());

        // did we get lucky and we can just return tmpSrc?
        if (tmpSrc.config() == dstConfig && NULL == alloc) {
            dst->swap(tmpSrc);
            if (dst->pixelRef()) {
                dst->pixelRef()->fGenerationID = fPixelRef->getGenerationID();
            }
            return true;
        }

        // fall through to the raster case
        src = &tmpSrc;
    }

    // we lock this now, since we may need its colortable
    SkAutoLockPixels srclock(*src);
    if (!src->readyToDraw()) {
        return false;
    }

    SkBitmap tmpDst;
    tmpDst.setConfig(dstConfig, src->width(), src->height());

    // allocate colortable if srcConfig == kIndex8_Config
    SkColorTable* ctable = (dstConfig == kIndex8_Config) ?
    new SkColorTable(*src->getColorTable()) : NULL;
    SkAutoUnref au(ctable);
    if (!tmpDst.allocPixels(alloc, ctable)) {
        return false;
    }

    if (!tmpDst.readyToDraw()) {
        // allocator/lock failed
        return false;
    }

    /* do memcpy for the same configs cases, else use drawing
    */
    if (src->config() == dstConfig) {
        if (tmpDst.getSize() == src->getSize()) {
            memcpy(tmpDst.getPixels(), src->getPixels(), src->getSafeSize());
            SkPixelRef* pixelRef = tmpDst.pixelRef();
            if (pixelRef != NULL) {
                pixelRef->fGenerationID = this->getGenerationID();
            }
        } else {
            const char* srcP = reinterpret_cast<const char*>(src->getPixels());
            char* dstP = reinterpret_cast<char*>(tmpDst.getPixels());
            // to be sure we don't read too much, only copy our logical pixels
            size_t bytesToCopy = tmpDst.width() * tmpDst.bytesPerPixel();
            for (int y = 0; y < tmpDst.height(); y++) {
                memcpy(dstP, srcP, bytesToCopy);
                srcP += src->rowBytes();
                dstP += tmpDst.rowBytes();
            }
        }
    } else {
        // if the src has alpha, we have to clear the dst first
        if (!src->isOpaque()) {
            tmpDst.eraseColor(0);
        }

        SkCanvas canvas(tmpDst);
        SkPaint  paint;

        paint.setDither(true);
        canvas.drawBitmap(*src, 0, 0, &paint);
    }

    tmpDst.setIsOpaque(src->isOpaque());

    dst->swap(tmpDst);
    return true;
}

bool SkBitmap::deepCopyTo(SkBitmap* dst, Config dstConfig) const {
    if (!this->canCopyTo(dstConfig)) {
        return false;
    }

    // If we have a PixelRef, and it supports deep copy, use it.
    // Currently supported only by texture-backed bitmaps.
    if (fPixelRef) {
        SkPixelRef* pixelRef = fPixelRef->deepCopy(dstConfig);
        if (pixelRef) {
            if (dstConfig == fConfig) {
                pixelRef->fGenerationID = fPixelRef->getGenerationID();
            }
            dst->setConfig(dstConfig, fWidth, fHeight);
            dst->setPixelRef(pixelRef)->unref();
            return true;
        }
    }

    if (this->getTexture()) {
        return false;
    } else {
        return this->copyTo(dst, dstConfig, NULL);
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void downsampleby2_proc32(SkBitmap* dst, int x, int y,
                                 const SkBitmap& src) {
    x <<= 1;
    y <<= 1;
    const SkPMColor* p = src.getAddr32(x, y);
    const SkPMColor* baseP = p;
    SkPMColor c, ag, rb;

    c = *p; ag = (c >> 8) & 0xFF00FF; rb = c & 0xFF00FF;
    if (x < src.width() - 1) {
        p += 1;
    }
    c = *p; ag += (c >> 8) & 0xFF00FF; rb += c & 0xFF00FF;

    p = baseP;
    if (y < src.height() - 1) {
        p += src.rowBytes() >> 2;
    }
    c = *p; ag += (c >> 8) & 0xFF00FF; rb += c & 0xFF00FF;
    if (x < src.width() - 1) {
        p += 1;
    }
    c = *p; ag += (c >> 8) & 0xFF00FF; rb += c & 0xFF00FF;

    *dst->getAddr32(x >> 1, y >> 1) =
        ((rb >> 2) & 0xFF00FF) | ((ag << 6) & 0xFF00FF00);
}

static inline uint32_t expand16(U16CPU c) {
    return (c & ~SK_G16_MASK_IN_PLACE) | ((c & SK_G16_MASK_IN_PLACE) << 16);
}

// returns dirt in the top 16bits, but we don't care, since we only
// store the low 16bits.
static inline U16CPU pack16(uint32_t c) {
    return (c & ~SK_G16_MASK_IN_PLACE) | ((c >> 16) & SK_G16_MASK_IN_PLACE);
}

static void downsampleby2_proc16(SkBitmap* dst, int x, int y,
                                 const SkBitmap& src) {
    x <<= 1;
    y <<= 1;
    const uint16_t* p = src.getAddr16(x, y);
    const uint16_t* baseP = p;
    SkPMColor       c;

    c = expand16(*p);
    if (x < src.width() - 1) {
        p += 1;
    }
    c += expand16(*p);

    p = baseP;
    if (y < src.height() - 1) {
        p += src.rowBytes() >> 1;
    }
    c += expand16(*p);
    if (x < src.width() - 1) {
        p += 1;
    }
    c += expand16(*p);

    *dst->getAddr16(x >> 1, y >> 1) = (uint16_t)pack16(c >> 2);
}

static uint32_t expand4444(U16CPU c) {
    return (c & 0xF0F) | ((c & ~0xF0F) << 12);
}

static U16CPU collaps4444(uint32_t c) {
    return (c & 0xF0F) | ((c >> 12) & ~0xF0F);
}

static void downsampleby2_proc4444(SkBitmap* dst, int x, int y,
                                   const SkBitmap& src) {
    x <<= 1;
    y <<= 1;
    const uint16_t* p = src.getAddr16(x, y);
    const uint16_t* baseP = p;
    uint32_t        c;

    c = expand4444(*p);
    if (x < src.width() - 1) {
        p += 1;
    }
    c += expand4444(*p);

    p = baseP;
    if (y < src.height() - 1) {
        p += src.rowBytes() >> 1;
    }
    c += expand4444(*p);
    if (x < src.width() - 1) {
        p += 1;
    }
    c += expand4444(*p);

    *dst->getAddr16(x >> 1, y >> 1) = (uint16_t)collaps4444(c >> 2);
}

void SkBitmap::buildMipMap(bool forceRebuild) {
    if (forceRebuild)
        this->freeMipMap();
    else if (fMipMap)
        return; // we're already built

    SkASSERT(NULL == fMipMap);

    void (*proc)(SkBitmap* dst, int x, int y, const SkBitmap& src);

    const SkBitmap::Config config = this->getConfig();

    switch (config) {
        case kARGB_8888_Config:
            proc = downsampleby2_proc32;
            break;
        case kRGB_565_Config:
            proc = downsampleby2_proc16;
            break;
        case kARGB_4444_Config:
            proc = downsampleby2_proc4444;
            break;
        case kIndex8_Config:
        case kA8_Config:
        default:
            return; // don't build mipmaps for these configs
    }

    SkAutoLockPixels alp(*this);
    if (!this->readyToDraw()) {
        return;
    }

    // whip through our loop to compute the exact size needed
    size_t  size = 0;
    int     maxLevels = 0;
    {
        int width = this->width();
        int height = this->height();
        for (;;) {
            width >>= 1;
            height >>= 1;
            if (0 == width || 0 == height) {
                break;
            }
            size += ComputeRowBytes(config, width) * height;
            maxLevels += 1;
        }
    }

    // nothing to build
    if (0 == maxLevels) {
        return;
    }

    SkBitmap srcBM(*this);
    srcBM.lockPixels();
    if (!srcBM.readyToDraw()) {
        return;
    }

    MipMap* mm = MipMap::Alloc(maxLevels, size);
    if (NULL == mm) {
        return;
    }

    MipLevel*   level = mm->levels();
    uint8_t*    addr = (uint8_t*)mm->pixels();
    int         width = this->width();
    int         height = this->height();
    unsigned    rowBytes = this->rowBytes();
    SkBitmap    dstBM;

    for (int i = 0; i < maxLevels; i++) {
        width >>= 1;
        height >>= 1;
        rowBytes = ComputeRowBytes(config, width);

        level[i].fPixels   = addr;
        level[i].fWidth    = width;
        level[i].fHeight   = height;
        level[i].fRowBytes = rowBytes;

        dstBM.setConfig(config, width, height, rowBytes);
        dstBM.setPixels(addr);

        srcBM.lockPixels();
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                proc(&dstBM, x, y, srcBM);
            }
        }
        srcBM.unlockPixels();

        srcBM = dstBM;
        addr += height * rowBytes;
    }
    SkASSERT(addr == (uint8_t*)mm->pixels() + size);
    fMipMap = mm;
}

bool SkBitmap::hasMipMap() const {
    return fMipMap != NULL;
}

int SkBitmap::extractMipLevel(SkBitmap* dst, SkFixed sx, SkFixed sy) {
    if (NULL == fMipMap) {
        return 0;
    }

    int level = ComputeMipLevel(sx, sy) >> 16;
    SkASSERT(level >= 0);
    if (level <= 0) {
        return 0;
    }

    if (level >= fMipMap->fLevelCount) {
        level = fMipMap->fLevelCount - 1;
    }
    if (dst) {
        const MipLevel& mip = fMipMap->levels()[level - 1];
        dst->setConfig((SkBitmap::Config)this->config(),
                       mip.fWidth, mip.fHeight, mip.fRowBytes);
        dst->setPixels(mip.fPixels);
    }
    return level;
}

SkFixed SkBitmap::ComputeMipLevel(SkFixed sx, SkFixed sy) {
    sx = SkAbs32(sx);
    sy = SkAbs32(sy);
    if (sx < sy) {
        sx = sy;
    }
    if (sx < SK_Fixed1) {
        return 0;
    }
    int clz = SkCLZ(sx);
    SkASSERT(clz >= 1 && clz <= 15);
    return SkIntToFixed(15 - clz) + ((unsigned)(sx << (clz + 1)) >> 16);
}

///////////////////////////////////////////////////////////////////////////////

static bool GetBitmapAlpha(const SkBitmap& src, uint8_t* SK_RESTRICT alpha,
                           int alphaRowBytes) {
    SkASSERT(alpha != NULL);
    SkASSERT(alphaRowBytes >= src.width());

    SkBitmap::Config config = src.getConfig();
    int              w = src.width();
    int              h = src.height();
    int              rb = src.rowBytes();

    SkAutoLockPixels alp(src);
    if (!src.readyToDraw()) {
        // zero out the alpha buffer and return
        while (--h >= 0) {
            memset(alpha, 0, w);
            alpha += alphaRowBytes;
        }
        return false;
    }

    if (SkBitmap::kA8_Config == config && !src.isOpaque()) {
        const uint8_t* s = src.getAddr8(0, 0);
        while (--h >= 0) {
            memcpy(alpha, s, w);
            s += rb;
            alpha += alphaRowBytes;
        }
    } else if (SkBitmap::kARGB_8888_Config == config && !src.isOpaque()) {
        const SkPMColor* SK_RESTRICT s = src.getAddr32(0, 0);
        while (--h >= 0) {
            for (int x = 0; x < w; x++) {
                alpha[x] = SkGetPackedA32(s[x]);
            }
            s = (const SkPMColor*)((const char*)s + rb);
            alpha += alphaRowBytes;
        }
    } else if (SkBitmap::kARGB_4444_Config == config && !src.isOpaque()) {
        const SkPMColor16* SK_RESTRICT s = src.getAddr16(0, 0);
        while (--h >= 0) {
            for (int x = 0; x < w; x++) {
                alpha[x] = SkPacked4444ToA32(s[x]);
            }
            s = (const SkPMColor16*)((const char*)s + rb);
            alpha += alphaRowBytes;
        }
    } else if (SkBitmap::kIndex8_Config == config && !src.isOpaque()) {
        SkColorTable* ct = src.getColorTable();
        if (ct) {
            const SkPMColor* SK_RESTRICT table = ct->lockColors();
            const uint8_t* SK_RESTRICT s = src.getAddr8(0, 0);
            while (--h >= 0) {
                for (int x = 0; x < w; x++) {
                    alpha[x] = SkGetPackedA32(table[s[x]]);
                }
                s += rb;
                alpha += alphaRowBytes;
            }
            ct->unlockColors(false);
        }
    } else {    // src is opaque, so just fill alpha[] with 0xFF
        memset(alpha, 0xFF, h * alphaRowBytes);
    }
    return true;
}

#include "SkPaint.h"
#include "SkMaskFilter.h"
#include "SkMatrix.h"

bool SkBitmap::extractAlpha(SkBitmap* dst, const SkPaint* paint,
                            Allocator *allocator, SkIPoint* offset) const {
    SkDEBUGCODE(this->validate();)

    SkBitmap    tmpBitmap;
    SkMatrix    identity;
    SkMask      srcM, dstM;

    srcM.fBounds.set(0, 0, this->width(), this->height());
    srcM.fRowBytes = SkAlign4(this->width());
    srcM.fFormat = SkMask::kA8_Format;

    SkMaskFilter* filter = paint ? paint->getMaskFilter() : NULL;

    // compute our (larger?) dst bounds if we have a filter
    if (NULL != filter) {
        identity.reset();
        srcM.fImage = NULL;
        if (!filter->filterMask(&dstM, srcM, identity, NULL)) {
            goto NO_FILTER_CASE;
        }
        dstM.fRowBytes = SkAlign4(dstM.fBounds.width());
    } else {
    NO_FILTER_CASE:
        tmpBitmap.setConfig(SkBitmap::kA8_Config, this->width(), this->height(),
                       srcM.fRowBytes);
        if (!tmpBitmap.allocPixels(allocator, NULL)) {
            // Allocation of pixels for alpha bitmap failed.
            SkDebugf("extractAlpha failed to allocate (%d,%d) alpha bitmap\n",
                    tmpBitmap.width(), tmpBitmap.height());
            return false;
        }
        GetBitmapAlpha(*this, tmpBitmap.getAddr8(0, 0), srcM.fRowBytes);
        if (offset) {
            offset->set(0, 0);
        }
        tmpBitmap.swap(*dst);
        return true;
    }
    srcM.fImage = SkMask::AllocImage(srcM.computeImageSize());
    SkAutoMaskFreeImage srcCleanup(srcM.fImage);

    GetBitmapAlpha(*this, srcM.fImage, srcM.fRowBytes);
    if (!filter->filterMask(&dstM, srcM, identity, NULL)) {
        goto NO_FILTER_CASE;
    }
    SkAutoMaskFreeImage dstCleanup(dstM.fImage);

    tmpBitmap.setConfig(SkBitmap::kA8_Config, dstM.fBounds.width(),
                   dstM.fBounds.height(), dstM.fRowBytes);
    if (!tmpBitmap.allocPixels(allocator, NULL)) {
        // Allocation of pixels for alpha bitmap failed.
        SkDebugf("extractAlpha failed to allocate (%d,%d) alpha bitmap\n",
                tmpBitmap.width(), tmpBitmap.height());
        return false;
    }
    memcpy(tmpBitmap.getPixels(), dstM.fImage, dstM.computeImageSize());
    if (offset) {
        offset->set(dstM.fBounds.fLeft, dstM.fBounds.fTop);
    }
    SkDEBUGCODE(tmpBitmap.validate();)

    tmpBitmap.swap(*dst);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

enum {
    SERIALIZE_PIXELTYPE_NONE,
    SERIALIZE_PIXELTYPE_REF_DATA
};

void SkBitmap::flatten(SkFlattenableWriteBuffer& buffer) const {
    buffer.writeInt(fWidth);
    buffer.writeInt(fHeight);
    buffer.writeInt(fRowBytes);
    buffer.writeInt(fConfig);
    buffer.writeBool(this->isOpaque());

    if (fPixelRef) {
        if (fPixelRef->getFactory()) {
            buffer.writeInt(SERIALIZE_PIXELTYPE_REF_DATA);
            buffer.writeUInt(fPixelRefOffset);
            buffer.writeFlattenable(fPixelRef);
            return;
        }
        // if we get here, we can't record the pixels
        buffer.writeInt(SERIALIZE_PIXELTYPE_NONE);
    } else {
        buffer.writeInt(SERIALIZE_PIXELTYPE_NONE);
    }
}

void SkBitmap::unflatten(SkFlattenableReadBuffer& buffer) {
    this->reset();

    int width = buffer.readInt();
    int height = buffer.readInt();
    int rowBytes = buffer.readInt();
    int config = buffer.readInt();

    this->setConfig((Config)config, width, height, rowBytes);
    this->setIsOpaque(buffer.readBool());

    int reftype = buffer.readInt();
    switch (reftype) {
        case SERIALIZE_PIXELTYPE_REF_DATA: {
            size_t offset = buffer.readUInt();
            SkPixelRef* pr = buffer.readFlattenableT<SkPixelRef>();
            SkSafeUnref(this->setPixelRef(pr, offset));
            break;
        }
        case SERIALIZE_PIXELTYPE_NONE:
            break;
        default:
            SkDEBUGFAIL("unrecognized pixeltype in serialized data");
            sk_throw();
    }
}

///////////////////////////////////////////////////////////////////////////////

SkBitmap::RLEPixels::RLEPixels(int width, int height) {
    fHeight = height;
    fYPtrs = (uint8_t**)sk_malloc_throw(height * sizeof(uint8_t*));
    sk_bzero(fYPtrs, height * sizeof(uint8_t*));
}

SkBitmap::RLEPixels::~RLEPixels() {
    sk_free(fYPtrs);
}

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG
void SkBitmap::validate() const {
    SkASSERT(fConfig < kConfigCount);
    SkASSERT(fRowBytes >= (unsigned)ComputeRowBytes((Config)fConfig, fWidth));
    SkASSERT(fFlags <= (kImageIsOpaque_Flag | kImageIsVolatile_Flag | kImageIsImmutable_Flag));
    SkASSERT(fPixelLockCount >= 0);
    SkASSERT(NULL == fColorTable || (unsigned)fColorTable->getRefCnt() < 10000);
    SkASSERT((uint8_t)ComputeBytesPerPixel((Config)fConfig) == fBytesPerPixel);

#if 0   // these asserts are not thread-correct, so disable for now
    if (fPixelRef) {
        if (fPixelLockCount > 0) {
            SkASSERT(fPixelRef->isLocked());
        } else {
            SkASSERT(NULL == fPixels);
            SkASSERT(NULL == fColorTable);
        }
    }
#endif
}
#endif
