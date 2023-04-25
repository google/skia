/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkImage_Picture_DEFINED
#define SkImage_Picture_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkImage_Lazy.h"

class SkColorSpace;
class SkImage;
class SkMatrix;
class SkPaint;
class SkPicture;
class SkPictureImageGenerator;
class SkSurfaceProps;
struct SkISize;

namespace SkImages { enum class BitDepth; }

class SkImage_Picture : public SkImage_Lazy {
public:
    static sk_sp<SkImage> Make(sk_sp<SkPicture> picture, const SkISize& dimensions,
                               const SkMatrix* matrix, const SkPaint* paint,
                               SkImages::BitDepth bitDepth, sk_sp<SkColorSpace> colorSpace,
                               SkSurfaceProps props);

    SkImage_Picture(Validator* validator) : SkImage_Lazy(validator) {}

     SkImage_Base::Type type() const override { return SkImage_Base::Type::kLazyPicture; }

     // These are not necessarily thread-safe. Be sure to grab the mutex from the shared
     // generator before accessing them.
     SkPicture* picture() const;
     SkMatrix* matrix() const;
     SkPaint* paint() const;
     SkSurfaceProps* props() const;

private:
    SkPictureImageGenerator* gen() const;
};

#endif
