/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorSpaceXformer_DEFINED
#define SkColorSpaceXformer_DEFINED

#include "SkColorSpaceXform.h"
#include "SkImage.h"
#include "SkShader.h"

class SkColorSpaceXformer : public SkNoncopyable {
public:
    static std::unique_ptr<SkColorSpaceXformer> Make(sk_sp<SkColorSpace> dst);

    sk_sp<SkImage> apply(const SkImage* src);
    sk_sp<SkImage> apply(const SkBitmap& bitmap);
    sk_sp<SkColorFilter> apply(const SkColorFilter* shader);
    const SkPaint* apply(const SkPaint* src);
    const SkPaint& apply(const SkPaint& src);
    void apply(SkColor dst[], const SkColor src[], int n);
    SkColor apply(SkColor srgb);

private:
    sk_sp<SkShader> apply(const SkShader* shader);

    SkColorSpaceXformer() {}

    sk_sp<SkColorSpace>                fDst;
    std::unique_ptr<SkColorSpaceXform> fFromSRGB;
    SkPaint                            fDstPaint;
};

#endif
