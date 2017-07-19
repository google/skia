/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAtomics.h"
#include "SkBitmap.h"
#include "SkColorPriv.h"
#include "SkColorTable.h"
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
    : fPixels        (nullptr)
    , fPixelRefOrigin{0, 0}
    , fRowBytes      (0)
    , fFlags         (0) {}

SkBitmap::SkBitmap(const SkBitmap& src)
    : fPixelRef      (src.fPixelRef)
    , fPixels        (src.fPixels)
    , fPixelRefOrigin(src.fPixelRefOrigin)
    , fInfo          (src.fInfo)
    , fRowBytes      (src.fRowBytes)
    , fFlags         (src.fFlags)
{
    SkDEBUGCODE(src.validate();)
    SkDEBUGCODE(this->validate();)
}

SkBitmap::SkBitmap(SkBitmap&& other)
    : fPixelRef      (std::move(other.fPixelRef))
    , fPixels                  (other.fPixels)
    , fPixelRefOrigin          (other.fPixelRefOrigin)
    , fInfo          (std::move(other.fInfo))
    , fRowBytes                (other.fRowBytes)
    , fFlags                   (other.fFlags)
{
    SkASSERT(!other.fPixelRef);
    other.fInfo.reset();
    other.fPixels         = nullptr;
    other.fPixelRefOrigin = SkIPoint{0, 0};
    other.fRowBytes       = 0;
    other.fFlags          = 0;
}

SkBitmap::~SkBitmap() {}

