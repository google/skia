/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAtomics.h"
#include "SkBitmap.h"
#include "SkColorPriv.h"
#include "SkConvertPixels.h"
#include "SkData.h"
#include "SkFilterQuality.h"
#include "SkHalf.h"
#include "SkImageInfoPriv.h"
#include "SkMallocPixelRef.h"
#include "SkMask.h"
#include "SkMath.h"
#include "SkPixelRef.h"
#include "SkReadBuffer.h"
#include "SkRect.h"
#include "SkScalar.h"
#include "SkTemplates.h"
#include "SkUnPreMultiply.h"
#include "SkWriteBuffer.h"
#include "SkWritePixelsRec.h"

#include <string.h>

static bool reset_return_false(SkBitmap* bm) {
    bm->reset();
    return false;
}

SkBitmap::SkBitmap()
    : fPixelLockCount(0)
    , fPixels        (nullptr)
    , fColorTable    (nullptr)
    , fPixelRefOrigin{0, 0}
    , fRowBytes      (0)
    , fFlags         (0) {}

// copy pixelref, but don't copy lock.
SkBitmap::SkBitmap(const SkBitmap& src)
    : fPixelRef      (src.fPixelRef)
    , fPixelLockCount(0)
    , fPixels        (nullptr)
    , fColorTable    (nullptr)
    , fPixelRefOrigin(src.fPixelRefOrigin)
    , fInfo          (src.fInfo)
    , fRowBytes      (src.fRowBytes)
    , fFlags         (src.fFlags)
{
    SkDEBUGCODE(src.validate();)
    SkDEBUGCODE(this->validate();)
}

// take lock and lockcount from other.
SkBitmap::SkBitmap(SkBitmap&& other)
    : fPixelRef      (std::move(other.fPixelRef))
    , fPixelLockCount          (other.fPixelLockCount)
    , fPixels                  (other.fPixels)
    , fColorTable              (other.fColorTable)
    , fPixelRefOrigin          (other.fPixelRefOrigin)
    , fInfo          (std::move(other.fInfo))
    , fRowBytes                (other.fRowBytes)
    , fFlags                   (other.fFlags) {
    SkASSERT(!other.fPixelRef);
    other.fInfo.reset();
    other.fPixelLockCount = 0;
    other.fPixels         = nullptr;
    other.fColorTable     = nullptr;
    other.fPixelRefOrigin = SkIPoint{0, 0};
    other.fRowBytes       = 0;
    other.fFlags          = 0;
}

SkBitmap::~SkBitmap() {
    SkDEBUGCODE(this->validate();)
    this->freePixels();
}

SkBitmap& SkBitmap::operator=(const SkBitmap& src) {
    if (this != &src) {
        this->freePixels();
        SkASSERT(!fPixels);
        SkASSERT(!fColorTable);
        SkASSERT(!fPixelLockCount);
        fPixelRef       = src.fPixelRef;
        fPixelRefOrigin = src.fPixelRefOrigin;
        fInfo           = src.fInfo;
        fRowBytes       = src.fRowBytes;
        fFlags          = src.fFlags;
    }
    SkDEBUGCODE(this->validate();)
    return *this;
}

SkBitmap& SkBitmap::operator=(SkBitmap&& other) {
    if (this != &other) {
        this->freePixels();
        SkASSERT(!fPixels);
        SkASSERT(!fColorTable);
        SkASSERT(!fPixelLockCount);
        fPixelRef       = std::move(other.fPixelRef);
        fInfo           = std::move(other.fInfo);
        fPixelLockCount = other.fPixelLockCount;
        fPixels         = other.fPixels;
        fColorTable     = other.fColorTable;
        fPixelRefOrigin = other.fPixelRefOrigin;
        fRowBytes       = other.fRowBytes;
        fFlags          = other.fFlags;
        SkASSERT(!other.fPixelRef);
        other.fInfo.reset();
        other.fPixelLockCount = 0;
        other.fPixels         = nullptr;
        other.fColorTable     = nullptr;
        other.fPixelRefOrigin = SkIPoint{0, 0};
        other.fRowBytes       = 0;
        other.fFlags          = 0;
    }
    return *this;
}

