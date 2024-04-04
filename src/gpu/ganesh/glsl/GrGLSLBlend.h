/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLBlend_DEFINED
#define GrGLBlend_DEFINED

#include "src/gpu/ganesh/glsl/GrGLSLProgramDataManager.h"

#include <string>

class GrGLSLUniformHandler;
class GrProcessor;
enum class SkBlendMode;

namespace GrGLSLBlend {

/**
 * Returns an SkSL expression that blends the passed-in srcColor and dstColor values.
 * Matching calls to SetBlendModeUniformData and BlendKey must be made from your GrProcessor.
 */
std::string BlendExpression(const GrProcessor* processor,
                            GrGLSLUniformHandler* uniformHandler,
                            GrGLSLProgramDataManager::UniformHandle* uniform,
                            const char* srcColor,
                            const char* dstColor,
                            SkBlendMode mode);

/**
 * Returns a key, for use in onAddToKey from any GrProcessor. You must pass the same blend mode that
 * was passed to BlendExpression
 */
int BlendKey(SkBlendMode mode);

/**
 * Sets up uniforms, for use in onSetData from any GrProcessor. You must pass the same uniform and
 * blend mode that were passed to BlendExpression.
 */
void SetBlendModeUniformData(const GrGLSLProgramDataManager& pdman,
                             GrGLSLProgramDataManager::UniformHandle uniform,
                             SkBlendMode mode);

}  // namespace GrGLSLBlend

#endif
