/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_ganesh_AtlasTextOpPriv_DEFINED
#define skgpu_ganesh_AtlasTextOpPriv_DEFINED

#include "src/gpu/ganesh/ops/GrOp.h"

class SkPaint;
class SkFont;
class SkMatrix;

namespace skgpu::ganesh {
class SurfaceDrawContext;

class AtlasTextOpTools final {
public:
    static GrOp::Owner CreateOp(SurfaceDrawContext*,
                                const SkPaint&,
                                const SkFont&,
                                const SkMatrix&,
                                const char* text,
                                int x,
                                int y);

private:
    AtlasTextOpTools();
};

}  // namespace skgpu::ganesh

#endif
