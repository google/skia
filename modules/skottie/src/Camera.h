/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieCamera_DEFINED
#define SkottieCamera_DEFINED

#include "modules/skottie/src/Transform.h"

#include "include/core/SkMatrix44.h"

namespace skottie {
namespace internal {

class CameraAdapter final : public TransformAdapter3D {
public:
    enum class Type {
        kOneNode, // implicitly facing forward (decreasing z), does not auto-orient
        kTwoNode, // explicitly facing a POI (the anchor point), auto-orients
    };

    static sk_sp<CameraAdapter> MakeDefault(const SkSize& viewport_size);

    CameraAdapter(const SkSize& viewport_size, Type);
    ~CameraAdapter() override;

    ADAPTER_PROPERTY(Zoom, SkScalar, 0)

private:
    SkMatrix44 totalMatrix() const override;

    SkPoint3 poi() const;

    const SkSize fViewportSize;
    const Type   fType;

    using INHERITED = TransformAdapter3D;
};

} // namespace internal
} // namespace skottie

#endif // SkottieCamera_DEFINED
