/*
 * Copyright (C) 2006-2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "SkBitmap.h"
#include "SkColorPriv.h"
#include "SkDither.h"
#include "SkFlattenable.h"
#include "SkMallocPixelRef.h"
#include "SkMask.h"
#include "SkPixelRef.h"
#include "SkThread.h"
#include "SkUtils.h"
#include "SkPackBits.h"
#include <new>

#ifdef SK_SUPPORT_MIPMAP
struct MipLevel {
    void*       fPixels;
    uint32_t    fRowBytes;
    uint16_t    fWidth, fHeight;
};

struct SkBitmap::MipMap : SkNoncopyable {
    int32_t fRefCnt;
    int     fLevelCount;
//  MipLevel    fLevel[fLevelCount];
//  Pixels[]
    
    static MipMap* Alloc(int levelCount, size_t pixelSize) {
        MipMap* mm = (MipMap*)sk_malloc_throw(sizeof(MipMap) +
                                              levelCount * sizeof(MipLevel) +
                                              pixelSize);
        mm->fRefCnt = 1;
        mm->fLevelCount = levelCount;
        return mm;
    }

    const MipLevel* levels() const { return (const MipLevel*)(this + 1); }
    MipLevel* levels() { return (MipLevel*)(this + 1); }

    const void* pixels() const { return levels() + fLevelCount; }
    void* pixels() { return levels() + fLevelCount; }
    
    void safeRef() {
        if (this) {
            SkASSERT(fRefCnt > 0);
            sk_atomic_inc(&fRefCnt);
        }
    }
    void safeUnref() {
        if (this) {
            SkASSERT(fRefCnt > 0);
            if (sk_atomic_dec(&fRefCnt) == 1) {
                sk_free(this);
            }
        }
    }
};
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

SkBitmap::SkBitmap() {
    bzero(this, sizeof(*this));
}

SkBitmap::SkBitmap(const SkBitmap& src) {
    SkDEBUGCODE(src.validate();)
    bzero(this, sizeof(*this));
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
        src.fPixelRef->safeRef();
#ifdef SK_SUPPORT_MIPMAP
        src.fMipMap->safeRef();
#endif

        // we reset our locks if we get blown away
        fPixelLockCount = 0;
        
        /*  The src could be in 3 states
            1. no pixelref, in which case we just copy/ref the pixels/ctable
            2. unlocked pixelref, pixels/ctable should be null
            3. locked pixelref, we should lock the ref again ourselves
        */
        if (NULL == fPixelRef) {
            // leave fPixels as it is
            fColorTable->safeRef(); // ref the user's ctable if present
        } else {    // we have a pixelref, so pixels/ctable reflect it
            // ignore the values from the memcpy
            fPixels = NULL;
            fColorTable = NULL;
        }
    }

    SkDEBUGCODE(this->validate();)
    return *this;
}

void SkBitmap::swap(SkBitmap& other) {
    SkTSwap<SkColorTable*>(fColorTable, other.fColorTable);
    SkTSwap<SkPixelRef*>(fPixelRef, other.fPixelRef);
    SkTSwap<size_t>(fPixelRefOffset, other.fPixelRefOffset);
    SkTSwap<int>(fPixelLockCount, other.fPixelLockCount);
#ifdef SK_SUPPORT_MIPMAP
    SkTSwap<MipMap*>(fMipMap, other.fMipMap);
#endif
    SkTSwap<void*>(fPixels, other.fPixels);
    SkTSwap<uint16_t>(fWidth, other.fWidth);
    SkTSwap<uint16_t>(fHeight, other.fHeight);
    SkTSwap<uint32_t>(fRowBytes, other.fRowBytes);
    SkTSwap<uint8_t>(fConfig, other.fConfig);
    SkTSwap<uint8_t>(fFlags, other.fFlags);
    SkTSwap<uint8_t>(fBytesPerPixel, other.fBytesPerPixel);

    SkDEBUGCODE(this->validate();)
}

void SkBitmap::reset() {
    this->freePixels();
    bzero(this, sizeof(*this));
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
            SkASSERT(!"unknown config");
            bpp = 0;   // error
            break;
    }
    return bpp;
}

int SkBitmap::ComputeRowBytes(Config c, int width) {
    int rowBytes = 0;

    switch (c) {
        case kNo_Config:
        case kRLE_Index8_Config:
            // assume that the bitmap has no pixels to draw to
            rowBytes = 0;
            break;
        case kA1_Config:
            rowBytes = (width + 7) >> 3;
            break;
        case kA8_Config:
        case kIndex8_Config:
            rowBytes = width;
            break;
        case kRGB_565_Config:
        case kARGB_4444_Config:
            rowBytes = width << 1;
            break;
        case kARGB_8888_Config:
            rowBytes = width << 2;
            break;
        default:
            SkASSERT(!"unknown config");
            break;
    }
    return rowBytes;
}

