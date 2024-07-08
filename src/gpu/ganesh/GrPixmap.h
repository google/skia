/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPixmap_DEFINED
#define GrPixmap_DEFINED

#include "include/core/SkData.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/private/base/SkTLogic.h"
#include "include/private/base/SkTo.h"
#include "src/gpu/ganesh/GrImageInfo.h"

#include <cstddef>
#include <utility>

class GrColorInfo;
class SkColorSpace;
enum SkAlphaType : int;
enum class GrColorType;

template <typename T, typename DERIVED> class GrPixmapBase {
public:
    const GrImageInfo& info() const { return fInfo; }
    const GrColorInfo& colorInfo() const { return fInfo.colorInfo(); }

    T* addr() const { return fAddr; }
    size_t rowBytes() const { return fRowBytes; }

    bool hasPixels() const { return SkToBool(fAddr); }
    bool ownsPixels() const { return SkToBool(fPixelStorage); }
    sk_sp<SkData> pixelStorage() const { return fPixelStorage; }

    int width() const { return fInfo.width(); }
    int height() const { return fInfo.height(); }
    SkISize dimensions() const { return fInfo.dimensions(); }
    GrColorType colorType() const { return fInfo.colorType(); }
    SkAlphaType alphaType() const { return fInfo.alphaType(); }
    SkColorSpace* colorSpace() const { return fInfo.colorSpace(); }
    sk_sp<SkColorSpace> refColorSpace() const { return fInfo.refColorSpace(); }

    /**
     * Map this pixmap to a rect in a surface of indicated dimensions at offset surfacePt. Clip the
     * logical rectangle to the bounds of the surface. If the rect does not intersect the surface
     * bounds or is empty then return a default GrPixmap. Otherwise, surfacePt is updated to refer
     * to the upper left of the clipped rectangle. The returned pixmap will refer to the portion
     * of the original pixmap inside the surface bounds.
     */
    DERIVED clip(SkISize surfaceDims, SkIPoint* surfacePt) {
        auto bounds = SkIRect::MakeSize(surfaceDims);
        auto rect = SkIRect::MakePtSize(*surfacePt, this->dimensions());
        if (!rect.intersect(bounds)) {
            return {};
        }
        T* addr = static_cast<sknonstd::copy_const_t<char, T>*>(fAddr) +
                  (rect.fTop - surfacePt->fY) * fRowBytes +
                  (rect.fLeft - surfacePt->fX) * fInfo.bpp();
        surfacePt->fX = rect.fLeft;
        surfacePt->fY = rect.fTop;
        return DERIVED{this->info().makeDimensions(rect.size()), addr, fRowBytes};
    }

protected:
    GrPixmapBase() = default;
    GrPixmapBase(const GrPixmapBase& that) = default;
    GrPixmapBase(GrPixmapBase&& that) = default;
    GrPixmapBase& operator=(const GrPixmapBase& that) = default;
    GrPixmapBase& operator=(GrPixmapBase&& that) = default;

    GrPixmapBase(GrImageInfo info, T* addr, size_t rowBytes)
            : fAddr(addr), fRowBytes(rowBytes), fInfo(std::move(info)) {
        if (fRowBytes < fInfo.minRowBytes() || !addr) {
            *this = {};
        }
    }

    GrPixmapBase(GrImageInfo info, sk_sp<SkData> storage, size_t rowBytes)
            : GrPixmapBase(std::move(info), const_cast<void*>(storage->data()), rowBytes) {
        fPixelStorage = std::move(storage);
    }

private:
    T* fAddr = nullptr;
    size_t fRowBytes = 0;
    GrImageInfo fInfo;
    sk_sp<SkData> fPixelStorage;
};

/** A pixmap with mutable pixels. */
class GrPixmap : public GrPixmapBase<void, GrPixmap> {
public:
    GrPixmap() = default;
    GrPixmap(const GrPixmap&) = default;
    GrPixmap(GrPixmap&&) = default;
    GrPixmap& operator=(const GrPixmap&) = default;
    GrPixmap& operator=(GrPixmap&&) = default;

    GrPixmap(GrImageInfo info, void* addr, size_t rowBytes)
            : GrPixmapBase(std::move(info), addr, rowBytes) {}

    /* implicit */ GrPixmap(const SkPixmap& pixmap)
            : GrPixmapBase(pixmap.info(), pixmap.writable_addr(), pixmap.rowBytes()) {}

    /**
     * Returns a GrPixmap that owns its backing store. Copies of the pixmap (as GrPixmap or
     * GrCPixmap) will share ownership.
     */
    static GrPixmap Allocate(const GrImageInfo& info) {
        size_t rb = info.minRowBytes();
        size_t size = info.height()*rb;
        if (!size) {
            return {};
        }
        return GrPixmap(info, SkData::MakeUninitialized(size), rb);
    }

private:
    GrPixmap(GrImageInfo info, sk_sp<SkData> storage, size_t rowBytes)
            : GrPixmapBase(std::move(info), std::move(storage), rowBytes) {}
};

/**
 * A pixmap with immutable pixels. Note that this pixmap need not be the unique owner of the pixels
 * and thus it is context-dependent whether the pixels could be manipulated externally.
 */
class GrCPixmap : public GrPixmapBase<const void, GrCPixmap> {
public:
    GrCPixmap() = default;
    GrCPixmap(const GrCPixmap&) = default;
    GrCPixmap(GrCPixmap&&) = default;
    GrCPixmap& operator=(const GrCPixmap&) = default;
    GrCPixmap& operator=(GrCPixmap&&) = default;

    /* implicit*/ GrCPixmap(const GrPixmap& pixmap) {
        if (auto storage = pixmap.pixelStorage()) {
            *this = GrCPixmap(pixmap.info(), std::move(storage), pixmap.rowBytes());
        } else {
            *this = GrCPixmap(pixmap.info(), pixmap.addr(), pixmap.rowBytes());
        }
    }

    /* implicit */ GrCPixmap(const SkPixmap& pixmap)
            : GrPixmapBase(pixmap.info(), pixmap.addr(), pixmap.rowBytes()) {}

    GrCPixmap(GrImageInfo info, const void* addr, size_t rowBytes)
            : GrPixmapBase(std::move(info), addr, rowBytes) {}

private:
    GrCPixmap(GrImageInfo info, sk_sp<SkData> storage, size_t rowBytes)
            : GrPixmapBase(std::move(info), std::move(storage), rowBytes) {}
};

#endif