void SkBitmap::swap(SkBitmap& other) {
    SkTSwap(*this, other);
    SkDEBUGCODE(this->validate();)
}

void SkBitmap::reset() {
    this->freePixels();
    this->fInfo.reset();
    sk_bzero(this, sizeof(*this));
}

void SkBitmap::getBounds(SkRect* bounds) const {
    SkASSERT(bounds);
    bounds->set(0, 0,
                SkIntToScalar(fInfo.width()), SkIntToScalar(fInfo.height()));
}

void SkBitmap::getBounds(SkIRect* bounds) const {
    SkASSERT(bounds);
    bounds->set(0, 0, fInfo.width(), fInfo.height());
}

///////////////////////////////////////////////////////////////////////////////

bool SkBitmap::setInfo(const SkImageInfo& info, size_t rowBytes) {
    SkAlphaType newAT = info.alphaType();
    if (!SkColorTypeValidateAlphaType(info.colorType(), info.alphaType(), &newAT)) {
        return reset_return_false(this);
    }
    // don't look at info.alphaType(), since newAT is the real value...

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

    fInfo = info.makeAlphaType(newAT);
    fRowBytes = SkToU32(rowBytes);
    return true;
}

bool SkBitmap::setAlphaType(SkAlphaType newAlphaType) {
    if (!SkColorTypeValidateAlphaType(fInfo.colorType(), newAlphaType, &newAlphaType)) {
        return false;
    }
    if (fInfo.alphaType() != newAlphaType) {
        fInfo = fInfo.makeAlphaType(newAlphaType);
        if (fPixelRef) {
            fPixelRef->changeAlphaType(newAlphaType);
        }
    }
    return true;
}

void SkBitmap::updatePixelsFromRef() const {
    if (fPixelRef) {
        if (fPixelLockCount > 0) {
            SkASSERT(fPixelRef->isLocked());

            void* p = fPixelRef->pixels();
            if (p) {
                p = (char*)p
                    + fPixelRefOrigin.fY * fRowBytes
                    + fPixelRefOrigin.fX * fInfo.bytesPerPixel();
            }
            fPixels = p;
            fColorTable = fPixelRef->colorTable();
        } else {
            SkASSERT(0 == fPixelLockCount);
            fPixels = nullptr;
            fColorTable = nullptr;
        }
    }
}

void SkBitmap::setPixelRef(sk_sp<SkPixelRef> pr, int dx, int dy) {
#ifdef SK_DEBUG
    if (pr) {
        if (kUnknown_SkColorType != fInfo.colorType()) {
            const SkImageInfo& prInfo = pr->info();
            SkASSERT(fInfo.width() <= prInfo.width());
            SkASSERT(fInfo.height() <= prInfo.height());
            SkASSERT(fInfo.colorType() == prInfo.colorType());
            switch (prInfo.alphaType()) {
                case kUnknown_SkAlphaType:
                    SkASSERT(fInfo.alphaType() == kUnknown_SkAlphaType);
                    break;
                case kOpaque_SkAlphaType:
                case kPremul_SkAlphaType:
                    SkASSERT(fInfo.alphaType() == kOpaque_SkAlphaType ||
                             fInfo.alphaType() == kPremul_SkAlphaType);
                    break;
                case kUnpremul_SkAlphaType:
                    SkASSERT(fInfo.alphaType() == kOpaque_SkAlphaType ||
                             fInfo.alphaType() == kUnpremul_SkAlphaType);
                    break;
            }
        }
    }
#endif

    if (pr) {
        const SkImageInfo& info = pr->info();
        fPixelRefOrigin.set(SkTPin(dx, 0, info.width()), SkTPin(dy, 0, info.height()));
    } else {
        // ignore dx,dy if there is no pixelref
        fPixelRefOrigin.setZero();
    }

    if (fPixelRef != pr) {
        this->freePixels();
        SkASSERT(!fPixelRef);

        fPixelRef = std::move(pr);
        this->updatePixelsFromRef();
    }

    SkDEBUGCODE(this->validate();)
}

