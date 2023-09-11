/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef AAHairLinePathRenderer_DEFINED
#define AAHairLinePathRenderer_DEFINED

#include "src/gpu/ganesh/PathRenderer.h"

namespace skgpu::ganesh {

class AAHairLinePathRenderer final : public PathRenderer {
public:
    AAHairLinePathRenderer() = default;

    const char* name() const override { return "AAHairline"; }

private:
    CanDrawPath onCanDrawPath(const CanDrawPathArgs&) const override;

    bool onDrawPath(const DrawPathArgs&) override;
};

}  // namespace skgpu::ganesh

#endif // AAHairLinePathRenderer_DEFINED