SkBitmap& SkBitmap::operator=(const SkBitmap& src) {
    if (this != &src) {
        fPixelRef       = src.fPixelRef;
        fPixels         = src.fPixels;
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
        fPixelRef       = std::move(other.fPixelRef);
        fInfo           = std::move(other.fInfo);
        fPixels         = other.fPixels;
        fPixelRefOrigin = other.fPixelRefOrigin;
        fRowBytes       = other.fRowBytes;
        fFlags          = other.fFlags;
        SkASSERT(!other.fPixelRef);
        other.fInfo.reset();
        other.fPixels         = nullptr;
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
    SkDEBUGCODE(this->validate();)
    return true;
}

bool SkBitmap::setAlphaType(SkAlphaType newAlphaType) {
    if (!SkColorTypeValidateAlphaType(fInfo.colorType(), newAlphaType, &newAlphaType)) {
        return false;
    }
    if (fInfo.alphaType() != newAlphaType) {
        fInfo = fInfo.makeAlphaType(newAlphaType);
    }
    SkDEBUGCODE(this->validate();)
    return true;
}

void SkBitmap::updatePixelsFromRef() {
    void* p = nullptr;
    if (fPixelRef) {
        // wish we could assert that a pixelref *always* has pixels
        p = fPixelRef->pixels();
        if (p) {
            SkASSERT(fRowBytes == fPixelRef->rowBytes());
            p = (char*)p
                + fPixelRefOrigin.fY * fRowBytes
                + fPixelRefOrigin.fX * fInfo.bytesPerPixel();
        }
    }
    fPixels = p;
}

void SkBitmap::setPixelRef(sk_sp<SkPixelRef> pr, int dx, int dy) {
#ifdef SK_DEBUG
    if (pr) {
        if (kUnknown_SkColorType != fInfo.colorType()) {
            SkASSERT(fInfo.width() + dx <= pr->width());
            SkASSERT(fInfo.height() + dy <= pr->height());
        }
    }
#endif

    fPixelRef = std::move(pr);
    if (fPixelRef) {
        fPixelRefOrigin.set(SkTPin(dx, 0, fPixelRef->width()), SkTPin(dy, 0, fPixelRef->height()));
        this->updatePixelsFromRef();
    } else {
        // ignore dx,dy if there is no pixelref
        fPixelRefOrigin.setZero();
        fPixels = nullptr;
    }

    SkDEBUGCODE(this->validate();)
}

void SkBitmap::setPixels(void* p) {
    if (nullptr == p) {
        this->setPixelRef(nullptr, 0, 0);
        return;
    }

    if (kUnknown_SkColorType == fInfo.colorType()) {
        this->setPixelRef(nullptr, 0, 0);
        return;
    }

    this->setPixelRef(SkMallocPixelRef::MakeDirect(fInfo, p, fRowBytes), 0, 0);
    if (!fPixelRef) {
        return;
    }
    SkDEBUGCODE(this->validate();)
}

bool SkBitmap::tryAllocPixels(Allocator* allocator) {
    HeapAllocator stdalloc;

    if (nullptr == allocator) {
        allocator = &stdalloc;
    }
    return allocator->allocPixelRef(this);
}

///////////////////////////////////////////////////////////////////////////////

bool SkBitmap::tryAllocPixels(const SkImageInfo& requestedInfo, size_t rowBytes) {
    if (!this->setInfo(requestedInfo, rowBytes)) {
        return reset_return_false(this);
    }

    // setInfo may have corrected info (e.g. 565 is always opaque).
    const SkImageInfo& correctedInfo = this->info();
    // setInfo may have computed a valid rowbytes if 0 were passed in
    rowBytes = this->rowBytes();

    sk_sp<SkPixelRef> pr = SkMallocPixelRef::MakeAllocate(correctedInfo, rowBytes);
    if (!pr) {
        return reset_return_false(this);
    }
    this->setPixelRef(std::move(pr), 0, 0);
    if (nullptr == this->getPixels()) {
        return reset_return_false(this);
    }
    SkDEBUGCODE(this->validate();)
    return true;
}

bool SkBitmap::tryAllocPixelsFlags(const SkImageInfo& requestedInfo, uint32_t allocFlags) {
    if (!this->setInfo(requestedInfo)) {
        return reset_return_false(this);
    }

    // setInfo may have corrected info (e.g. 565 is always opaque).
    const SkImageInfo& correctedInfo = this->info();

    sk_sp<SkPixelRef> pr = (allocFlags & kZeroPixels_AllocFlag) ?
        SkMallocPixelRef::MakeZeroed(correctedInfo, correctedInfo.minRowBytes()) :
        SkMallocPixelRef::MakeAllocate(correctedInfo, correctedInfo.minRowBytes());
    if (!pr) {
        return reset_return_false(this);
    }
    this->setPixelRef(std::move(pr), 0, 0);
    if (nullptr == this->getPixels()) {
        return reset_return_false(this);
    }
    SkDEBUGCODE(this->validate();)
    return true;
}

static void invoke_release_proc(void (*proc)(void* pixels, void* ctx), void* pixels, void* ctx) {
    if (proc) {
        proc(pixels, ctx);
    }
}

bool SkBitmap::installPixels(const SkImageInfo& requestedInfo, void* pixels, size_t rb,
                             void (*releaseProc)(void* addr, void* context), void* context) {
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

    sk_sp<SkPixelRef> pr = SkMallocPixelRef::MakeWithProc(correctedInfo, rb, pixels,
                                                          releaseProc, context);
    if (!pr) {
        this->reset();
        return false;
    }

    this->setPixelRef(std::move(pr), 0, 0);
    SkDEBUGCODE(this->validate();)
    return true;
}

bool SkBitmap::installPixels(const SkPixmap& pixmap) {
    return this->installPixels(pixmap.info(), pixmap.writable_addr(), pixmap.rowBytes(),
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
    fPixelRef = nullptr;
    fPixelRefOrigin.setZero();
    fPixels = nullptr;
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
bool SkBitmap::HeapAllocator::allocPixelRef(SkBitmap* dst) {
    const SkImageInfo info = dst->info();
    if (kUnknown_SkColorType == info.colorType()) {
//        SkDebugf("unsupported config for info %d\n", dst->config());
        return false;
    }

    sk_sp<SkPixelRef> pr = SkMallocPixelRef::MakeAllocate(info, dst->rowBytes());
    if (!pr) {
        return false;
    }

    dst->setPixelRef(std::move(pr), 0, 0);
    SkDEBUGCODE(dst->validate();)
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
            // TODO: can we ASSERT that we never get here?
            return; // can't erase. Should we bzero so the memory is not uninitialized?
        default:
            break;
    }

    SkPixmap result;
    if (!this->peekPixels(&result)) {
        return;
    }

    if (result.erase(c, area)) {
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

bool SkBitmap::readPixels(const SkImageInfo& requestedDstInfo, void* dstPixels, size_t dstRB,
                          int x, int y, SkTransferFunctionBehavior behavior) const {
    SkPixmap src;
    if (!this->peekPixels(&src)) {
        return false;
    }
    return src.readPixels(requestedDstInfo, dstPixels, dstRB, x, y, behavior);
}

bool SkBitmap::readPixels(const SkPixmap& dst, int srcX, int srcY) const {
    return this->readPixels(dst.info(), dst.writable_addr(), dst.rowBytes(), srcX, srcY);
}

bool SkBitmap::writePixels(const SkPixmap& src, int dstX, int dstY,
                           SkTransferFunctionBehavior behavior) {
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
                    nullptr, behavior);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

static bool GetBitmapAlpha(const SkBitmap& src, uint8_t* SK_RESTRICT alpha, int alphaRowBytes) {
    SkASSERT(alpha != nullptr);
    SkASSERT(alphaRowBytes >= src.width());

    SkPixmap pmap;
    if (!src.peekPixels(&pmap)) {
        for (int y = 0; y < src.height(); ++y) {
            memset(alpha, 0, src.width());
            alpha += alphaRowBytes;
        }
        return false;
    }
    SkConvertPixels(SkImageInfo::MakeA8(pmap.width(), pmap.height()), alpha, alphaRowBytes,
                    pmap.info(), pmap.addr(), pmap.rowBytes(), nullptr,
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
        if (!tmpBitmap.tryAllocPixels(allocator)) {
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
    if (!tmpBitmap.tryAllocPixels(allocator)) {
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
    // no colortable
    buffer->writeBool(false);
}

void SkBitmap::WriteRawPixels(SkWriteBuffer* buffer, const SkBitmap& bitmap) {
    const SkImageInfo info = bitmap.info();
    if (0 == info.width() || 0 == info.height() || bitmap.isNull()) {
        buffer->writeUInt(0); // instead of snugRB, signaling no pixels
        return;
    }

    SkPixmap result;
    if (!bitmap.peekPixels(&result)) {
        buffer->writeUInt(0); // instead of snugRB, signaling no pixels
        return;
    }

    write_raw_pixels(buffer, result);
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

    if (buffer->readBool()) {
        sk_sp<SkColorTable> ctable = SkColorTable::Create(*buffer);
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
                                                          std::move(data));
    if (!pr) {
        return false;
    }
    bitmap->setInfo(info);
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

    if (fPixelRef && fPixelRef->pixels()) {
        SkASSERT(fPixels);
    } else {
        SkASSERT(!fPixels);
    }

    if (fPixels) {
        SkASSERT(fPixelRef);
        SkASSERT(fPixelRef->rowBytes() == fRowBytes);
        SkASSERT(fPixelRefOrigin.fX >= 0);
        SkASSERT(fPixelRefOrigin.fY >= 0);
        SkASSERT(fPixelRef->width() >= (int)this->width() + fPixelRefOrigin.fX);
        SkASSERT(fPixelRef->height() >= (int)this->height() + fPixelRefOrigin.fY);
        SkASSERT(fPixelRef->rowBytes() >= fInfo.minRowBytes());
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

bool SkBitmap::peekPixels(SkPixmap* pmap) const {
    if (fPixels) {
        if (pmap) {
            pmap->reset(fInfo, fPixels, fRowBytes);
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
