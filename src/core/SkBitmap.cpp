/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"

#include "include/core/SkColorSpace.h" // IWYU pragma: keep
#include "include/core/SkColorType.h"
#include "include/core/SkImage.h"
#include "include/core/SkMallocPixelRef.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPixelRef.h"
#include "include/core/SkRect.h"
#include "include/core/SkShader.h"
#include "include/core/SkTileMode.h"
#include "include/private/base/SkAlign.h"
#include "include/private/base/SkTFitsIn.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkTo.h"
#include "src/core/SkConvertPixels.h"
#include "src/core/SkImageInfoPriv.h"
#include "src/core/SkImagePriv.h"
#include "src/core/SkMask.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkMipmap.h"
#include "src/core/SkPixelRefPriv.h"
#include "src/core/SkWritePixelsRec.h"
#include "src/shaders/SkImageShader.h"

#include <cstring>
#include <utility>
class SkMaskFilter;

static bool reset_return_false(SkBitmap* bm) {
    bm->reset();
    return false;
}

SkBitmap::SkBitmap() {}

SkBitmap::SkBitmap(const SkBitmap& src)
    : fPixelRef      (src.fPixelRef)
    , fPixmap        (src.fPixmap)
    , fMips          (src.fMips)
{
    SkDEBUGCODE(src.validate();)
    SkDEBUGCODE(this->validate();)
}

SkBitmap::SkBitmap(SkBitmap&& other)
    : fPixelRef      (std::move(other.fPixelRef))
    , fPixmap        (std::move(other.fPixmap))
    , fMips          (std::move(other.fMips))
{
    SkASSERT(!other.fPixelRef);
    other.fPixmap.reset();
}

SkBitmap::~SkBitmap() {}

SkBitmap& SkBitmap::operator=(const SkBitmap& src) {
    if (this != &src) {
        fPixelRef       = src.fPixelRef;
        fPixmap         = src.fPixmap;
        fMips           = src.fMips;
    }
    SkDEBUGCODE(this->validate();)
    return *this;
}

SkBitmap& SkBitmap::operator=(SkBitmap&& other) {
    if (this != &other) {
        fPixelRef       = std::move(other.fPixelRef);
        fPixmap         = std::move(other.fPixmap);
        fMips           = std::move(other.fMips);
        SkASSERT(!other.fPixelRef);
        other.fPixmap.reset();
    }
    return *this;
}

void SkBitmap::swap(SkBitmap& other) {
    using std::swap;
    swap(*this, other);
    SkDEBUGCODE(this->validate();)
}

void SkBitmap::reset() {
    fPixelRef = nullptr;  // Free pixels.
    fPixmap.reset();
    fMips.reset();
}

void SkBitmap::getBounds(SkRect* bounds) const {
    SkASSERT(bounds);
    *bounds = SkRect::Make(this->dimensions());
}

void SkBitmap::getBounds(SkIRect* bounds) const {
    SkASSERT(bounds);
    *bounds = fPixmap.bounds();
}

SkColorSpace* SkBitmap::colorSpace() const { return fPixmap.colorSpace(); }

sk_sp<SkColorSpace> SkBitmap::refColorSpace() const { return fPixmap.info().refColorSpace(); }

///////////////////////////////////////////////////////////////////////////////

