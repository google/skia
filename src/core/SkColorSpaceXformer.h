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
#include "SkRefCnt.h"
#include "SkTLazy.h"

class SkColorSpaceXformer : public SkRefCnt {
public:
    static sk_sp<SkColorSpaceXformer> Make(sk_sp<SkColorSpace> dst);

    sk_sp<SkImage> apply(const SkImage* src) const;
    const SkPaint* apply(SkTLazy<SkPaint>* dst, const SkPaint* src) const;
    const SkPaint& apply(SkTLazy<SkPaint>* dst, const SkPaint& src) const;
    void apply(SkColor* dst, const SkColor* src, int n) const;

private:
    SkColor apply(SkColor srgb) const;
    sk_sp<SkShader> apply(const SkShader* shader) const;

    SkColorSpaceXformer(sk_sp<SkColorSpace> dst, std::unique_ptr<SkColorSpaceXform> fromSRGB);

    sk_sp<SkColorSpace>                fDst;
    std::unique_ptr<SkColorSpaceXform> fFromSRGB;
};

#endif
