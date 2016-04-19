/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSLSampler_DEFINED
#define GrGLSLSampler_DEFINED

#include "GrTypes.h"
#include "SkTArray.h"
#include "glsl/GrGLSLProgramDataManager.h"

class GrGLSLSampler {
public:
    typedef GrGLSLProgramDataManager::UniformHandle UniformHandle;
    typedef SkTArray<GrGLSLSampler> SamplerArray;

    GrGLSLSampler(UniformHandle uniform, GrPixelConfig config)
        : fSamplerUniform(uniform)
        , fConfig(config) {
        SkASSERT(kUnknown_GrPixelConfig != fConfig);
    }

    GrPixelConfig config() const { return fConfig; }

private:
    UniformHandle fSamplerUniform;
    GrPixelConfig fConfig;

    friend class GrGLSLShaderBuilder;
};

#endif