void SkBitmap::lockPixels() const {
    if (fPixelRef && 0 == sk_atomic_inc(&fPixelLockCount)) {
        fPixelRef->lockPixels();
        this->updatePixelsFromRef();
    }
    SkDEBUGCODE(this->validate();)
}

void SkBitmap::unlockPixels() const {
    SkASSERT(!fPixelRef || fPixelLockCount > 0);

    if (fPixelRef && 1 == sk_atomic_dec(&fPixelLockCount)) {
        fPixelRef->unlockPixels();
        this->updatePixelsFromRef();
    }
    SkDEBUGCODE(this->validate();)
}

void SkBitmap::setPixels(void* p, SkColorTable* ctable) {
    if (nullptr == p) {
        this->setPixelRef(nullptr, 0, 0);
        return;
    }

    if (kUnknown_SkColorType == fInfo.colorType()) {
        this->setPixelRef(nullptr, 0, 0);
        return;
    }

    this->setPixelRef(SkMallocPixelRef::MakeDirect(fInfo, p, fRowBytes, sk_ref_sp(ctable)), 0, 0);
    if (!fPixelRef) {
        return;
    }
    // since we're already allocated, we lockPixels right away
    this->lockPixels();
    SkDEBUGCODE(this->validate();)
}

bool SkBitmap::tryAllocPixels(Allocator* allocator, SkColorTable* ctable) {
    HeapAllocator stdalloc;

    if (nullptr == allocator) {
        allocator = &stdalloc;
    }
    return allocator->allocPixelRef(this, ctable);
}

///////////////////////////////////////////////////////////////////////////////

bool SkBitmap::tryAllocPixels(const SkImageInfo& requestedInfo, size_t rowBytes) {
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

    sk_sp<SkPixelRef> pr = SkMallocPixelRef::MakeAllocate(correctedInfo, rowBytes, nullptr);
    if (!pr) {
        return reset_return_false(this);
    }
    this->setPixelRef(std::move(pr), 0, 0);

    // TODO: lockPixels could/should return bool or void*/nullptr
    this->lockPixels();
    if (nullptr == this->getPixels()) {
        return reset_return_false(this);
    }
    return true;
}

bool SkBitmap::tryAllocPixels(const SkImageInfo& requestedInfo, sk_sp<SkColorTable> ctable,
                              uint32_t allocFlags) {
    if (kIndex_8_SkColorType == requestedInfo.colorType() && nullptr == ctable) {
        return reset_return_false(this);
    }
    if (!this->setInfo(requestedInfo)) {
        return reset_return_false(this);
    }

    // setInfo may have corrected info (e.g. 565 is always opaque).
    const SkImageInfo& correctedInfo = this->info();

    sk_sp<SkPixelRef> pr = (allocFlags & kZeroPixels_AllocFlag) ?
        SkMallocPixelRef::MakeZeroed(correctedInfo, correctedInfo.minRowBytes(), ctable) :
        SkMallocPixelRef::MakeAllocate(correctedInfo, correctedInfo.minRowBytes(), ctable);
    if (!pr) {
        return reset_return_false(this);
    }
    this->setPixelRef(std::move(pr), 0, 0);

    this->lockPixels();
    if (nullptr == this->getPixels()) {
        return reset_return_false(this);
    }
    return true;
}

static void invoke_release_proc(void (*proc)(void* pixels, void* ctx), void* pixels, void* ctx) {
    if (proc) {
        proc(pixels, ctx);
    }
}

