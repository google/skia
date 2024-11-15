/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkImage_Picture_DEFINED
#define SkImage_Picture_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkTiledImageUtils.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkImage_Lazy.h"

#include <cstdint>

class GrDirectContext;
class SkCanvas;
class SkColorSpace;
class SkImage;
class SkMatrix;
class SkPaint;
class SkPicture;
class SkSurfaceProps;
struct SkIRect;
struct SkISize;

namespace SkImages { enum class BitDepth; }
namespace skgpu::graphite { class Recorder; }

class SkImage_Picture : public SkImage_Lazy {
public:
    static sk_sp<SkImage> Make(sk_sp<SkPicture> picture, const SkISize& dimensions,
                               const SkMatrix* matrix, const SkPaint* paint,
                               SkImages::BitDepth bitDepth, sk_sp<SkColorSpace> colorSpace,
                               SkSurfaceProps props);

    SkImage_Picture(Validator* validator) : SkImage_Lazy(validator) {}

    SkImage_Base::Type type() const override { return SkImage_Base::Type::kLazyPicture; }

    // This is thread safe. It is a const field set in the constructor.
    const SkSurfaceProps* props() const;

    // Call drawPicture on the provided canvas taking care of any required mutex locking.
    void replay(SkCanvas*) const;

    sk_sp<SkImage> onMakeSubset(GrDirectContext*, const SkIRect&) const override;
    sk_sp<SkImage> onMakeSubset(skgpu::graphite::Recorder*,
                                const SkIRect&,
                                RequiredProperties) const override;

    // If possible, extract key data based on the underlying drawPicture-call's parameters.
    // Takes care of any required mutex locking.
    bool getImageKeyValues(uint32_t keyValues[SkTiledImageUtils::kNumImageKeyValues]) const;
};

#endif
