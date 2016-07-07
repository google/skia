/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef gr_instanced_GLInstancedRendering_DEFINED
#define gr_instanced_GLInstancedRendering_DEFINED

#include "gl/GrGLBuffer.h"
#include "instanced/InstancedRendering.h"

class GrGLGpu;

#define GR_GL_LOG_INSTANCED_BATCHES 0

namespace gr_instanced {

class GLInstancedRendering final : public InstancedRendering {
public:
    static GLInstancedRendering* CreateIfSupported(GrGLGpu*);
    ~GLInstancedRendering() override;

private:
    GLInstancedRendering(GrGLGpu*, AntialiasMode lastSupportedAAMode);

    GrGLGpu* glGpu() const;

    Batch* createBatch() override;

    void onBeginFlush(GrResourceProvider*) override;
    void onDraw(const GrPipeline&, const InstanceProcessor&, const Batch*) override;
    void onEndFlush() override;
    void onResetGpuResources(ResetType) override;

    void flushInstanceAttribs(int baseInstance);

    struct GLDrawCmdInfo {
        int fInstanceCount;
#if GR_GL_LOG_INSTANCED_BATCHES
        IndexRange fGeometry;
#endif
    };

    GrGLuint                              fVertexArrayID;
    SkAutoTUnref<GrGLBuffer>              fInstanceBuffer;
    SkAutoTUnref<GrGLBuffer>              fDrawIndirectBuffer;
    SkAutoSTMalloc<1024, GLDrawCmdInfo>   fGLDrawCmdsInfo;
    uint32_t                              fInstanceAttribsBufferUniqueId;
    int                                   fInstanceAttribsBaseInstance;

    class GLBatch;

    typedef InstancedRendering INHERITED;
};

}

#endif
