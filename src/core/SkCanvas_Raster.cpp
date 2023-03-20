/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkRasterHandleAllocator.h"
#include "include/core/SkRefCnt.h"
#include "src/core/SkBitmapDevice.h"
#include "src/core/SkDevice.h"
#include "src/core/SkSurfacePriv.h"
#include "src/text/GlyphRun.h" // IWYU pragma: keep

#include <memory>
#include <utility>

class SkBitmap;
class SkSurfaceProps;

#if defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)
#include "include/core/SkBitmap.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImageInfo.h"

#endif

SkCanvas::SkCanvas(const SkBitmap& bitmap, const SkSurfaceProps& props)
        : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage)), fProps(props) {
    this->init(sk_make_sp<SkBitmapDevice>(bitmap, fProps));
}

SkCanvas::SkCanvas(const SkBitmap& bitmap,
                   std::unique_ptr<SkRasterHandleAllocator> alloc,
                   SkRasterHandleAllocator::Handle hndl,
                   const SkSurfaceProps* props)
        : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage))
        , fProps(SkSurfacePropsCopyOrDefault(props))
        , fAllocator(std::move(alloc)) {
    this->init(sk_make_sp<SkBitmapDevice>(bitmap, fProps, hndl));
}

SkCanvas::SkCanvas(const SkBitmap& bitmap) : SkCanvas(bitmap, nullptr, nullptr, nullptr) {}

#if defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)
SkCanvas::SkCanvas(const SkBitmap& bitmap, ColorBehavior)
        : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage)) {
    SkBitmap tmp(bitmap);
    *const_cast<SkImageInfo*>(&tmp.info()) = tmp.info().makeColorSpace(nullptr);
    this->init(sk_make_sp<SkBitmapDevice>(tmp, fProps));
}
#endif

