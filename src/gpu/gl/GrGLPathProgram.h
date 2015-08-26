/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLPathProgram_DEFINED
#define GrGLPathProgram_DEFINED

#include "gl/GrGLProgram.h"
#include "gl/GrGLPathProgramDataManager.h"

/*
 * The default GrGL programs consist of at the very least a vertex and fragment shader.
 * 1.3+ Nvpr ignores the vertex shader, but both require
 * specialized methods for setting transform data.  NVPR also requires setting the
 * projection matrix through a special function call.
 */
class GrGLPathProgram : public GrGLProgram {
protected:
    typedef GrGLPathProgramDataManager::SeparableVaryingInfoArray SeparableVaryingInfoArray;
    GrGLPathProgram(GrGLGpu*,
                    const GrProgramDesc&,
                    const BuiltinUniformHandles&,
                    GrGLuint programID,
                    const UniformInfoArray&,
                    const SeparableVaryingInfoArray&,
                    GrGLInstalledGeoProc*,
                    GrGLInstalledXferProc* xferProcessor,
                    GrGLInstalledFragProcs* fragmentProcessors,
                    SkTArray<UniformHandle>* passSamplerUniforms);

private:
    void didSetData() override;
    virtual void setTransformData(const GrPrimitiveProcessor&,
                                  const GrPendingFragmentStage&,
                                  int index,
                                  GrGLInstalledFragProc*) override;
    void onSetRenderTargetState(const GrPrimitiveProcessor&, const GrPipeline&) override;

    friend class GrGLPathProgramBuilder;

    GrGLPathProgramDataManager fPathProgramDataManager;

    typedef GrGLProgram INHERITED;
};

#endif