bool SkBitmap::installPixels(const SkImageInfo& requestedInfo, void* pixels, size_t rb,
                             SkColorTable* ct, void (*releaseProc)(void* addr, void* context),
                             void* context) {
    if (!this->setInfo(requestedInfo, rb)) {
        invoke_release_proc(releaseProc, pixels, context);
        this->reset();
        return false;
    }
    if (nullptr == pixels) {
        invoke_release_proc(releaseProc, pixels, context);
        return true;    // we behaved as if they called setInfo()
    }

    // setInfo may have corrected info (e.g. 565 is always opaque).
    const SkImageInfo& correctedInfo = this->info();

    sk_sp<SkPixelRef> pr = SkMallocPixelRef::MakeWithProc(correctedInfo, rb, sk_ref_sp(ct),
                                                          pixels, releaseProc, context);
    if (!pr) {
        this->reset();
        return false;
    }

    this->setPixelRef(std::move(pr), 0, 0);

    // since we're already allocated, we lockPixels right away
    this->lockPixels();
    SkDEBUGCODE(this->validate();)
    return true;
}

bool SkBitmap::installPixels(const SkPixmap& pixmap) {
    return this->installPixels(pixmap.info(), pixmap.writable_addr(),
                               pixmap.rowBytes(), pixmap.ctable(),
                               nullptr, nullptr);
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
    if (fPixelRef) {
        if (fPixelLockCount > 0) {
            fPixelRef->unlockPixels();
        }
        fPixelRef = nullptr;
        fPixelRefOrigin.setZero();
    }
    fPixelLockCount = 0;
    fPixels = nullptr;
    fColorTable = nullptr;
}

uint32_t SkBitmap::getGenerationID() const {
    return fPixelRef ? fPixelRef->getGenerationID() : 0;
}

void SkBitmap::notifyPixelsChanged() const {
    SkASSERT(!this->isImmutable());
    if (fPixelRef) {
        fPixelRef->notifyPixelsChanged();
    }
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

    sk_sp<SkPixelRef> pr = SkMallocPixelRef::MakeAllocate(info, dst->rowBytes(), sk_ref_sp(ctable));
    if (!pr) {
        return false;
    }

    dst->setPixelRef(std::move(pr), 0, 0);
    // since we're already allocated, we lockPixels right away
    dst->lockPixels();
    return true;
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
            case kRGBA_F16_SkColorType:
                base += x << 3;
                break;
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
            case kGray_8_SkColorType:
                base += x;
                break;
            default:
                SkDEBUGFAIL("Can't return addr for config");
                base = nullptr;
                break;
        }
    }
    return base;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SkBitmap::erase(SkColor c, const SkIRect& area) const {
    SkDEBUGCODE(this->validate();)

    switch (fInfo.colorType()) {
        case kUnknown_SkColorType:
        case kIndex_8_SkColorType:
            // TODO: can we ASSERT that we never get here?
            return; // can't erase. Should we bzero so the memory is not uninitialized?
        default:
            break;
    }

    SkAutoPixmapUnlock result;
    if (!this->requestLock(&result)) {
        return;
    }

    if (result.pixmap().erase(c, area)) {
        this->notifyPixelsChanged();
    }
}

