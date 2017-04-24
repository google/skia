/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef gr_instanced_GLInstancedRendering_DEFINED
#define gr_instanced_GLInstancedRendering_DEFINED

#include "GrCaps.h"
#include "gl/GrGLBuffer.h"
#include "instanced/InstancedOp.h"
#include "instanced/InstancedRendering.h"

class GrGLCaps;
class GrGLGpu;

#define GR_GL_LOG_INSTANCED_OPS 0

namespace gr_instanced {

class GLOpAllocator final : public OpAllocator {
public:
    GLOpAllocator(const GrCaps* caps) : INHERITED(caps) {}

private:
    std::unique_ptr<InstancedOp> makeOp(GrPaint&& paint) override;

    typedef OpAllocator INHERITED;
};

class GLInstancedRendering final : public InstancedRendering {
public:
    GLInstancedRendering(GrGLGpu*);
    ~GLInstancedRendering() override;

private:
    /**
     * Called by GrGLCaps to determine the level of support this class can offer for instanced
     * rendering on the current platform.
     */
    static GrCaps::InstancedSupport CheckSupport(const GrGLCaps&);

    GrGLGpu* glGpu() const;

    void onBeginFlush(GrResourceProvider*) override;
    void onDraw(const GrPipeline&, const InstanceProcessor&, const InstancedOp*) override;
    void onEndFlush() override;
    void onResetGpuResources(ResetType) override;

    void flushInstanceAttribs(int baseInstance);

    struct GLDrawCmdInfo {
        IndexRange fGeometry;
        int fInstanceCount;
    };

    GrGLuint                              fVertexArrayID;
    sk_sp<GrBuffer>                       fInstanceBuffer;
    sk_sp<GrBuffer>                       fDrawIndirectBuffer;
    SkAutoSTMalloc<1024, GLDrawCmdInfo>   fGLDrawCmdsInfo;
    GrGpuResource::UniqueID               fInstanceAttribsBufferUniqueId;
    int                                   fInstanceAttribsBaseInstance;

    friend class ::GrGLCaps; // For CheckSupport.

    typedef InstancedRendering INHERITED;
};

}

#endif
