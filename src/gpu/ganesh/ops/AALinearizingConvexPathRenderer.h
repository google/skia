/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef AALinearizingConvexPathRenderer_DEFINED
#define AALinearizingConvexPathRenderer_DEFINED

#include "src/gpu/ganesh/PathRenderer.h"

namespace skgpu::ganesh {

class AALinearizingConvexPathRenderer final : public PathRenderer {
public:
    AALinearizingConvexPathRenderer() = default;

    const char* name() const override { return "AALinear"; }

private:
    CanDrawPath onCanDrawPath(const CanDrawPathArgs&) const override;

    bool onDrawPath(const DrawPathArgs&) override;
};

}  // namespace skgpu::ganesh

#endif // AALinearizingConvexPathRenderer_DEFINED
