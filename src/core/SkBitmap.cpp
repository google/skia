
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

#ifdef SK_SUPPORT_LEGACY_BITMAP_CONFIG
SkBitmap::Config SkBitmap::config() const {
    return SkColorTypeToBitmapConfig(fInfo.colorType());
}
#endif

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

bool SkBitmap::setInfo(const SkImageInfo& origInfo, size_t rowBytes) {
    SkImageInfo info = origInfo;

    if (!SkColorTypeValidateAlphaType(info.fColorType, info.fAlphaType,
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
    } else if (!info.validRowBytes(rowBytes)) {
        return reset_return_false(this);
    }

    this->freePixels();

    fInfo = info;
    fRowBytes = SkToU32(rowBytes);
    return true;
}

bool SkBitmap::setAlphaType(SkAlphaType alphaType) {
    if (!SkColorTypeValidateAlphaType(fInfo.fColorType, alphaType, &alphaType)) {
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

SkPixelRef* SkBitmap::setPixelRef(SkPixelRef* pr, int dx, int dy) {
#ifdef SK_DEBUG
    if (pr) {
        if (kUnknown_SkColorType != fInfo.colorType()) {
            const SkImageInfo& prInfo = pr->info();
            SkASSERT(fInfo.fWidth <= prInfo.fWidth);
            SkASSERT(fInfo.fHeight <= prInfo.fHeight);
            SkASSERT(fInfo.fColorType == prInfo.fColorType);
            switch (prInfo.fAlphaType) {
                case kIgnore_SkAlphaType:
                    SkASSERT(fInfo.fAlphaType == kIgnore_SkAlphaType);
                    break;
                case kOpaque_SkAlphaType:
                case kPremul_SkAlphaType:
                    SkASSERT(fInfo.fAlphaType == kOpaque_SkAlphaType ||
                             fInfo.fAlphaType == kPremul_SkAlphaType);
                    break;
                case kUnpremul_SkAlphaType:
                    SkASSERT(fInfo.fAlphaType == kOpaque_SkAlphaType ||
                             fInfo.fAlphaType == kUnpremul_SkAlphaType);
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
        this->freePixels();
        SkASSERT(NULL == fPixelRef);

        SkSafeRef(pr);
        fPixelRef = pr;
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

    if (kUnknown_SkColorType == fInfo.colorType()) {
        this->setPixelRef(NULL);
        return;
    }

    SkPixelRef* pr = SkMallocPixelRef::NewDirect(fInfo, p, fRowBytes, ctable);
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

bool SkBitmap::allocPixels(const SkImageInfo& requestedInfo, size_t rowBytes) {
    if (kIndex_8_SkColorType == requestedInfo.colorType()) {
        return reset_return_false(this);
    }
    if (!this->setInfo(requestedInfo, rowBytes)) {
        return reset_return_false(this);
    }
    
    // setInfo may have corrected info (e.g. 565 is always opaque).
    const SkImageInfo& correctedInfo = this->info();
    // setInfo may have computed a valid rowbytes if 0 were passed in
    rowBytes = this->rowBytes();

    SkMallocPixelRef::PRFactory defaultFactory;
    
    SkPixelRef* pr = defaultFactory.create(correctedInfo, rowBytes, NULL);
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

bool SkBitmap::allocPixels(const SkImageInfo& requestedInfo, SkPixelRefFactory* factory,
                           SkColorTable* ctable) {
    if (kIndex_8_SkColorType == requestedInfo.fColorType && NULL == ctable) {
        return reset_return_false(this);
    }
    if (!this->setInfo(requestedInfo)) {
        return reset_return_false(this);
    }

    // setInfo may have corrected info (e.g. 565 is always opaque).
    const SkImageInfo& correctedInfo = this->info();

    SkMallocPixelRef::PRFactory defaultFactory;
    if (NULL == factory) {
        factory = &defaultFactory;
    }

    SkPixelRef* pr = factory->create(correctedInfo, correctedInfo.minRowBytes(), ctable);
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

bool SkBitmap::installPixels(const SkImageInfo& requestedInfo, void* pixels, size_t rb,
                             SkColorTable* ct, void (*releaseProc)(void* addr, void* context),
                             void* context) {
    if (!this->setInfo(requestedInfo, rb)) {
        this->reset();
        return false;
    }

    // setInfo may have corrected info (e.g. 565 is always opaque).
    const SkImageInfo& correctedInfo = this->info();

    SkPixelRef* pr = SkMallocPixelRef::NewWithProc(correctedInfo, rb, ct, pixels, releaseProc,
                                                   context);
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

///////////////////////////////////////////////////////////////////////////////

void SkBitmap::freePixels() {
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
    const SkImageInfo info = dst->info();
    if (kUnknown_SkColorType == info.colorType()) {
//        SkDebugf("unsupported config for info %d\n", dst->config());
        return false;
    }

    SkPixelRef* pr = SkMallocPixelRef::NewAllocate(info, dst->rowBytes(), ctable);
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
    return fPixelRef ? fPixelRef->isImmutable() : false;
}

void SkBitmap::setImmutable() {
    if (fPixelRef) {
        fPixelRef->setImmutable();
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

    switch (this->colorType()) {
        case kAlpha_8_SkColorType: {
            uint8_t* addr = this->getAddr8(x, y);
            return SkColorSetA(0, addr[0]);
        }
        case kIndex_8_SkColorType: {
            SkPMColor c = this->getIndex8Color(x, y);
            return SkUnPreMultiply::PMColorToColor(c);
        }
        case kRGB_565_SkColorType: {
            uint16_t* addr = this->getAddr16(x, y);
            return SkPixel16ToColor(addr[0]);
        }
        case kARGB_4444_SkColorType: {
            uint16_t* addr = this->getAddr16(x, y);
            SkPMColor c = SkPixel4444ToPixel32(addr[0]);
            return SkUnPreMultiply::PMColorToColor(c);
        }
        case kBGRA_8888_SkColorType:
        case kRGBA_8888_SkColorType: {
            uint32_t* addr = this->getAddr32(x, y);
            return SkUnPreMultiply::PMColorToColor(addr[0]);
        }
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

    switch (bm.colorType()) {
        case kAlpha_8_SkColorType: {
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
        case kIndex_8_SkColorType: {
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
        case kRGB_565_SkColorType:
            return true;
            break;
        case kARGB_4444_SkColorType: {
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
        case kBGRA_8888_SkColorType:
        case kRGBA_8888_SkColorType: {
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
            return; // can't erase. Should we bzero so the memory is not uninitialized?
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

            // make rgb premultiplied
            if (255 != a) {
                r = SkAlphaMul(r, a);
                g = SkAlphaMul(g, a);
                b = SkAlphaMul(b, a);
            }

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
        case kBGRA_8888_SkColorType:
        case kRGBA_8888_SkColorType: {
            uint32_t* p = this->getAddr32(area.fLeft, area.fTop);

            if (255 != a && kPremul_SkAlphaType == this->alphaType()) {
                r = SkAlphaMul(r, a);
                g = SkAlphaMul(g, a);
                b = SkAlphaMul(b, a);
            }
            uint32_t v = kRGBA_8888_SkColorType == this->colorType() ?
                         SkPackARGB_as_RGBA(a, r, g, b) : SkPackARGB_as_BGRA(a, r, g, b);

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
        SkPixelRef* pixelRef = fPixelRef->deepCopy(this->colorType(), &subset);
        if (pixelRef != NULL) {
            SkBitmap dst;
            dst.setInfo(SkImageInfo::Make(subset.width(), subset.height(),
                                          this->colorType(), this->alphaType()));
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
    dst.setInfo(SkImageInfo::Make(r.width(), r.height(), this->colorType(), this->alphaType()),
                this->rowBytes());
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
    const SkColorType srcCT = this->colorType();

    if (srcCT == kUnknown_SkColorType) {
        return false;
    }

    bool sameConfigs = (srcCT == dstColorType);
    switch (dstColorType) {
        case kAlpha_8_SkColorType:
        case kRGB_565_SkColorType:
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
            break;
        case kIndex_8_SkColorType:
            if (!sameConfigs) {
                return false;
            }
            break;
        case kARGB_4444_SkColorType:
            return sameConfigs || kN32_SkColorType == srcCT || kIndex_8_SkColorType == srcCT;
        default:
            return false;
    }
    return true;
}

#include "SkConfig8888.h"

bool SkBitmap::readPixels(const SkImageInfo& requestedDstInfo, void* dstPixels, size_t dstRB,
                          int x, int y) const {
    if (kUnknown_SkColorType == requestedDstInfo.colorType()) {
        return false;
    }
    if (NULL == dstPixels || dstRB < requestedDstInfo.minRowBytes()) {
        return false;
    }
    if (0 == requestedDstInfo.width() || 0 == requestedDstInfo.height()) {
        return false;
    }
    
    SkIRect srcR = SkIRect::MakeXYWH(x, y, requestedDstInfo.width(), requestedDstInfo.height());
    if (!srcR.intersect(0, 0, this->width(), this->height())) {
        return false;
    }
    
    SkImageInfo dstInfo = requestedDstInfo;
    // the intersect may have shrunk info's logical size
    dstInfo.fWidth = srcR.width();
    dstInfo.fHeight = srcR.height();
    
    // if x or y are negative, then we have to adjust pixels
    if (x > 0) {
        x = 0;
    }
    if (y > 0) {
        y = 0;
    }
    // here x,y are either 0 or negative
    dstPixels = ((char*)dstPixels - y * dstRB - x * dstInfo.bytesPerPixel());

    //////////////
    
    SkAutoLockPixels alp(*this);
    
    // since we don't stop creating un-pixeled devices yet, check for no pixels here
    if (NULL == this->getPixels()) {
        return false;
    }
    
    SkImageInfo srcInfo = this->info();
    srcInfo.fWidth = dstInfo.width();
    srcInfo.fHeight = dstInfo.height();
    
    const void* srcPixels = this->getAddr(srcR.x(), srcR.y());
    return SkPixelInfo::CopyPixels(dstInfo, dstPixels, dstRB, srcInfo, srcPixels, this->rowBytes(),
                                   this->getColorTable());
}

bool SkBitmap::copyTo(SkBitmap* dst, SkColorType dstColorType, Allocator* alloc) const {
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
            if (fPixelRef->info().alphaType() == kUnpremul_SkAlphaType) {
                // FIXME: The only meaningful implementation of readPixels
                // (GrPixelRef) assumes premultiplied pixels.
                return false;
            }
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
    if (!tmpDst.setInfo(dstInfo)) {
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

    if (!src->readPixels(tmpDst.info(), tmpDst.getPixels(), tmpDst.rowBytes(), 0, 0)) {
        return false;
    }

    //  (for BitmapHeap) Clone the pixelref genID even though we have a new pixelref.
    //  The old copyTo impl did this, so we continue it for now.
    //
    //  TODO: should we ignore rowbytes (i.e. getSize)? Then it could just be
    //      if (src_pixelref->info == dst_pixelref->info)
    //
    if (src->colorType() == dstColorType && tmpDst.getSize() == src->getSize()) {
        SkPixelRef* dstPixelRef = tmpDst.pixelRef();
        if (dstPixelRef->info() == fPixelRef->info()) {
            dstPixelRef->cloneGenID(*fPixelRef);
        }
    }

    dst->swap(tmpDst);
    return true;
}

bool SkBitmap::deepCopyTo(SkBitmap* dst) const {
    const SkColorType dstCT = this->colorType();

    if (!this->canCopyTo(dstCT)) {
        return false;
    }

    // If we have a PixelRef, and it supports deep copy, use it.
    // Currently supported only by texture-backed bitmaps.
    if (fPixelRef) {
        SkPixelRef* pixelRef = fPixelRef->deepCopy(dstCT, NULL);
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
                // With the new config, an appropriate fRowBytes will be computed by setInfo.
                rowBytes = 0;
            }

            SkImageInfo info = fInfo;
            info.fColorType = dstCT;
            if (!dst->setInfo(info, rowBytes)) {
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

static bool GetBitmapAlpha(const SkBitmap& src, uint8_t* SK_RESTRICT alpha,
                           int alphaRowBytes) {
    SkASSERT(alpha != NULL);
    SkASSERT(alphaRowBytes >= src.width());

    SkColorType colorType = src.colorType();
    int         w = src.width();
    int         h = src.height();
    size_t      rb = src.rowBytes();

    SkAutoLockPixels alp(src);
    if (!src.readyToDraw()) {
        // zero out the alpha buffer and return
        while (--h >= 0) {
            memset(alpha, 0, w);
            alpha += alphaRowBytes;
        }
        return false;
    }

    if (kAlpha_8_SkColorType == colorType && !src.isOpaque()) {
        const uint8_t* s = src.getAddr8(0, 0);
        while (--h >= 0) {
            memcpy(alpha, s, w);
            s += rb;
            alpha += alphaRowBytes;
        }
    } else if (kN32_SkColorType == colorType && !src.isOpaque()) {
        const SkPMColor* SK_RESTRICT s = src.getAddr32(0, 0);
        while (--h >= 0) {
            for (int x = 0; x < w; x++) {
                alpha[x] = SkGetPackedA32(s[x]);
            }
            s = (const SkPMColor*)((const char*)s + rb);
            alpha += alphaRowBytes;
        }
    } else if (kARGB_4444_SkColorType == colorType && !src.isOpaque()) {
        const SkPMColor16* SK_RESTRICT s = src.getAddr16(0, 0);
        while (--h >= 0) {
            for (int x = 0; x < w; x++) {
                alpha[x] = SkPacked4444ToA32(s[x]);
            }
            s = (const SkPMColor16*)((const char*)s + rb);
            alpha += alphaRowBytes;
        }
    } else if (kIndex_8_SkColorType == colorType && !src.isOpaque()) {
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
        tmpBitmap.setInfo(SkImageInfo::MakeA8(this->width(), this->height()), srcM.fRowBytes);
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

    tmpBitmap.setInfo(SkImageInfo::MakeA8(dstM.fBounds.width(), dstM.fBounds.height()),
                      dstM.fRowBytes);
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

void SkBitmap::WriteRawPixels(SkWriteBuffer* buffer, const SkBitmap& bitmap) {
    const SkImageInfo info = bitmap.info();
    SkAutoLockPixels alp(bitmap);
    if (0 == info.width() || 0 == info.height() || NULL == bitmap.getPixels()) {
        buffer->writeUInt(0); // instead of snugRB, signaling no pixels
        return;
    }

    const size_t snugRB = info.width() * info.bytesPerPixel();
    const char* src = (const char*)bitmap.getPixels();
    const size_t ramRB = bitmap.rowBytes();

    buffer->write32(SkToU32(snugRB));
    info.flatten(*buffer);

    const size_t size = snugRB * info.height();
    SkAutoMalloc storage(size);
    char* dst = (char*)storage.get();
    for (int y = 0; y < info.height(); ++y) {
        memcpy(dst, src, snugRB);
        dst += snugRB;
        src += ramRB;
    }
    buffer->writeByteArray(storage.get(), size);

    SkColorTable* ct = bitmap.getColorTable();
    if (kIndex_8_SkColorType == info.colorType() && ct) {
        buffer->writeBool(true);
        ct->writeToBuffer(*buffer);
    } else {
        buffer->writeBool(false);
    }
}

bool SkBitmap::ReadRawPixels(SkReadBuffer* buffer, SkBitmap* bitmap) {
    const size_t snugRB = buffer->readUInt();
    if (0 == snugRB) {  // no pixels
        return false;
    }

    SkImageInfo info;
    info.unflatten(*buffer);

    // If there was an error reading "info", don't use it to compute minRowBytes()
    if (!buffer->validate(true)) {
        return false;
    }

    const size_t ramRB = info.minRowBytes();
    const int height = info.height();
    const size_t snugSize = snugRB * height;
    const size_t ramSize = ramRB * height;
    if (!buffer->validate(snugSize <= ramSize)) {
        return false;
    }

    char* dst = (char*)sk_malloc_throw(ramSize);
    buffer->readByteArray(dst, snugSize);
    SkAutoDataUnref data(SkData::NewFromMalloc(dst, ramSize));

    if (snugSize != ramSize) {
        const char* srcRow = dst + snugRB * (height - 1);
        char* dstRow = dst + ramRB * (height - 1);
        for (int y = height - 1; y >= 1; --y) {
            memmove(dstRow, srcRow, snugRB);
            srcRow -= snugRB;
            dstRow -= ramRB;
        }
        SkASSERT(srcRow == dstRow); // first row does not need to be moved
    }

    SkAutoTUnref<SkColorTable> ctable;
    if (buffer->readBool()) {
        ctable.reset(SkNEW_ARGS(SkColorTable, (*buffer)));
    }

    SkAutoTUnref<SkPixelRef> pr(SkMallocPixelRef::NewWithData(info, info.minRowBytes(),
                                                              ctable.get(), data.get()));
    bitmap->setInfo(pr->info());
    bitmap->setPixelRef(pr, 0, 0);
    return true;
}

enum {
    SERIALIZE_PIXELTYPE_NONE,
    SERIALIZE_PIXELTYPE_REF_DATA
};

void SkBitmap::legacyUnflatten(SkReadBuffer& buffer) {
#ifdef SK_SUPPORT_LEGACY_PIXELREF_UNFLATTENABLE
    this->reset();

    SkImageInfo info;
    info.unflatten(buffer);
    size_t rowBytes = buffer.readInt();
    if (!buffer.validate((info.width() >= 0) && (info.height() >= 0) &&
                         SkColorTypeIsValid(info.fColorType) &&
                         SkAlphaTypeIsValid(info.fAlphaType) &&
                         SkColorTypeValidateAlphaType(info.fColorType, info.fAlphaType) &&
                         info.validRowBytes(rowBytes))) {
        return;
    }

    bool configIsValid = this->setInfo(info, rowBytes);
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
                SkPixelRef* pr = buffer.readFlattenable<SkPixelRef>();
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
#else
    sk_throw();
#endif
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
    uint8_t allFlags = kImageIsVolatile_Flag;
#ifdef SK_BUILD_FOR_ANDROID
    allFlags |= kHasHardwareMipMap_Flag;
#endif
    SkASSERT((~allFlags & fFlags) == 0);
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

    static const char* gColorTypeNames[kLastEnum_SkColorType + 1] = {
        "UNKNOWN", "A8", "565", "4444", "RGBA", "BGRA", "INDEX8",
    };

    str->appendf("bitmap: ((%d, %d) %s", this->width(), this->height(),
                 gColorTypeNames[this->colorType()]);

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
