/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStrokeInstancedShaderImpl_DEFINED
#define GrStrokeInstancedShaderImpl_DEFINED

#include "src/gpu/tessellate/GrStrokeShader.h"

class GrStrokeInstancedShaderImpl : public GrStrokeShaderImpl {
    void onEmitCode(EmitArgs&, GrGPArgs*) override;
};

#endif