bool SkBitmap::setInfo(const SkImageInfo& info, size_t rowBytes) {
    SkAlphaType newAT = info.alphaType();
    if (!SkColorTypeValidateAlphaType(info.colorType(), info.alphaType(), &newAT)) {
        return reset_return_false(this);
    }
    // don't look at info.alphaType(), since newAT is the real value...

    // require that rowBytes fit in 31bits
    int64_t mrb = info.minRowBytes64();
    if (!SkTFitsIn<int32_t>(mrb)) {
        return reset_return_false(this);
    }
    if (!SkTFitsIn<int32_t>(rowBytes)) {
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

    fPixelRef = nullptr;  // Free pixels.
    fPixmap.reset(info.makeAlphaType(newAT), nullptr, SkToU32(rowBytes));
    SkDEBUGCODE(this->validate();)
    return true;
}



bool SkBitmap::setAlphaType(SkAlphaType newAlphaType) {
    if (!SkColorTypeValidateAlphaType(this->colorType(), newAlphaType, &newAlphaType)) {
        return false;
    }
    if (this->alphaType() != newAlphaType) {
        auto newInfo = fPixmap.info().makeAlphaType(newAlphaType);
        fPixmap.reset(std::move(newInfo), fPixmap.addr(), fPixmap.rowBytes());
    }
    SkDEBUGCODE(this->validate();)
    return true;
}

SkIPoint SkBitmap::pixelRefOrigin() const {
    const char* addr = (const char*)fPixmap.addr();
    const char* pix = (const char*)(fPixelRef ? fPixelRef->pixels() : nullptr);
    size_t rb = this->rowBytes();
    if (!pix || 0 == rb) {
        return {0, 0};
    }
    SkASSERT(this->bytesPerPixel() > 0);
    SkASSERT(this->bytesPerPixel() == (1 << this->shiftPerPixel()));
    SkASSERT(addr >= pix);
    size_t off = addr - pix;
    return {SkToS32((off % rb) >> this->shiftPerPixel()), SkToS32(off / rb)};
}

void SkBitmap::setPixelRef(sk_sp<SkPixelRef> pr, int dx, int dy) {
#ifdef SK_DEBUG
    if (pr) {
        if (kUnknown_SkColorType != this->colorType()) {
            SkASSERT(dx >= 0 && this->width() + dx <= pr->width());
            SkASSERT(dy >= 0 && this->height() + dy <= pr->height());
        }
    }
#endif
    fPixelRef = kUnknown_SkColorType != this->colorType() ? std::move(pr) : nullptr;
    void* p = nullptr;
    size_t rowBytes = this->rowBytes();
    // ignore dx,dy if there is no pixelref
    if (fPixelRef) {
        rowBytes = fPixelRef->rowBytes();
        // TODO(reed):  Enforce that PixelRefs must have non-null pixels.
        p = fPixelRef->pixels();
        if (p) {
            p = (char*)p + dy * rowBytes + dx * this->bytesPerPixel();
        }
    }
    fPixmap.reset(fPixmap.info(), p, rowBytes);
    SkDEBUGCODE(this->validate();)
}

void SkBitmap::setPixels(void* p) {
    if (kUnknown_SkColorType == this->colorType()) {
        p = nullptr;
    }
    size_t rb = this->rowBytes();
    fPixmap.reset(fPixmap.info(), p, rb);
    fPixelRef = p ? sk_make_sp<SkPixelRef>(this->width(), this->height(), p, rb) : nullptr;
    SkDEBUGCODE(this->validate();)
}

bool SkBitmap::tryAllocPixels(Allocator* allocator) {
    HeapAllocator stdalloc;

    if (nullptr == allocator) {
        allocator = &stdalloc;
    }
    return allocator->allocPixelRef(this);
}

bool SkBitmap::tryAllocN32Pixels(int width, int height, bool isOpaque) {
    SkImageInfo info = SkImageInfo::MakeN32(width, height,
            isOpaque ? kOpaque_SkAlphaType : kPremul_SkAlphaType);
    return this->tryAllocPixels(info);
}

void SkBitmap::allocN32Pixels(int width, int height, bool isOpaque) {
    SkImageInfo info = SkImageInfo::MakeN32(width, height,
                                        isOpaque ? kOpaque_SkAlphaType : kPremul_SkAlphaType);
    this->allocPixels(info);
}

void SkBitmap::allocPixels() {
    this->allocPixels((Allocator*)nullptr);
}

void SkBitmap::allocPixels(Allocator* allocator) {
    if (!this->tryAllocPixels(allocator)) {
        const SkImageInfo& info = this->info();
        SK_ABORT("SkBitmap::tryAllocPixels failed "
                 "ColorType:%d AlphaType:%d [w:%d h:%d] rb:%zu",
                 info.colorType(), info.alphaType(), info.width(), info.height(), this->rowBytes());
    }
}

void SkBitmap::allocPixelsFlags(const SkImageInfo& info, uint32_t flags) {
    SkASSERT_RELEASE(this->tryAllocPixelsFlags(info, flags));
}

void SkBitmap::allocPixels(const SkImageInfo& info, size_t rowBytes) {
    SkASSERT_RELEASE(this->tryAllocPixels(info, rowBytes));
}

void SkBitmap::allocPixels(const SkImageInfo& info) {
    this->allocPixels(info, info.minRowBytes());
}

///////////////////////////////////////////////////////////////////////////////

bool SkBitmap::tryAllocPixels(const SkImageInfo& requestedInfo, size_t rowBytes) {
    if (!this->setInfo(requestedInfo, rowBytes)) {
        return reset_return_false(this);
    }

    // setInfo may have corrected info (e.g. 565 is always opaque).
    const SkImageInfo& correctedInfo = this->info();
    if (kUnknown_SkColorType == correctedInfo.colorType()) {
        return true;
    }
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

    sk_sp<SkPixelRef> pr = SkMallocPixelRef::MakeAllocate(correctedInfo,
                                                          correctedInfo.minRowBytes());
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
    this->setPixelRef(
            SkMakePixelRefWithProc(correctedInfo.width(), correctedInfo.height(),
                                   rb, pixels, releaseProc, context), 0, 0);
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
    const SkImageInfo& info = dst->info();
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

void* SkBitmap::getAddr(int x, int y) const {
    SkASSERT((unsigned)x < (unsigned)this->width());
    SkASSERT((unsigned)y < (unsigned)this->height());

    char* base = (char*)this->getPixels();
    if (base) {
        base += (y * this->rowBytes()) + (x << this->shiftPerPixel());
    }
    return base;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SkBitmap::erase(SkColor4f c, SkColorSpace* colorSpace, const SkIRect& area) const {
    SkDEBUGCODE(this->validate();)

    if (kUnknown_SkColorType == this->colorType()) {
        // TODO: can we ASSERT that we never get here?
        return; // can't erase. Should we bzero so the memory is not uninitialized?
    }

    SkPixmap result;
    if (!this->peekPixels(&result)) {
        return;
    }

    if (result.erase(c, colorSpace, &area)) {
        this->notifyPixelsChanged();
    }
}

void SkBitmap::erase(SkColor c, const SkIRect& area) const {
    this->erase(SkColor4f::FromColor(c), nullptr, area);
}

void SkBitmap::erase(SkColor4f c, const SkIRect& area) const {
    this->erase(c, nullptr, area);
}

void SkBitmap::eraseColor(SkColor4f c, SkColorSpace* colorSpace) const {
    this->erase(c, colorSpace, SkIRect::MakeWH(this->width(), this->height()));
}

void SkBitmap::eraseColor(SkColor c) const {
    this->erase(SkColor4f::FromColor(c), nullptr, SkIRect::MakeWH(this->width(), this->height()));
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

bool SkBitmap::extractSubset(SkBitmap* result, const SkIRect& subset) const {
    SkDEBUGCODE(this->validate();)

    if (nullptr == result || !fPixelRef) {
        return false;   // no src pixels
    }

    SkIRect srcRect, r;
    srcRect.setWH(this->width(), this->height());
    if (!r.intersect(srcRect, subset)) {
        return false;   // r is empty (i.e. no intersection)
    }

    // If the upper left of the rectangle was outside the bounds of this SkBitmap, we should have
    // exited above.
    SkASSERT(static_cast<unsigned>(r.fLeft) < static_cast<unsigned>(this->width()));
    SkASSERT(static_cast<unsigned>(r.fTop) < static_cast<unsigned>(this->height()));

    SkBitmap dst;
    dst.setInfo(this->info().makeDimensions(r.size()), this->rowBytes());

    if (fPixelRef) {
        SkIPoint origin = this->pixelRefOrigin();
        // share the pixelref with a custom offset
        dst.setPixelRef(fPixelRef, origin.x() + r.fLeft, origin.y() + r.fTop);
    }
    SkDEBUGCODE(dst.validate();)

    // we know we're good, so commit to result
    result->swap(dst);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool SkBitmap::readPixels(const SkImageInfo& requestedDstInfo, void* dstPixels, size_t dstRB,
                          int x, int y) const {
    SkPixmap src;
    if (!this->peekPixels(&src)) {
        return false;
    }
    return src.readPixels(requestedDstInfo, dstPixels, dstRB, x, y);
}

bool SkBitmap::readPixels(const SkPixmap& dst, int srcX, int srcY) const {
    return this->readPixels(dst.info(), dst.writable_addr(), dst.rowBytes(), srcX, srcY);
}

bool SkBitmap::writePixels(const SkPixmap& src, int dstX, int dstY) {
    if (!SkImageInfoValidConversion(this->info(), src.info())) {
        return false;
    }

    SkWritePixelsRec rec(src.info(), src.addr(), src.rowBytes(), dstX, dstY);
    if (!rec.trim(this->width(), this->height())) {
        return false;
    }

    void* dstPixels = this->getAddr(rec.fX, rec.fY);
    const SkImageInfo dstInfo = this->info().makeDimensions(rec.fInfo.dimensions());
    if (!SkConvertPixels(dstInfo,     dstPixels, this->rowBytes(),
                         rec.fInfo, rec.fPixels,   rec.fRowBytes)) {
        return false;
    }
    this->notifyPixelsChanged();
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
    return SkConvertPixels(SkImageInfo::MakeA8(pmap.width(), pmap.height()), alpha, alphaRowBytes,
                           pmap.info(), pmap.addr(), pmap.rowBytes());
}

bool SkBitmap::extractAlpha(SkBitmap* dst, const SkPaint* paint,
                            Allocator *allocator, SkIPoint* offset) const {
    SkDEBUGCODE(this->validate();)

    SkBitmap    tmpBitmap;
    SkMatrix    identity;
    SkMask      srcM, dstM;

    if (this->width() == 0 || this->height() == 0) {
        return false;
    }
    srcM.fBounds.setWH(this->width(), this->height());
    srcM.fRowBytes = SkAlign4(this->width());
    srcM.fFormat = SkMask::kA8_Format;

    SkMaskFilter* filter = paint ? paint->getMaskFilter() : nullptr;

    // compute our (larger?) dst bounds if we have a filter
    if (filter) {
        identity.reset();
        if (!as_MFB(filter)->filterMask(&dstM, srcM, identity, nullptr)) {
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
    if (!as_MFB(filter)->filterMask(&dstM, srcM, identity, nullptr)) {
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

#ifdef SK_DEBUG
void SkBitmap::validate() const {
    this->info().validate();

    SkASSERT(this->info().validRowBytes(this->rowBytes()));

    if (fPixelRef && fPixelRef->pixels()) {
        SkASSERT(this->getPixels());
    } else {
        SkASSERT(!this->getPixels());
    }

    if (this->getPixels()) {
        SkASSERT(fPixelRef);
        SkASSERT(fPixelRef->rowBytes() == this->rowBytes());
        SkIPoint origin = this->pixelRefOrigin();
        SkASSERT(origin.fX >= 0);
        SkASSERT(origin.fY >= 0);
        SkASSERT(fPixelRef->width() >= (int)this->width() + origin.fX);
        SkASSERT(fPixelRef->height() >= (int)this->height() + origin.fY);
        SkASSERT(fPixelRef->rowBytes() >= this->info().minRowBytes());
    }
}
#endif

///////////////////////////////////////////////////////////////////////////////

bool SkBitmap::peekPixels(SkPixmap* pmap) const {
    if (this->getPixels()) {
        if (pmap) {
            *pmap = fPixmap;
        }
        return true;
    }
    return false;
}

sk_sp<SkImage> SkBitmap::asImage() const {
    return SkImage::MakeFromBitmap(*this);
}

sk_sp<SkShader> SkBitmap::makeShader(const SkSamplingOptions& sampling,
                                     const SkMatrix& lm) const {
    return this->makeShader(SkTileMode::kClamp, SkTileMode::kClamp,
                            sampling, &lm);
}

sk_sp<SkShader> SkBitmap::makeShader(const SkSamplingOptions& sampling,
                                     const SkMatrix* lm) const {
    return this->makeShader(SkTileMode::kClamp, SkTileMode::kClamp,
                            sampling, lm);
}

sk_sp<SkShader> SkBitmap::makeShader(SkTileMode tmx, SkTileMode tmy,
                                     const SkSamplingOptions& sampling,
                                     const SkMatrix& lm) const {
    if (!lm.invert(nullptr)) {
        return nullptr;
    }
    return SkImageShader::Make(SkMakeImageFromRasterBitmap(*this, kIfMutable_SkCopyPixelsMode),
                               tmx, tmy, sampling, &lm);
}

sk_sp<SkShader> SkBitmap::makeShader(SkTileMode tmx, SkTileMode tmy,
                                     const SkSamplingOptions& sampling,
                                     const SkMatrix* lm) const {
    if (lm && !lm->invert(nullptr)) {
        return nullptr;
    }
    return SkImageShader::Make(SkMakeImageFromRasterBitmap(*this, kIfMutable_SkCopyPixelsMode),
                               tmx, tmy, sampling, lm);
}
