/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef YUVUtils_DEFINED
#define YUVUtils_DEFINED

#include "SkAutoMalloc.h"
#include "SkImage.h"
#include "SkYUVAIndex.h"
#include "SkYUVASizeInfo.h"

class SkData;

namespace sk_gpu_test {

// Utility that decodes a JPEG but preserves the YUVA8 planes in the image, and uses
// MakeFromYUVAPixmaps to create a GPU multiplane YUVA image for a context. It extracts the planar
// data once, and lazily creates the actual SkImage when the GrContext is provided (and refreshes
// the image if the context has changed, as in Viewer)
class LazyYUVImage {
public:
    // Returns null if the data could not be extracted into YUVA8 planes
    static std::unique_ptr<LazyYUVImage> Make(sk_sp<SkData> data);

    sk_sp<SkImage> refImage(GrContext* context);

    const SkImage* getImage(GrContext* context);

private:
    // Decoded YUV data
    SkYUVASizeInfo fSizeInfo;
    SkYUVColorSpace fColorSpace;
    SkYUVAIndex fComponents[SkYUVAIndex::kIndexCount];
    SkAutoMalloc fPlaneData;
    SkPixmap fPlanes[SkYUVASizeInfo::kMaxCount];

    // Memoized SkImage formed with planes
    sk_sp<SkImage> fYUVImage;
    uint32_t fOwningContextID;

    LazyYUVImage() : fOwningContextID(SK_InvalidGenID) {}

    bool reset(sk_sp<SkData> data);

    bool ensureYUVImage(GrContext* context);
};

} // namespace sk_gpu_test

#endif // YUVUtils_DEFINED
