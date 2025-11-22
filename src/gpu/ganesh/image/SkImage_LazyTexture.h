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
class SkRecorder;
struct SkIRect;

/**
 * SkImage_LazyTexture is a GPU-backed SkImage that defers the creation of its backing texture
 * until it is actually required for a draw. It is instantiated with a GrTextureGenerator, which is
 * a factory object that knows how to create the texture on demand.
 *
 * This class is the concrete implementation for SkImages created from platform-specific objects
 * like Android's AHardwareBuffer. In that case, the generator wraps the hardware buffer and the
 * "generation" of the texture is simply the act of importing that buffer into the graphics API
 * (e.g., Vulkan or GL).
 */
class SkImage_LazyTexture final : public SkImage_Lazy {
public:
    explicit SkImage_LazyTexture(SkImage_Lazy::Validator* validator) : SkImage_Lazy(validator) {}

    bool readPixelsProxy(GrDirectContext*, const SkPixmap&) const override;

    SkImage_Base::Type type() const override { return SkImage_Base::Type::kLazyTexture; }

    sk_sp<SkImage> onMakeSubset(SkRecorder*, const SkIRect&, RequiredProperties) const override;
};

#endif