Sk64 SkBitmap::ComputeSize64(Config c, int width, int height) {
    Sk64 size;
    size.setMul(SkBitmap::ComputeRowBytes(c, width), height);
    return size;
}

size_t SkBitmap::ComputeSize(Config c, int width, int height) {
    Sk64 size = SkBitmap::ComputeSize64(c, width, height);
    if (size.isNeg() || !size.is32()) {
        return 0;
    }
    return size.get32();
}

void SkBitmap::setConfig(Config c, int width, int height, int rowBytes) {
    this->freePixels();

    if (rowBytes == 0) {
        rowBytes = SkBitmap::ComputeRowBytes(c, width);
    }
    fConfig     = SkToU8(c);
    fWidth      = SkToU16(width);
    fHeight     = SkToU16(height);
    fRowBytes   = rowBytes;

    fBytesPerPixel = (uint8_t)ComputeBytesPerPixel(c);

    SkDEBUGCODE(this->validate();)
}

void SkBitmap::updatePixelsFromRef() const {
    if (NULL != fPixelRef) {
        if (fPixelLockCount > 0) {
            SkASSERT(fPixelRef->getLockCount() > 0);
            
            void* p = fPixelRef->pixels();
            if (NULL != p) {
                p = (char*)p + fPixelRefOffset;
            }
            fPixels = p;
            SkRefCnt_SafeAssign(fColorTable, fPixelRef->colorTable());
        } else {
            SkASSERT(0 == fPixelLockCount);
            fPixels = NULL;
            fColorTable->safeUnref();
            fColorTable = NULL;
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
            
            pr->safeRef();
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

void SkBitmap::setPixels(void* p, SkColorTable* ctable) {
    this->freePixels();
    fPixels = p;
    SkRefCnt_SafeAssign(fColorTable, ctable);

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

    fColorTable->safeUnref();
    fColorTable = NULL;

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
#ifdef SK_SUPPORT_MIPMAP
    fMipMap->safeUnref();
    fMipMap = NULL;
#endif
}

uint32_t SkBitmap::getGenerationID() const {
    return fPixelRef ? fPixelRef->getGenerationID() : 0;
}

void SkBitmap::notifyPixelsChanged() const {
    if (fPixelRef) {
        fPixelRef->notifyPixelsChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////

SkMallocPixelRef::SkMallocPixelRef(void* storage, size_t size,
                                   SkColorTable* ctable) {
    SkASSERT(storage);
    fStorage = storage;
    fSize = size;
    fCTable = ctable;
    ctable->safeRef();
}

SkMallocPixelRef::~SkMallocPixelRef() {
    fCTable->safeUnref();
    sk_free(fStorage);
}

void* SkMallocPixelRef::onLockPixels(SkColorTable** ct) {
    *ct = fCTable;
    return fStorage;
}

void SkMallocPixelRef::onUnlockPixels() {
    // nothing to do
}

void SkMallocPixelRef::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    
    buffer.write32(fSize);
    buffer.writePad(fStorage, fSize);
    if (fCTable) {
        buffer.writeBool(true);
        fCTable->flatten(buffer);
    } else {
        buffer.writeBool(false);
    }
}

SkMallocPixelRef::SkMallocPixelRef(SkFlattenableReadBuffer& buffer) : INHERITED(buffer, NULL) {
    fSize = buffer.readU32();
    fStorage = sk_malloc_throw(fSize);
    buffer.read(fStorage, fSize);
    if (buffer.readBool()) {
        fCTable = SkNEW_ARGS(SkColorTable, (buffer));
    } else {
        fCTable = NULL;
    }
}

static SkPixelRef::Registrar reg("SkMallocPixelRef",
                                 SkMallocPixelRef::Create);

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
            SkASSERT(!"unknown bitmap config pased to isOpaque");
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
                SkASSERT(!"Can't return addr for kRLE_Index8_Config");
                base = NULL;
                break;
            default:
                SkASSERT(!"Can't return addr for config");
                base = NULL;
                break;
        }
    }
    return base;
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

    if (NULL == result || (NULL == fPixelRef && NULL == fPixels)) {
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

    if (fPixelRef) {
        // share the pixelref with a custom offset
        dst.setPixelRef(fPixelRef, fPixelRefOffset + offset);
    } else {
        // share the pixels (owned by the caller)
        dst.setPixels((char*)fPixels + offset, this->getColorTable());
    }
    SkDEBUGCODE(dst.validate();)

    // we know we're good, so commit to result
    result->swap(dst);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

#include "SkCanvas.h"
#include "SkPaint.h"

bool SkBitmap::copyTo(SkBitmap* dst, Config dstConfig, Allocator* alloc) const {
    if (NULL == dst || this->width() == 0 || this->height() == 0) {
        return false;
    }

    switch (dstConfig) {
        case kA8_Config:
        case kARGB_4444_Config:
        case kRGB_565_Config:
        case kARGB_8888_Config:
            break;
        default:
            return false;
    }
    
    SkBitmap    tmp;
    
    tmp.setConfig(dstConfig, this->width(), this->height());
    // pass null for colortable, since we don't support Index8 config for dst
    if (!tmp.allocPixels(alloc, NULL)) {
        return false;
    }
    
    SkAutoLockPixels srclock(*this);
    SkAutoLockPixels dstlock(tmp);
    
    if (!this->readyToDraw() || !tmp.readyToDraw()) {
        // allocator/lock failed
        return false;
    }

    // if the src has alpha, we have to clear the dst first
    if (!this->isOpaque()) {
        tmp.eraseColor(0);
    }

    SkCanvas canvas(tmp);
    SkPaint  paint;
    
    paint.setDither(true);
    canvas.drawBitmap(*this, 0, 0, &paint);
    
    dst->swap(tmp);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void downsampleby2_proc32(SkBitmap* dst, int x, int y,
                                 const SkBitmap& src) {
    x <<= 1;
    y <<= 1;
    const SkPMColor* p = src.getAddr32(x, y);
    SkPMColor c, ag, rb;

    c = *p; ag = (c >> 8) & 0xFF00FF; rb = c & 0xFF00FF;
    if (x < src.width() - 1) {
        p += 1;
    }
    c = *p; ag += (c >> 8) & 0xFF00FF; rb += c & 0xFF00FF;

    if (y < src.height() - 1) {
        p = src.getAddr32(x, y + 1);
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
    SkPMColor       c;
    
    c = expand16(*p);
    if (x < (int)src.width() - 1) {
        p += 1;
    }
    c += expand16(*p);
    
    if (y < (int)src.height() - 1) {
        p = src.getAddr16(x, y + 1);
    }
    c += expand16(*p);
    if (x < (int)src.width() - 1) {
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
    uint32_t        c;
    
    c = expand4444(*p);
    if (x < src.width() - 1) {
        p += 1;
    }
    c += expand4444(*p);
    
    if (y < src.height() - 1) {
        p = src.getAddr16(x, y + 1);
    }
    c += expand4444(*p);
    if (x < src.width() - 1) {
        p += 1;
    }
    c += expand4444(*p);
    
    *dst->getAddr16(x >> 1, y >> 1) = (uint16_t)collaps4444(c >> 2);
}

void SkBitmap::buildMipMap(bool forceRebuild) {
#ifdef SK_SUPPORT_MIPMAP
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

    // whip through our loop to compute the exact size needed
    size_t  size = 0;
    int     maxLevels = 0;
    {
        unsigned    width = this->width();
        unsigned    height = this->height();
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
    if (0 == maxLevels) {
        return;
    }

    MipMap*     mm = MipMap::Alloc(maxLevels, size);
    MipLevel*   level = mm->levels();
    uint8_t*    addr = (uint8_t*)mm->pixels();

    unsigned    width = this->width();
    unsigned    height = this->height();
    unsigned    rowBytes = this->rowBytes();
    SkBitmap    srcBM(*this), dstBM;

    srcBM.lockPixels();

    for (int i = 0; i < maxLevels; i++) {
        width >>= 1;
        height >>= 1;
        rowBytes = ComputeRowBytes(config, width);

        level[i].fPixels   = addr;
        level[i].fWidth    = SkToU16(width);
        level[i].fHeight   = SkToU16(height);
        level[i].fRowBytes = SkToU16(rowBytes);

        dstBM.setConfig(config, width, height, rowBytes);
        dstBM.setPixels(addr);
    
        for (unsigned y = 0; y < height; y++) {
            for (unsigned x = 0; x < width; x++) {
                proc(&dstBM, x, y, srcBM);
            }
        }

        srcBM = dstBM;
        addr += height * rowBytes;
    }
    SkASSERT(addr == (uint8_t*)mm->pixels() + size);
    fMipMap = mm;
#endif
}

bool SkBitmap::hasMipMap() const {
#ifdef SK_SUPPORT_MIPMAP
    return fMipMap != NULL;
#else
    return false;
#endif
}

int SkBitmap::extractMipLevel(SkBitmap* dst, SkFixed sx, SkFixed sy) {
#ifdef SK_SUPPORT_MIPMAP
    if (NULL == fMipMap)
        return 0;
    
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
#else
    return 0;
#endif
}

SkFixed SkBitmap::ComputeMipLevel(SkFixed sx, SkFixed sy) {
#ifdef SK_SUPPORT_MIPMAP
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
#else
    return 0;
#endif
}

///////////////////////////////////////////////////////////////////////////////

static void GetBitmapAlpha(const SkBitmap& src, uint8_t SK_RESTRICT alpha[],
                           int alphaRowBytes) {
    SkASSERT(alpha != NULL);
    SkASSERT(alphaRowBytes >= src.width());

    SkBitmap::Config config = src.getConfig();
    int              w = src.width();
    int              h = src.height();
    int              rb = src.rowBytes();

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
}

#include "SkPaint.h"
#include "SkMaskFilter.h"
#include "SkMatrix.h"

void SkBitmap::extractAlpha(SkBitmap* dst, const SkPaint* paint,
                            SkIPoint* offset) const {
    SkDEBUGCODE(this->validate();)

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
        dst->setConfig(SkBitmap::kA8_Config, this->width(), this->height(),
                       srcM.fRowBytes);
        dst->allocPixels();        
        GetBitmapAlpha(*this, dst->getAddr8(0, 0), srcM.fRowBytes);
        if (offset) {
            offset->set(0, 0);
        }
        return;
    }

    SkAutoMaskImage srcCleanup(&srcM, true);

    GetBitmapAlpha(*this, srcM.fImage, srcM.fRowBytes);
    if (!filter->filterMask(&dstM, srcM, identity, NULL)) {
        goto NO_FILTER_CASE;
    }

    SkAutoMaskImage dstCleanup(&dstM, false);

    dst->setConfig(SkBitmap::kA8_Config, dstM.fBounds.width(),
                   dstM.fBounds.height(), dstM.fRowBytes);
    dst->allocPixels();
    memcpy(dst->getPixels(), dstM.fImage, dstM.computeImageSize());
    if (offset) {
        offset->set(dstM.fBounds.fLeft, dstM.fBounds.fTop);
    }
    SkDEBUGCODE(dst->validate();)
}

///////////////////////////////////////////////////////////////////////////////

enum {
    SERIALIZE_PIXELTYPE_NONE,
    SERIALIZE_PIXELTYPE_RAW_WITH_CTABLE,
    SERIALIZE_PIXELTYPE_RAW_NO_CTABLE,
    SERIALIZE_PIXELTYPE_REF_DATA,
    SERIALIZE_PIXELTYPE_REF_PTR,
};

static void writeString(SkFlattenableWriteBuffer& buffer, const char str[]) {
    size_t len = strlen(str);
    buffer.write32(len);
    buffer.writePad(str, len);
}

static SkPixelRef::Factory deserialize_factory(SkFlattenableReadBuffer& buffer) {
    size_t len = buffer.readInt();
    SkAutoSMalloc<256> storage(len + 1);
    char* str = (char*)storage.get();
    buffer.read(str, len);
    str[len] = 0;
    return SkPixelRef::NameToFactory(str);
}

/*
    It is tricky to know how much to flatten. If we don't have a pixelref (i.e.
    we just have pixels, then we can only flatten the pixels, or write out an
    empty bitmap.
 
    With a pixelref, we still have the question of recognizing when two sitings
    of the same pixelref are the same, and when they are different. Perhaps we
    should look at the generationID and keep a record of that in some dictionary
    associated with the buffer. SkGLTextureCache does this sort of thing to know
    when to create a new texture.
*/
void SkBitmap::flatten(SkFlattenableWriteBuffer& buffer) const {
    buffer.write32(fWidth);
    buffer.write32(fHeight);
    buffer.write32(fRowBytes);
    buffer.write8(fConfig);
    buffer.writeBool(this->isOpaque());
    
    /*  If we are called in this mode, then it is up to the caller to manage
        the owner-counts on the pixelref, as we just record the ptr itself.
    */
    if (!buffer.persistBitmapPixels()) {
        if (fPixelRef) {
            buffer.write8(SERIALIZE_PIXELTYPE_REF_PTR);
            buffer.write32(fPixelRefOffset);
            buffer.writeRefCnt(fPixelRef);
            return;
        } else {
            // we ignore the non-persist request, since we don't have a ref
            // ... or we could just write an empty bitmap...
            // (true) will write an empty bitmap, (false) will flatten the pix
            if (true) {
                buffer.write8(SERIALIZE_PIXELTYPE_NONE);
                return;
            }
        }
    }

    if (fPixelRef) {
        SkPixelRef::Factory fact = fPixelRef->getFactory();
        if (fact) {
            const char* name = SkPixelRef::FactoryToName(fact);
            if (name && *name) {
                buffer.write8(SERIALIZE_PIXELTYPE_REF_DATA);
                buffer.write32(fPixelRefOffset);
                writeString(buffer, name);
                fPixelRef->flatten(buffer);
                return;
            }
        }
        // if we get here, we can't record the pixels
        buffer.write8(SERIALIZE_PIXELTYPE_NONE);
    } else if (fPixels) {
        if (fColorTable) {
            buffer.write8(SERIALIZE_PIXELTYPE_RAW_WITH_CTABLE);
            fColorTable->flatten(buffer);
        } else {
            buffer.write8(SERIALIZE_PIXELTYPE_RAW_NO_CTABLE);
        }
        buffer.writePad(fPixels, this->getSize());
    } else {
        buffer.write8(SERIALIZE_PIXELTYPE_NONE);
    }
}

void SkBitmap::unflatten(SkFlattenableReadBuffer& buffer) {
    this->reset();
    
    int width = buffer.readInt();
    int height = buffer.readInt();
    int rowBytes = buffer.readInt();
    int config = buffer.readU8();
    
    this->setConfig((Config)config, width, height, rowBytes);
    this->setIsOpaque(buffer.readBool());
    
    size_t size = this->getSize();
    int reftype = buffer.readU8();
    switch (reftype) {
        case SERIALIZE_PIXELTYPE_REF_PTR: {
            size_t offset = buffer.readU32();
            SkPixelRef* pr = (SkPixelRef*)buffer.readRefCnt();
            this->setPixelRef(pr, offset);
            break;
        }
        case SERIALIZE_PIXELTYPE_REF_DATA: {
            size_t offset = buffer.readU32();
            SkPixelRef::Factory fact = deserialize_factory(buffer);
            SkPixelRef* pr = fact(buffer);
            this->setPixelRef(pr, offset)->safeUnref();
            break;
        }
        case SERIALIZE_PIXELTYPE_RAW_WITH_CTABLE:
        case SERIALIZE_PIXELTYPE_RAW_NO_CTABLE: {
            SkColorTable* ctable = NULL;
            if (SERIALIZE_PIXELTYPE_RAW_WITH_CTABLE == reftype) {
                ctable = SkNEW_ARGS(SkColorTable, (buffer));
            }
            if (this->allocPixels(ctable)) {
                this->lockPixels();
                buffer.read(this->getPixels(), size);
                this->unlockPixels();
            } else {
                buffer.skip(size);
            }
            ctable->safeUnref();
            break;
        }
        case SERIALIZE_PIXELTYPE_NONE:
            break;
        default:
            SkASSERT(!"unrecognized pixeltype in serialized data");
            sk_throw();
    }
}

///////////////////////////////////////////////////////////////////////////////

SkBitmap::RLEPixels::RLEPixels(int width, int height) {
    fHeight = height;
    fYPtrs = (uint8_t**)sk_malloc_throw(height * sizeof(uint8_t*));
    bzero(fYPtrs, height * sizeof(uint8_t*));
}

SkBitmap::RLEPixels::~RLEPixels() {
    sk_free(fYPtrs);
}

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG
void SkBitmap::validate() const {
    SkASSERT(fConfig < kConfigCount);
    SkASSERT(fRowBytes >= (unsigned)ComputeRowBytes((Config)fConfig, fWidth));
    SkASSERT(fFlags <= kImageIsOpaque_Flag);
    SkASSERT(fPixelLockCount >= 0);
    SkASSERT(NULL == fColorTable || (unsigned)fColorTable->getRefCnt() < 10000);
    SkASSERT((uint8_t)ComputeBytesPerPixel((Config)fConfig) == fBytesPerPixel);

#if 0   // these asserts are not thread-correct, so disable for now
    if (fPixelRef) {
        if (fPixelLockCount > 0) {
            SkASSERT(fPixelRef->getLockCount() > 0);
        } else {
            SkASSERT(NULL == fPixels);
            SkASSERT(NULL == fColorTable);
        }
    }
#endif
}
#endif