void SkBitmap::eraseColor(SkColor c) const {
    this->erase(c, SkIRect::MakeWH(this->width(), this->height()));
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

bool SkBitmap::extractSubset(SkBitmap* result, const SkIRect& subset) const {
    SkDEBUGCODE(this->validate();)

    if (nullptr == result || !fPixelRef) {
        return false;   // no src pixels
    }

    SkIRect srcRect, r;
    srcRect.set(0, 0, this->width(), this->height());
    if (!r.intersect(srcRect, subset)) {
        return false;   // r is empty (i.e. no intersection)
    }

    // If the upper left of the rectangle was outside the bounds of this SkBitmap, we should have
    // exited above.
    SkASSERT(static_cast<unsigned>(r.fLeft) < static_cast<unsigned>(this->width()));
    SkASSERT(static_cast<unsigned>(r.fTop) < static_cast<unsigned>(this->height()));

    SkBitmap dst;
    dst.setInfo(this->info().makeWH(r.width(), r.height()), this->rowBytes());
    dst.setIsVolatile(this->isVolatile());

    if (fPixelRef) {
        SkIPoint origin = fPixelRefOrigin;
        origin.fX += r.fLeft;
        origin.fY += r.fTop;
        // share the pixelref with a custom offset
        dst.setPixelRef(fPixelRef, origin.x(), origin.y());
    }
    SkDEBUGCODE(dst.validate();)

    // we know we're good, so commit to result
    result->swap(dst);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool SkBitmap::canCopyTo(SkColorType dstCT) const {
    const SkColorType srcCT = this->colorType();

    if (srcCT == kUnknown_SkColorType) {
        return false;
    }
    if (srcCT == kAlpha_8_SkColorType && dstCT != kAlpha_8_SkColorType) {
        return false;   // can't convert from alpha to non-alpha
    }

    bool sameConfigs = (srcCT == dstCT);
    switch (dstCT) {
        case kAlpha_8_SkColorType:
        case kRGB_565_SkColorType:
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
        case kRGBA_F16_SkColorType:
            break;
        case kGray_8_SkColorType:
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

bool SkBitmap::readPixels(const SkImageInfo& requestedDstInfo, void* dstPixels, size_t dstRB,
                          int x, int y) const {
    SkAutoPixmapUnlock src;
    if (!this->requestLock(&src)) {
        return false;
    }
    return src.pixmap().readPixels(requestedDstInfo, dstPixels, dstRB, x, y);
}

bool SkBitmap::readPixels(const SkPixmap& dst, int srcX, int srcY) const {
    return this->readPixels(dst.info(), dst.writable_addr(), dst.rowBytes(), srcX, srcY);
}

bool SkBitmap::writePixels(const SkPixmap& src, int dstX, int dstY,
                           SkTransferFunctionBehavior behavior) {
    SkAutoPixmapUnlock dst;
    if (!this->requestLock(&dst)) {
        return false;
    }

    if (!SkImageInfoValidConversion(fInfo, src.info())) {
        return false;
    }

    SkWritePixelsRec rec(src.info(), src.addr(), src.rowBytes(), dstX, dstY);
    if (!rec.trim(fInfo.width(), fInfo.height())) {
        return false;
    }

    void* dstPixels = this->getAddr(rec.fX, rec.fY);
    const SkImageInfo dstInfo = fInfo.makeWH(rec.fInfo.width(), rec.fInfo.height());
    SkConvertPixels(dstInfo, dstPixels, this->rowBytes(), rec.fInfo, rec.fPixels, rec.fRowBytes,
                    src.ctable(), behavior);
    return true;
}

bool SkBitmap::internalCopyTo(SkBitmap* dst, SkColorType dstColorType, Allocator* alloc) const {
    if (!this->canCopyTo(dstColorType)) {
        return false;
    }

    SkAutoPixmapUnlock srcUnlocker;
    if (!this->requestLock(&srcUnlocker)) {
        return false;
    }
    SkPixmap srcPM = srcUnlocker.pixmap();

    // Various Android specific compatibility modes.
    // TODO:
    // Move the logic of this entire function into the framework, then call readPixels() directly.
    SkImageInfo dstInfo = srcPM.info().makeColorType(dstColorType);
    switch (dstColorType) {
        case kRGB_565_SkColorType:
            // copyTo() is not strict on alpha type.  Here we set the src to opaque to allow
            // the call to readPixels() to succeed and preserve this lenient behavior.
            if (kOpaque_SkAlphaType != srcPM.alphaType()) {
                srcPM = SkPixmap(srcPM.info().makeAlphaType(kOpaque_SkAlphaType), srcPM.addr(),
                                 srcPM.rowBytes(), srcPM.ctable());
                dstInfo = dstInfo.makeAlphaType(kOpaque_SkAlphaType);
            }
            break;
        case kRGBA_F16_SkColorType:
            // The caller does not have an opportunity to pass a dst color space.  Assume that
            // they want linear sRGB.
            dstInfo = dstInfo.makeColorSpace(SkColorSpace::MakeSRGBLinear());

            if (!srcPM.colorSpace()) {
                // We can't do a sane conversion to F16 without a dst color space.  Guess sRGB
                // in this case.
                srcPM.setColorSpace(SkColorSpace::MakeSRGB());
            }
            break;
        default:
            break;
    }

    SkBitmap tmpDst;
    if (!tmpDst.setInfo(dstInfo)) {
        return false;
    }

    // allocate colortable if srcConfig == kIndex8_Config
    sk_sp<SkColorTable> ctable;
    if (dstColorType == kIndex_8_SkColorType) {
        ctable.reset(SkRef(srcPM.ctable()));
    }
    if (!tmpDst.tryAllocPixels(alloc, ctable.get())) {
        return false;
    }

    SkAutoPixmapUnlock dstUnlocker;
    if (!tmpDst.requestLock(&dstUnlocker)) {
        return false;
    }

    SkPixmap dstPM = dstUnlocker.pixmap();

    // We can't do a sane conversion from F16 without a src color space.  Guess sRGB in this case.
    if (kRGBA_F16_SkColorType == srcPM.colorType() && !dstPM.colorSpace()) {
        dstPM.setColorSpace(SkColorSpace::MakeSRGB());
    }

    // readPixels does not yet support color spaces with parametric transfer functions.  This
    // works around that restriction when the color spaces are equal.
    if (kRGBA_F16_SkColorType != dstColorType && kRGBA_F16_SkColorType != srcPM.colorType() &&
            dstPM.colorSpace() == srcPM.colorSpace()) {
        dstPM.setColorSpace(nullptr);
        srcPM.setColorSpace(nullptr);
    }

    if (!srcPM.readPixels(dstPM)) {
        return false;
    }

    //  (for BitmapHeap) Clone the pixelref genID even though we have a new pixelref.
    //  The old copyTo impl did this, so we continue it for now.
    //
    //  TODO: should we ignore rowbytes (i.e. getSize)? Then it could just be
    //      if (src_pixelref->info == dst_pixelref->info)
    //
    if (srcPM.colorType() == dstColorType && tmpDst.getSize() == srcPM.getSize64()) {
        SkPixelRef* dstPixelRef = tmpDst.pixelRef();
        if (dstPixelRef->info() == fPixelRef->info()) {
            dstPixelRef->cloneGenID(*fPixelRef);
        }
    }

    dst->swap(tmpDst);
    return true;
}

bool SkBitmap::copyTo(SkBitmap* dst, SkColorType ct) const {
    return this->internalCopyTo(dst, ct, nullptr);
}

#ifdef SK_BUILD_FOR_ANDROID
bool SkBitmap::copyTo(SkBitmap* dst, SkColorType ct, Allocator* alloc) const {
    return this->internalCopyTo(dst, ct, alloc);
}
#endif

// TODO: can we merge this with copyTo?
bool SkBitmap::deepCopyTo(SkBitmap* dst) const {
    const SkColorType dstCT = this->colorType();

    if (!this->canCopyTo(dstCT)) {
        return false;
    }
    return this->copyTo(dst, dstCT);
}

///////////////////////////////////////////////////////////////////////////////

static bool GetBitmapAlpha(const SkBitmap& src, uint8_t* SK_RESTRICT alpha, int alphaRowBytes) {
    SkASSERT(alpha != nullptr);
    SkASSERT(alphaRowBytes >= src.width());

    SkAutoPixmapUnlock apl;
    if (!src.requestLock(&apl)) {
        for (int y = 0; y < src.height(); ++y) {
            memset(alpha, 0, src.width());
            alpha += alphaRowBytes;
        }
        return false;
    }
    const SkPixmap& pmap = apl.pixmap();
    SkConvertPixels(SkImageInfo::MakeA8(pmap.width(), pmap.height()), alpha, alphaRowBytes,
                    pmap.info(), pmap.addr(), pmap.rowBytes(), pmap.ctable(),
                    SkTransferFunctionBehavior::kRespect);
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

    SkMaskFilter* filter = paint ? paint->getMaskFilter() : nullptr;

    // compute our (larger?) dst bounds if we have a filter
    if (filter) {
        identity.reset();
        if (!filter->filterMask(&dstM, srcM, identity, nullptr)) {
            goto NO_FILTER_CASE;
        }
        dstM.fRowBytes = SkAlign4(dstM.fBounds.width());
    } else {
    NO_FILTER_CASE:
        tmpBitmap.setInfo(SkImageInfo::MakeA8(this->width(), this->height()), srcM.fRowBytes);
        if (!tmpBitmap.tryAllocPixels(allocator, nullptr)) {
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
    if (!filter->filterMask(&dstM, srcM, identity, nullptr)) {
        goto NO_FILTER_CASE;
    }
    SkAutoMaskFreeImage dstCleanup(dstM.fImage);

    tmpBitmap.setInfo(SkImageInfo::MakeA8(dstM.fBounds.width(), dstM.fBounds.height()),
                      dstM.fRowBytes);
    if (!tmpBitmap.tryAllocPixels(allocator, nullptr)) {
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

static void write_raw_pixels(SkWriteBuffer* buffer, const SkPixmap& pmap) {
    const SkImageInfo& info = pmap.info();
    const size_t snugRB = info.width() * info.bytesPerPixel();
    const char* src = (const char*)pmap.addr();
    const size_t ramRB = pmap.rowBytes();

    buffer->write32(SkToU32(snugRB));
    info.flatten(*buffer);

    const size_t size = snugRB * info.height();
    SkAutoTMalloc<char> storage(size);
    char* dst = storage.get();
    for (int y = 0; y < info.height(); ++y) {
        memcpy(dst, src, snugRB);
        dst += snugRB;
        src += ramRB;
    }
    buffer->writeByteArray(storage.get(), size);

    const SkColorTable* ct = pmap.ctable();
    if (kIndex_8_SkColorType == info.colorType() && ct) {
        buffer->writeBool(true);
        ct->writeToBuffer(*buffer);
    } else {
        buffer->writeBool(false);
    }
}

void SkBitmap::WriteRawPixels(SkWriteBuffer* buffer, const SkBitmap& bitmap) {
    const SkImageInfo info = bitmap.info();
    if (0 == info.width() || 0 == info.height() || bitmap.isNull()) {
        buffer->writeUInt(0); // instead of snugRB, signaling no pixels
        return;
    }

    SkAutoPixmapUnlock result;
    if (!bitmap.requestLock(&result)) {
        buffer->writeUInt(0); // instead of snugRB, signaling no pixels
        return;
    }

    write_raw_pixels(buffer, result.pixmap());
}

bool SkBitmap::ReadRawPixels(SkReadBuffer* buffer, SkBitmap* bitmap) {
    const size_t snugRB = buffer->readUInt();
    if (0 == snugRB) {  // no pixels
        return false;
    }

    SkImageInfo info;
    info.unflatten(*buffer);

    if (info.width() < 0 || info.height() < 0) {
        return false;
    }

    // If there was an error reading "info" or if it is bogus,
    // don't use it to compute minRowBytes()
    if (!buffer->validate(SkColorTypeValidateAlphaType(info.colorType(),
                                                       info.alphaType()))) {
        return false;
    }

    const size_t ramRB = info.minRowBytes();
    const int height = SkMax32(info.height(), 0);
    const uint64_t snugSize = sk_64_mul(snugRB, height);
    const uint64_t ramSize = sk_64_mul(ramRB, height);
    static const uint64_t max_size_t = (size_t)(-1);
    if (!buffer->validate((snugSize <= ramSize) && (ramSize <= max_size_t))) {
        return false;
    }

    sk_sp<SkData> data(SkData::MakeUninitialized(SkToSizeT(ramSize)));
    unsigned char* dst = (unsigned char*)data->writable_data();
    buffer->readByteArray(dst, SkToSizeT(snugSize));

    if (snugSize != ramSize) {
        const unsigned char* srcRow = dst + snugRB * (height - 1);
        unsigned char* dstRow = dst + ramRB * (height - 1);
        for (int y = height - 1; y >= 1; --y) {
            memmove(dstRow, srcRow, snugRB);
            srcRow -= snugRB;
            dstRow -= ramRB;
        }
        SkASSERT(srcRow == dstRow); // first row does not need to be moved
    }

    sk_sp<SkColorTable> ctable;
    if (buffer->readBool()) {
        ctable = SkColorTable::Create(*buffer);
        if (!ctable) {
            return false;
        }

        if (info.isEmpty()) {
            // require an empty ctable
            if (ctable->count() != 0) {
                buffer->validate(false);
                return false;
            }
        } else {
            // require a non-empty ctable
            if (ctable->count() == 0) {
                buffer->validate(false);
                return false;
            }
            unsigned char maxIndex = ctable->count() - 1;
            for (uint64_t i = 0; i < ramSize; ++i) {
                dst[i] = SkTMin(dst[i], maxIndex);
            }
        }
    }

    sk_sp<SkPixelRef> pr = SkMallocPixelRef::MakeWithData(info, info.minRowBytes(),
                                                          std::move(ctable), std::move(data));
    if (!pr) {
        return false;
    }
    bitmap->setInfo(pr->info());
    bitmap->setPixelRef(std::move(pr), 0, 0);
    return true;
}

enum {
    SERIALIZE_PIXELTYPE_NONE,
    SERIALIZE_PIXELTYPE_REF_DATA
};

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
        SkASSERT(fPixelRef->info().height() >= (int)this->height() + fPixelRefOrigin.fY);
        SkASSERT(fPixelRef->rowBytes() >= fInfo.minRowBytes());
    } else {
        SkASSERT(nullptr == fColorTable);
    }
}
#endif

#ifndef SK_IGNORE_TO_STRING
#include "SkString.h"
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

    str->appendf(" pixelref:%p", this->pixelRef());
    str->append(")");
}
#endif

///////////////////////////////////////////////////////////////////////////////

bool SkBitmap::requestLock(SkAutoPixmapUnlock* result) const {
    SkASSERT(result);

    SkPixelRef* pr = fPixelRef.get();
    if (nullptr == pr) {
        return false;
    }

    // We have to lock the whole thing (using the pixelref's dimensions) until the api supports
    // a partial lock (with offset/origin). Hence we can't use our fInfo.
    SkPixelRef::LockRequest req = { pr->info().dimensions(), kNone_SkFilterQuality };
    SkPixelRef::LockResult res;
    if (pr->requestLock(req, &res)) {
        SkASSERT(res.fPixels);
        // The bitmap may be a subset of the pixelref's dimensions
        SkASSERT(fPixelRefOrigin.x() + fInfo.width()  <= res.fSize.width());
        SkASSERT(fPixelRefOrigin.y() + fInfo.height() <= res.fSize.height());
        const void* addr = (const char*)res.fPixels + SkColorTypeComputeOffset(fInfo.colorType(),
                                                                               fPixelRefOrigin.x(),
                                                                               fPixelRefOrigin.y(),
                                                                               res.fRowBytes);

        result->reset(SkPixmap(this->info(), addr, res.fRowBytes, res.fCTable),
                      res.fUnlockProc, res.fUnlockContext);
        return true;
    }
    return false;
}

bool SkBitmap::peekPixels(SkPixmap* pmap) const {
    if (fPixels) {
        if (pmap) {
            pmap->reset(fInfo, fPixels, fRowBytes, fColorTable);
        }
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG
void SkImageInfo::validate() const {
    SkASSERT(fWidth >= 0);
    SkASSERT(fHeight >= 0);
    SkASSERT(SkColorTypeIsValid(fColorType));
    SkASSERT(SkAlphaTypeIsValid(fAlphaType));
}
#endif
