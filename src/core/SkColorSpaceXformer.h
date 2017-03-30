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
    const SkPaint* apply(const SkPaint* src);
    const SkPaint& apply(const SkPaint& src);
    void apply(SkColor dst[], const SkColor src[], int n);

private:
    SkColor apply(SkColor srgb);
    sk_sp<SkShader> apply(const SkShader* shader);

    SkColorSpaceXformer() {}

    sk_sp<SkColorSpace>                fDst;
    std::unique_ptr<SkColorSpaceXform> fFromSRGB;
    SkPaint                            fDstPaint;
};

#endif
