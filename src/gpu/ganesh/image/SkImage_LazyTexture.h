/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkImage_LazyTexture_DEFINED
#define SkImage_LazyTexture_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "src/image/SkImage_Lazy.h"

class GrDirectContext;
class SkImage;
class SkPixmap;
struct SkIRect;

class SkImage_LazyTexture final : public SkImage_Lazy {
public:
    SkImage_LazyTexture(SkImage_Lazy::Validator *validator) : SkImage_Lazy(validator) {}

    bool readPixelsProxy(GrDirectContext*, const SkPixmap&) const override;

    sk_sp<SkImage> onMakeSubset(GrDirectContext*, const SkIRect&) const override;
};

#endif
