
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
#include "SkImagePriv.h"
#include "SkMallocPixelRef.h"
#include "SkMask.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkPixelRef.h"
#include "SkThread.h"
#include "SkUnPreMultiply.h"
#include "SkUtils.h"
#include "SkValidationUtils.h"
#include "SkPackBits.h"
#include <new>

static bool reset_return_false(SkBitmap* bm) {
    bm->reset();
    return false;
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
        int64_t size = (levelCount + 1) * sizeof(MipLevel);
        size += sizeof(MipMap) + pixelSize;
        if (!sk_64_isS32(size)) {
            return NULL;
        }
        MipMap* mm = (MipMap*)sk_malloc_throw(sk_64_asS32(size));
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

        if (fPixelRef) {
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
    SkTSwap(fPixelRefOrigin, other.fPixelRefOrigin);
    SkTSwap(fPixelLockCount, other.fPixelLockCount);
    SkTSwap(fMipMap, other.fMipMap);
    SkTSwap(fPixels, other.fPixels);
    SkTSwap(fInfo, other.fInfo);
    SkTSwap(fRowBytes, other.fRowBytes);
    SkTSwap(fFlags, other.fFlags);

    SkDEBUGCODE(this->validate();)
}

void SkBitmap::reset() {
    this->freePixels();
    sk_bzero(this, sizeof(*this));
}

SkBitmap::Config SkBitmap::config() const {
    return SkColorTypeToBitmapConfig(fInfo.colorType());
}

int SkBitmap::ComputeBytesPerPixel(SkBitmap::Config config) {
    int bpp;
    switch (config) {
        case kNo_Config:
            bpp = 0;   // not applicable
            break;
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

size_t SkBitmap::ComputeRowBytes(Config c, int width) {
    return SkColorTypeMinRowBytes(SkBitmapConfigToColorType(c), width);
}

int64_t SkBitmap::ComputeSize64(Config config, int width, int height) {
    SkColorType ct = SkBitmapConfigToColorType(config);
    int64_t rowBytes = sk_64_mul(SkColorTypeBytesPerPixel(ct), width);
    return rowBytes * height;
}

size_t SkBitmap::ComputeSize(Config c, int width, int height) {
    int64_t size = SkBitmap::ComputeSize64(c, width, height);
    return sk_64_isS32(size) ? sk_64_asS32(size) : 0;
}

int64_t SkBitmap::ComputeSafeSize64(Config config,
                                    uint32_t width,
                                    uint32_t height,
                                    size_t rowBytes) {
    SkImageInfo info = SkImageInfo::Make(width, height,
                                         SkBitmapConfigToColorType(config),
                                         kPremul_SkAlphaType);
    return info.getSafeSize64(rowBytes);
}

size_t SkBitmap::ComputeSafeSize(Config config,
                                 uint32_t width,
                                 uint32_t height,
                                 size_t rowBytes) {
    int64_t safeSize = ComputeSafeSize64(config, width, height, rowBytes);
    int32_t safeSize32 = (int32_t)safeSize;

    if (safeSize32 != safeSize) {
        safeSize32 = 0;
    }
    return safeSize32;
}

void SkBitmap::getBounds(SkRect* bounds) const {
    SkASSERT(bounds);
    bounds->set(0, 0,
                SkIntToScalar(fInfo.fWidth), SkIntToScalar(fInfo.fHeight));
}

void SkBitmap::getBounds(SkIRect* bounds) const {
    SkASSERT(bounds);
    bounds->set(0, 0, fInfo.fWidth, fInfo.fHeight);
}

///////////////////////////////////////////////////////////////////////////////

static bool validate_alphaType(SkColorType colorType, SkAlphaType alphaType,
                               SkAlphaType* canonical = NULL) {
    switch (colorType) {
        case kUnknown_SkColorType:
            alphaType = kIgnore_SkAlphaType;
            break;
        case kAlpha_8_SkColorType:
            if (kUnpremul_SkAlphaType == alphaType) {
                alphaType = kPremul_SkAlphaType;
            }
            // fall-through
        case kIndex_8_SkColorType:
        case kARGB_4444_SkColorType:
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
            if (kIgnore_SkAlphaType == alphaType) {
                return false;
            }
            break;
        case kRGB_565_SkColorType:
            alphaType = kOpaque_SkAlphaType;
            break;
        default:
            return false;
    }
    if (canonical) {
        *canonical = alphaType;
    }
    return true;
}

bool SkBitmap::setConfig(const SkImageInfo& origInfo, size_t rowBytes) {
    SkImageInfo info = origInfo;

    if (!validate_alphaType(info.fColorType, info.fAlphaType,
                            &info.fAlphaType)) {
        return reset_return_false(this);
    }

    // require that rowBytes fit in 31bits
    int64_t mrb = info.minRowBytes64();
    if ((int32_t)mrb != mrb) {
        return reset_return_false(this);
    }
    if ((int64_t)rowBytes != (int32_t)rowBytes) {
        return reset_return_false(this);
    }

    if (info.width() < 0 || info.height() < 0) {
        return reset_return_false(this);
    }

    if (kUnknown_SkColorType == info.colorType()) {
        rowBytes = 0;
    } else if (0 == rowBytes) {
        rowBytes = (size_t)mrb;
    } else if (rowBytes < info.minRowBytes()) {
        return reset_return_false(this);
    }

    this->freePixels();

    fInfo = info;
    fRowBytes = SkToU32(rowBytes);
    return true;
}

bool SkBitmap::setConfig(Config config, int width, int height, size_t rowBytes,
                         SkAlphaType alphaType) {
    SkColorType ct = SkBitmapConfigToColorType(config);
    return this->setConfig(SkImageInfo::Make(width, height, ct, alphaType),
                           rowBytes);
}

bool SkBitmap::setAlphaType(SkAlphaType alphaType) {
    if (!validate_alphaType(fInfo.fColorType, alphaType, &alphaType)) {
        return false;
    }
    if (fInfo.fAlphaType != alphaType) {
        fInfo.fAlphaType = alphaType;
        if (fPixelRef) {
            fPixelRef->changeAlphaType(alphaType);
        }
    }
    return true;
}

void SkBitmap::updatePixelsFromRef() const {
    if (NULL != fPixelRef) {
        if (fPixelLockCount > 0) {
            SkASSERT(fPixelRef->isLocked());

            void* p = fPixelRef->pixels();
            if (NULL != p) {
                p = (char*)p
                    + fPixelRefOrigin.fY * fRowBytes
                    + fPixelRefOrigin.fX * fInfo.bytesPerPixel();
            }
            fPixels = p;
            fColorTable = fPixelRef->colorTable();
        } else {
            SkASSERT(0 == fPixelLockCount);
            fPixels = NULL;
            fColorTable = NULL;
        }
    }
}

static bool config_to_colorType(SkBitmap::Config config, SkColorType* ctOut) {
    SkColorType ct;
    switch (config) {
        case SkBitmap::kA8_Config:
            ct = kAlpha_8_SkColorType;
            break;
        case SkBitmap::kIndex8_Config:
            ct = kIndex_8_SkColorType;
            break;
        case SkBitmap::kRGB_565_Config:
            ct = kRGB_565_SkColorType;
            break;
        case SkBitmap::kARGB_4444_Config:
            ct = kARGB_4444_SkColorType;
            break;
        case SkBitmap::kARGB_8888_Config:
            ct = kPMColor_SkColorType;
            break;
        case SkBitmap::kNo_Config:
        default:
            return false;
    }
    if (ctOut) {
        *ctOut = ct;
    }
    return true;
}

SkPixelRef* SkBitmap::setPixelRef(SkPixelRef* pr, int dx, int dy) {
#ifdef SK_DEBUG
    if (pr) {
        SkImageInfo info;
        if (this->asImageInfo(&info)) {
            const SkImageInfo& prInfo = pr->info();
            SkASSERT(info.fWidth <= prInfo.fWidth);
            SkASSERT(info.fHeight <= prInfo.fHeight);
            SkASSERT(info.fColorType == prInfo.fColorType);
            switch (prInfo.fAlphaType) {
                case kIgnore_SkAlphaType:
                    SkASSERT(fInfo.fAlphaType == kIgnore_SkAlphaType);
                    break;
                case kOpaque_SkAlphaType:
                case kPremul_SkAlphaType:
                    SkASSERT(info.fAlphaType == kOpaque_SkAlphaType ||
                             info.fAlphaType == kPremul_SkAlphaType);
                    break;
                case kUnpremul_SkAlphaType:
                    SkASSERT(info.fAlphaType == kOpaque_SkAlphaType ||
                             info.fAlphaType == kUnpremul_SkAlphaType);
                    break;
            }
        }
    }
#endif

    if (pr) {
        const SkImageInfo& info = pr->info();
        fPixelRefOrigin.set(SkPin32(dx, 0, info.fWidth),
                            SkPin32(dy, 0, info.fHeight));
    } else {
        // ignore dx,dy if there is no pixelref
        fPixelRefOrigin.setZero();
    }

    if (fPixelRef != pr) {
        if (fPixelRef != pr) {
            this->freePixels();
            SkASSERT(NULL == fPixelRef);

            SkSafeRef(pr);
            fPixelRef = pr;
        }
        this->updatePixelsFromRef();
    }

    SkDEBUGCODE(this->validate();)
    return pr;
}

void SkBitmap::lockPixels() const {
    if (NULL != fPixelRef && 0 == sk_atomic_inc(&fPixelLockCount)) {
        fPixelRef->lockPixels();
        this->updatePixelsFromRef();
    }
    SkDEBUGCODE(this->validate();)
}

void SkBitmap::unlockPixels() const {
    SkASSERT(NULL == fPixelRef || fPixelLockCount > 0);

    if (NULL != fPixelRef && 1 == sk_atomic_dec(&fPixelLockCount)) {
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
        this->setPixelRef(NULL);
        return;
    }

    SkImageInfo info;
    if (!this->asImageInfo(&info)) {
        this->setPixelRef(NULL);
        return;
    }

    SkPixelRef* pr = SkMallocPixelRef::NewDirect(info, p, fRowBytes, ctable);
    if (NULL == pr) {
        this->setPixelRef(NULL);
        return;
    }

    this->setPixelRef(pr)->unref();

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

///////////////////////////////////////////////////////////////////////////////

bool SkBitmap::allocPixels(const SkImageInfo& info, SkPixelRefFactory* factory,
                           SkColorTable* ctable) {
    if (kIndex_8_SkColorType == info.fColorType && NULL == ctable) {
        return reset_return_false(this);
    }
    if (!this->setConfig(info)) {
        return reset_return_false(this);
    }

    SkMallocPixelRef::PRFactory defaultFactory;
    if (NULL == factory) {
        factory = &defaultFactory;
    }

    SkPixelRef* pr = factory->create(info, ctable);
    if (NULL == pr) {
        return reset_return_false(this);
    }
    this->setPixelRef(pr)->unref();

    // TODO: lockPixels could/should return bool or void*/NULL
    this->lockPixels();
    if (NULL == this->getPixels()) {
        return reset_return_false(this);
    }
    return true;
}

bool SkBitmap::installPixels(const SkImageInfo& info, void* pixels, size_t rb,
                             void (*releaseProc)(void* addr, void* context),
                             void* context) {
    if (!this->setConfig(info, rb)) {
        this->reset();
        return false;
    }

    SkPixelRef* pr = SkMallocPixelRef::NewWithProc(info, rb, NULL, pixels,
                                                   releaseProc, context);
    if (!pr) {
        this->reset();
        return false;
    }

    this->setPixelRef(pr)->unref();

    // since we're already allocated, we lockPixels right away
    this->lockPixels();
    SkDEBUGCODE(this->validate();)
    return true;
}

bool SkBitmap::installMaskPixels(const SkMask& mask) {
    if (SkMask::kA8_Format != mask.fFormat) {
        this->reset();
        return false;
    }
    return this->installPixels(SkImageInfo::MakeA8(mask.fBounds.width(),
                                                   mask.fBounds.height()),
                               mask.fImage, mask.fRowBytes);
}

bool SkBitmap::allocConfigPixels(Config config, int width, int height,
                                 bool isOpaque) {
    SkColorType ct;
    if (!config_to_colorType(config, &ct)) {
        return false;
    }

    SkAlphaType at = isOpaque ? kOpaque_SkAlphaType : kPremul_SkAlphaType;
    return this->allocPixels(SkImageInfo::Make(width, height, ct, at));
}

///////////////////////////////////////////////////////////////////////////////

void SkBitmap::freePixels() {
    // if we're gonna free the pixels, we certainly need to free the mipmap
    this->freeMipMap();

    if (NULL != fPixelRef) {
        if (fPixelLockCount > 0) {
            fPixelRef->unlockPixels();
        }
        fPixelRef->unref();
        fPixelRef = NULL;
        fPixelRefOrigin.setZero();
    }
    fPixelLockCount = 0;
    fPixels = NULL;
    fColorTable = NULL;
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

GrTexture* SkBitmap::getTexture() const {
    return fPixelRef ? fPixelRef->getTexture() : NULL;
}

///////////////////////////////////////////////////////////////////////////////

/** We explicitly use the same allocator for our pixels that SkMask does,
 so that we can freely assign memory allocated by one class to the other.
 */
bool SkBitmap::HeapAllocator::allocPixelRef(SkBitmap* dst,
                                            SkColorTable* ctable) {
    SkImageInfo info;
    if (!dst->asImageInfo(&info)) {
//        SkDebugf("unsupported config for info %d\n", dst->config());
        return false;
    }

    SkPixelRef* pr = SkMallocPixelRef::NewAllocate(info, dst->rowBytes(),
                                                   ctable);
    if (NULL == pr) {
        return false;
    }

    dst->setPixelRef(pr)->unref();
    // since we're already allocated, we lockPixels right away
    dst->lockPixels();
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool SkBitmap::copyPixelsTo(void* const dst, size_t dstSize,
                            size_t dstRowBytes, bool preserveDstPad) const {

    if (0 == dstRowBytes) {
        dstRowBytes = fRowBytes;
    }

    if (dstRowBytes < fInfo.minRowBytes() ||
        dst == NULL || (getPixels() == NULL && pixelRef() == NULL)) {
        return false;
    }

    if (!preserveDstPad && static_cast<uint32_t>(dstRowBytes) == fRowBytes) {
        size_t safeSize = this->getSafeSize();
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
        if (fInfo.getSafeSize(dstRowBytes) > dstSize) {
            return false;
        } else {
            // Just copy what we need on each line.
            size_t rowBytes = fInfo.minRowBytes();
            SkAutoLockPixels lock(*this);
            const uint8_t* srcP = reinterpret_cast<const uint8_t*>(getPixels());
            uint8_t* dstP = reinterpret_cast<uint8_t*>(dst);
            for (int row = 0; row < fInfo.fHeight;
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
        switch (this->colorType()) {
            case kRGBA_8888_SkColorType:
            case kBGRA_8888_SkColorType:
                base += x << 2;
                break;
            case kARGB_4444_SkColorType:
            case kRGB_565_SkColorType:
                base += x << 1;
                break;
            case kAlpha_8_SkColorType:
            case kIndex_8_SkColorType:
                base += x;
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
        case kNo_Config:
        default:
            SkASSERT(false);
            return 0;
    }
    SkASSERT(false);  // Not reached.
    return 0;
}

bool SkBitmap::ComputeIsOpaque(const SkBitmap& bm) {
    SkAutoLockPixels alp(bm);
    if (!bm.getPixels()) {
        return false;
    }

    const int height = bm.height();
    const int width = bm.width();

    switch (bm.config()) {
        case SkBitmap::kA8_Config: {
            unsigned a = 0xFF;
            for (int y = 0; y < height; ++y) {
                const uint8_t* row = bm.getAddr8(0, y);
                for (int x = 0; x < width; ++x) {
                    a &= row[x];
                }
                if (0xFF != a) {
                    return false;
                }
            }
            return true;
        } break;
        case SkBitmap::kIndex8_Config: {
            SkAutoLockColors alc(bm);
            const SkPMColor* table = alc.colors();
            if (!table) {
                return false;
            }
            SkPMColor c = (SkPMColor)~0;
            for (int i = bm.getColorTable()->count() - 1; i >= 0; --i) {
                c &= table[i];
            }
            return 0xFF == SkGetPackedA32(c);
        } break;
        case SkBitmap::kRGB_565_Config:
            return true;
            break;
        case SkBitmap::kARGB_4444_Config: {
            unsigned c = 0xFFFF;
            for (int y = 0; y < height; ++y) {
                const SkPMColor16* row = bm.getAddr16(0, y);
                for (int x = 0; x < width; ++x) {
                    c &= row[x];
                }
                if (0xF != SkGetPackedA4444(c)) {
                    return false;
                }
            }
            return true;
        } break;
        case SkBitmap::kARGB_8888_Config: {
            SkPMColor c = (SkPMColor)~0;
            for (int y = 0; y < height; ++y) {
                const SkPMColor* row = bm.getAddr32(0, y);
                for (int x = 0; x < width; ++x) {
                    c &= row[x];
                }
                if (0xFF != SkGetPackedA32(c)) {
                    return false;
                }
            }
            return true;
        }
        default:
            break;
    }
    return false;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static uint16_t pack_8888_to_4444(unsigned a, unsigned r, unsigned g, unsigned b) {
    unsigned pixel = (SkA32To4444(a) << SK_A4444_SHIFT) |
                     (SkR32To4444(r) << SK_R4444_SHIFT) |
                     (SkG32To4444(g) << SK_G4444_SHIFT) |
                     (SkB32To4444(b) << SK_B4444_SHIFT);
    return SkToU16(pixel);
}

void SkBitmap::internalErase(const SkIRect& area,
                             U8CPU a, U8CPU r, U8CPU g, U8CPU b) const {
#ifdef SK_DEBUG
    SkDEBUGCODE(this->validate();)
    SkASSERT(!area.isEmpty());
    {
        SkIRect total = { 0, 0, this->width(), this->height() };
        SkASSERT(total.contains(area));
    }
#endif

    switch (fInfo.colorType()) {
        case kUnknown_SkColorType:
        case kIndex_8_SkColorType:
            return; // can't erase
        default:
            break;
    }

    SkAutoLockPixels alp(*this);
    // perform this check after the lock call
    if (!this->readyToDraw()) {
        return;
    }

    int height = area.height();
    const int width = area.width();
    const int rowBytes = fRowBytes;

    // make rgb premultiplied
    if (255 != a) {
        r = SkAlphaMul(r, a);
        g = SkAlphaMul(g, a);
        b = SkAlphaMul(b, a);
    }

    switch (this->colorType()) {
        case kAlpha_8_SkColorType: {
            uint8_t* p = this->getAddr8(area.fLeft, area.fTop);
            while (--height >= 0) {
                memset(p, a, width);
                p += rowBytes;
            }
            break;
        }
        case kARGB_4444_SkColorType:
        case kRGB_565_SkColorType: {
            uint16_t* p = this->getAddr16(area.fLeft, area.fTop);;
            uint16_t v;

            if (kARGB_4444_SkColorType == this->colorType()) {
                v = pack_8888_to_4444(a, r, g, b);
            } else {
                v = SkPackRGB16(r >> (8 - SK_R16_BITS),
                                g >> (8 - SK_G16_BITS),
                                b >> (8 - SK_B16_BITS));
            }
            while (--height >= 0) {
                sk_memset16(p, v, width);
                p = (uint16_t*)((char*)p + rowBytes);
            }
            break;
        }
        case kPMColor_SkColorType: {
            // what to do about BGRA or RGBA (which ever is != PMColor ?
            // for now we don't support them.
            uint32_t* p = this->getAddr32(area.fLeft, area.fTop);
            uint32_t  v = SkPackARGB32(a, r, g, b);

            while (--height >= 0) {
                sk_memset32(p, v, width);
                p = (uint32_t*)((char*)p + rowBytes);
            }
            break;
        }
        default:
            return; // no change, so don't call notifyPixelsChanged()
    }

    this->notifyPixelsChanged();
}

void SkBitmap::eraseARGB(U8CPU a, U8CPU r, U8CPU g, U8CPU b) const {
    SkIRect area = { 0, 0, this->width(), this->height() };
    if (!area.isEmpty()) {
        this->internalErase(area, a, r, g, b);
    }
}

void SkBitmap::eraseArea(const SkIRect& rect, SkColor c) const {
    SkIRect area = { 0, 0, this->width(), this->height() };
    if (area.intersect(rect)) {
        this->internalErase(area, SkColorGetA(c), SkColorGetR(c),
                            SkColorGetG(c), SkColorGetB(c));
    }
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

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

    if (fPixelRef->getTexture() != NULL) {
        // Do a deep copy
        SkPixelRef* pixelRef = fPixelRef->deepCopy(this->config(), &subset);
        if (pixelRef != NULL) {
            SkBitmap dst;
            dst.setConfig(this->config(), subset.width(), subset.height(), 0,
                          this->alphaType());
            dst.setIsVolatile(this->isVolatile());
            dst.setPixelRef(pixelRef)->unref();
            SkDEBUGCODE(dst.validate());
            result->swap(dst);
            return true;
        }
    }

    // If the upper left of the rectangle was outside the bounds of this SkBitmap, we should have
    // exited above.
    SkASSERT(static_cast<unsigned>(r.fLeft) < static_cast<unsigned>(this->width()));
    SkASSERT(static_cast<unsigned>(r.fTop) < static_cast<unsigned>(this->height()));

    SkBitmap dst;
    dst.setConfig(this->config(), r.width(), r.height(), this->rowBytes(),
                  this->alphaType());
    dst.setIsVolatile(this->isVolatile());

    if (fPixelRef) {
        SkIPoint origin = fPixelRefOrigin;
        origin.fX += r.fLeft;
        origin.fY += r.fTop;
        // share the pixelref with a custom offset
        dst.setPixelRef(fPixelRef, origin);
    }
    SkDEBUGCODE(dst.validate();)

    // we know we're good, so commit to result
    result->swap(dst);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

#include "SkCanvas.h"
#include "SkPaint.h"

bool SkBitmap::canCopyTo(SkColorType dstColorType) const {
    if (this->colorType() == kUnknown_SkColorType) {
        return false;
    }

    bool sameConfigs = (this->colorType() == dstColorType);
    switch (dstColorType) {
        case kAlpha_8_SkColorType:
        case kRGB_565_SkColorType:
        case kPMColor_SkColorType:
            break;
        case kIndex_8_SkColorType:
            if (!sameConfigs) {
                return false;
            }
            break;
        case kARGB_4444_SkColorType:
            return sameConfigs || kPMColor_SkColorType == this->colorType();
        default:
            return false;
    }
    return true;
}

bool SkBitmap::copyTo(SkBitmap* dst, SkColorType dstColorType,
                      Allocator* alloc) const {
    if (!this->canCopyTo(dstColorType)) {
        return false;
    }

    // if we have a texture, first get those pixels
    SkBitmap tmpSrc;
    const SkBitmap* src = this;

    if (fPixelRef) {
        SkIRect subset;
        subset.setXYWH(fPixelRefOrigin.fX, fPixelRefOrigin.fY,
                       fInfo.width(), fInfo.height());
        if (fPixelRef->readPixels(&tmpSrc, &subset)) {
            SkASSERT(tmpSrc.width() == this->width());
            SkASSERT(tmpSrc.height() == this->height());

            // did we get lucky and we can just return tmpSrc?
            if (tmpSrc.colorType() == dstColorType && NULL == alloc) {
                dst->swap(tmpSrc);
                // If the result is an exact copy, clone the gen ID.
                if (dst->pixelRef() && dst->pixelRef()->info() == fPixelRef->info()) {
                    dst->pixelRef()->cloneGenID(*fPixelRef);
                }
                return true;
            }

            // fall through to the raster case
            src = &tmpSrc;
        }
    }

    // we lock this now, since we may need its colortable
    SkAutoLockPixels srclock(*src);
    if (!src->readyToDraw()) {
        return false;
    }

    // The only way to be readyToDraw is if fPixelRef is non NULL.
    SkASSERT(fPixelRef != NULL);

    SkImageInfo dstInfo = src->info();
    dstInfo.fColorType = dstColorType;

    SkBitmap tmpDst;
    if (!tmpDst.setConfig(dstInfo)) {
        return false;
    }

    // allocate colortable if srcConfig == kIndex8_Config
    SkAutoTUnref<SkColorTable> ctable;
    if (dstColorType == kIndex_8_SkColorType) {
        // TODO: can we just ref() the src colortable? Is it reentrant-safe?
        ctable.reset(SkNEW_ARGS(SkColorTable, (*src->getColorTable())));
    }
    if (!tmpDst.allocPixels(alloc, ctable)) {
        return false;
    }

    if (!tmpDst.readyToDraw()) {
        // allocator/lock failed
        return false;
    }

    // pixelRef must be non NULL or tmpDst.readyToDraw() would have
    // returned false.
    SkASSERT(tmpDst.pixelRef() != NULL);

    /* do memcpy for the same configs cases, else use drawing
    */
    if (src->colorType() == dstColorType) {
        if (tmpDst.getSize() == src->getSize()) {
            memcpy(tmpDst.getPixels(), src->getPixels(), src->getSafeSize());
            SkPixelRef* pixelRef = tmpDst.pixelRef();

            // In order to reach this point, we know that the width, config and
            // rowbytes of the SkPixelRefs are the same, but it is possible for
            // the heights to differ, if this SkBitmap's height is a subset of
            // fPixelRef. Only if the SkPixelRefs' heights match are we
            // guaranteed that this is an exact copy, meaning we should clone
            // the genID.
            if (pixelRef->info().fHeight == fPixelRef->info().fHeight) {
                // TODO: what to do if the two infos match, BUT
                // fPixelRef is premul and pixelRef is opaque?
                // skipping assert for now
                // https://code.google.com/p/skia/issues/detail?id=2012
//                SkASSERT(pixelRef->info() == fPixelRef->info());
                SkASSERT(pixelRef->info().fWidth == fPixelRef->info().fWidth);
                SkASSERT(pixelRef->info().fColorType == fPixelRef->info().fColorType);
                pixelRef->cloneGenID(*fPixelRef);
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
    } else if (kARGB_4444_SkColorType == dstColorType
               && kPMColor_SkColorType == src->colorType()) {
        SkASSERT(src->height() == tmpDst.height());
        SkASSERT(src->width() == tmpDst.width());
        for (int y = 0; y < src->height(); ++y) {
            SkPMColor16* SK_RESTRICT dstRow = (SkPMColor16*) tmpDst.getAddr16(0, y);
            SkPMColor* SK_RESTRICT srcRow = (SkPMColor*) src->getAddr32(0, y);
            DITHER_4444_SCAN(y);
            for (int x = 0; x < src->width(); ++x) {
                dstRow[x] = SkDitherARGB32To4444(srcRow[x],
                                                 DITHER_VALUE(x));
            }
        }
    } else {
        // Always clear the dest in case one of the blitters accesses it
        // TODO: switch the allocation of tmpDst to call sk_calloc_throw
        tmpDst.eraseColor(SK_ColorTRANSPARENT);

        SkCanvas canvas(tmpDst);
        SkPaint  paint;

        paint.setDither(true);
        canvas.drawBitmap(*src, 0, 0, &paint);
    }

    dst->swap(tmpDst);
    return true;
}

bool SkBitmap::deepCopyTo(SkBitmap* dst) const {
    const SkBitmap::Config dstConfig = this->config();
    const SkColorType dstCT = SkBitmapConfigToColorType(dstConfig);

    if (!this->canCopyTo(dstCT)) {
        return false;
    }

    // If we have a PixelRef, and it supports deep copy, use it.
    // Currently supported only by texture-backed bitmaps.
    if (fPixelRef) {
        SkPixelRef* pixelRef = fPixelRef->deepCopy(dstConfig);
        if (pixelRef) {
            uint32_t rowBytes;
            if (this->colorType() == dstCT) {
                // Since there is no subset to pass to deepCopy, and deepCopy
                // succeeded, the new pixel ref must be identical.
                SkASSERT(fPixelRef->info() == pixelRef->info());
                pixelRef->cloneGenID(*fPixelRef);
                // Use the same rowBytes as the original.
                rowBytes = fRowBytes;
            } else {
                // With the new config, an appropriate fRowBytes will be computed by setConfig.
                rowBytes = 0;
            }

            SkImageInfo info = fInfo;
            info.fColorType = dstCT;
            if (!dst->setConfig(info, rowBytes)) {
                return false;
            }
            dst->setPixelRef(pixelRef, fPixelRefOrigin)->unref();
            return true;
        }
    }

    if (this->getTexture()) {
        return false;
    } else {
        return this->copyTo(dst, dstCT, NULL);
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

    const SkBitmap::Config config = this->config();

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
    uint32_t    rowBytes;
    SkBitmap    dstBM;

    for (int i = 0; i < maxLevels; i++) {
        width >>= 1;
        height >>= 1;
        rowBytes = SkToU32(ComputeRowBytes(config, width));

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

    SkBitmap::Config config = src.config();
    int              w = src.width();
    int              h = src.height();
    size_t           rb = src.rowBytes();

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
            ct->unlockColors();
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

void SkBitmap::flatten(SkWriteBuffer& buffer) const {
    fInfo.flatten(buffer);
    buffer.writeInt(fRowBytes);

    if (fPixelRef) {
        if (fPixelRef->getFactory()) {
            buffer.writeInt(SERIALIZE_PIXELTYPE_REF_DATA);
            buffer.writeInt(fPixelRefOrigin.fX);
            buffer.writeInt(fPixelRefOrigin.fY);
            buffer.writeFlattenable(fPixelRef);
            return;
        }
        // if we get here, we can't record the pixels
        buffer.writeInt(SERIALIZE_PIXELTYPE_NONE);
    } else {
        buffer.writeInt(SERIALIZE_PIXELTYPE_NONE);
    }
}

void SkBitmap::unflatten(SkReadBuffer& buffer) {
    this->reset();

    SkImageInfo info;
    info.unflatten(buffer);
    size_t rowBytes = buffer.readInt();
    if (!buffer.validate((info.width() >= 0) && (info.height() >= 0) &&
                         SkColorTypeIsValid(info.fColorType) &&
                         SkAlphaTypeIsValid(info.fAlphaType) &&
                         validate_alphaType(info.fColorType, info.fAlphaType) &&
                         info.validRowBytes(rowBytes))) {
        return;
    }

    bool configIsValid = this->setConfig(info, rowBytes);
    buffer.validate(configIsValid);

    int reftype = buffer.readInt();
    if (buffer.validate((SERIALIZE_PIXELTYPE_REF_DATA == reftype) ||
                        (SERIALIZE_PIXELTYPE_NONE == reftype))) {
        switch (reftype) {
            case SERIALIZE_PIXELTYPE_REF_DATA: {
                SkIPoint origin;
                origin.fX = buffer.readInt();
                origin.fY = buffer.readInt();
                size_t offset = origin.fY * rowBytes + origin.fX * info.bytesPerPixel();
                SkPixelRef* pr = buffer.readPixelRef();
                if (!buffer.validate((NULL == pr) ||
                       (pr->getAllocatedSizeInBytes() >= (offset + this->getSafeSize())))) {
                    origin.setZero();
                }
                SkSafeUnref(this->setPixelRef(pr, origin));
                break;
            }
            case SERIALIZE_PIXELTYPE_NONE:
                break;
            default:
                SkDEBUGFAIL("unrecognized pixeltype in serialized data");
                sk_throw();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

SkBitmap::RLEPixels::RLEPixels(int width, int height) {
    fHeight = height;
    fYPtrs = (uint8_t**)sk_calloc_throw(height * sizeof(uint8_t*));
}

SkBitmap::RLEPixels::~RLEPixels() {
    sk_free(fYPtrs);
}

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG
void SkBitmap::validate() const {
    fInfo.validate();

    // ImageInfo may not require this, but Bitmap ensures that opaque-only
    // colorTypes report opaque for their alphatype
    if (kRGB_565_SkColorType == fInfo.colorType()) {
        SkASSERT(kOpaque_SkAlphaType == fInfo.alphaType());
    }

    SkASSERT(fInfo.validRowBytes(fRowBytes));
    uint8_t allFlags = kImageIsOpaque_Flag | kImageIsVolatile_Flag | kImageIsImmutable_Flag;
#ifdef SK_BUILD_FOR_ANDROID
    allFlags |= kHasHardwareMipMap_Flag;
#endif
    SkASSERT(fFlags <= allFlags);
    SkASSERT(fPixelLockCount >= 0);

    if (fPixels) {
        SkASSERT(fPixelRef);
        SkASSERT(fPixelLockCount > 0);
        SkASSERT(fPixelRef->isLocked());
        SkASSERT(fPixelRef->rowBytes() == fRowBytes);
        SkASSERT(fPixelRefOrigin.fX >= 0);
        SkASSERT(fPixelRefOrigin.fY >= 0);
        SkASSERT(fPixelRef->info().width() >= (int)this->width() + fPixelRefOrigin.fX);
        SkASSERT(fPixelRef->info().fHeight >= (int)this->height() + fPixelRefOrigin.fY);
        SkASSERT(fPixelRef->rowBytes() >= fInfo.minRowBytes());
    } else {
        SkASSERT(NULL == fColorTable);
    }
}
#endif

#ifndef SK_IGNORE_TO_STRING
void SkBitmap::toString(SkString* str) const {

    static const char* gConfigNames[kConfigCount] = {
        "NONE", "A8", "INDEX8", "565", "4444", "8888"
    };

    str->appendf("bitmap: ((%d, %d) %s", this->width(), this->height(),
                 gConfigNames[this->config()]);

    str->append(" (");
    if (this->isOpaque()) {
        str->append("opaque");
    } else {
        str->append("transparent");
    }
    if (this->isImmutable()) {
        str->append(", immutable");
    } else {
        str->append(", not-immutable");
    }
    str->append(")");

    SkPixelRef* pr = this->pixelRef();
    if (NULL == pr) {
        // show null or the explicit pixel address (rare)
        str->appendf(" pixels:%p", this->getPixels());
    } else {
        const char* uri = pr->getURI();
        if (NULL != uri) {
            str->appendf(" uri:\"%s\"", uri);
        } else {
            str->appendf(" pixelref:%p", pr);
        }
    }

    str->append(")");
}
#endif

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG
void SkImageInfo::validate() const {
    SkASSERT(fWidth >= 0);
    SkASSERT(fHeight >= 0);
    SkASSERT(SkColorTypeIsValid(fColorType));
    SkASSERT(SkAlphaTypeIsValid(fAlphaType));
}
#endif
