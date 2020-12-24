/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPixmap_DEFINED
#define GrPixmap_DEFINED

#include "include/core/SkPixmap.h"
#include "src/gpu/GrImageInfo.h"

class GrPixmap {
public:
    GrPixmap() = default;
    GrPixmap(const GrPixmap&) = default;
    GrPixmap(GrPixmap&&) = default;
    GrPixmap& operator=(const GrPixmap&) = default;
    GrPixmap& operator=(GrPixmap&&) = default;

    GrPixmap(GrImageInfo info, void* addr, size_t rowBytes)
            : fAddr(addr), fRowBytes(rowBytes), fInfo(std::move(info)) {
        if (fRowBytes < info.minRowBytes() || !addr) {
            *this = {};
        }
    }
    /* implicit */ GrPixmap(const SkPixmap& pixmap)
            : GrPixmap(pixmap.info(), pixmap.writable_addr(), pixmap.rowBytes()) {}

    const GrImageInfo& info() const { return fInfo; }
    void* addr() const { return fAddr; }
    size_t rowBytes() const { return fRowBytes; }

    int width() const { return fInfo.width(); }
    int height() const { return fInfo.height(); }
    SkISize dimensions() const { return fInfo.dimensions(); }
    GrColorType colorType() const { return fInfo.colorType(); }

private:
    void* fAddr = nullptr;
    size_t fRowBytes = 0;
    GrImageInfo fInfo;
};

#endif
