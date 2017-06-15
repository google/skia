/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef gr_instanced_InstancedRendering_DEFINED
#define gr_instanced_InstancedRendering_DEFINED

#include "instanced/InstancedOp.h"
#include "instanced/InstancedRenderingTypes.h"

#include "SkTInternalLList.h"

class GrGpu;
class GrResourceProvider;

namespace gr_instanced {

class InstancedOp;
class InstanceProcessor;


/**
 * Instanced Rendering occurs through the interaction of four class:
 *   InstancedOp
 *   OpAllocator
 *   InstancedRendering
 *   InstanceProcessor
 *
 * The InstancedOp is a GrDrawOp but is more of a proxy than normal operations. It accumulates a
 * LL of Draw objects that are allocated all together by the OpAllocator.
 *
 * There is only one OpAllocator which encapsulates the creation of InstancedOps and the pool
 * of memory used for their Draw objects.
 *
 * The InstancedRendering class tracks a list of InstancedOps that will all be drawn during 
 * the same flush. There is currently one per opList. The nature of instanced
 * rendering allows these ops to combine well and render efficiently.
 * During a flush, it assembles the accumulated draw data into a single vertex and texel
 * buffer per opList, and its subclasses draw the ops using backend-specific instanced
 * rendering APIs.
 *
 * InstanceProcessors implement the shaders required for instance rendering.
 */
class InstancedRendering : public SkNoncopyable {
public:
    virtual ~InstancedRendering();

    GrGpu* gpu() const { return fGpu.get(); }

    /**
     * Compiles all recorded draws into GPU buffers and allows the client to begin flushing the
     * ops created by this class.
     */
    void beginFlush(GrResourceProvider*);

    void draw(const GrPipeline& pipeline, OpInfo info, const InstancedOp* baseOp);

    /**
     * Called once the ops created previously by this class have all been released. Allows the
     * client to begin recording draws again.
     */
    void endFlush();

    enum class ResetType : bool {
        kDestroy,
        kAbandon
    };

    /**
     * Resets all GPU resources, including those that are held long term. They will be lazily
     * reinitialized if the class begins to be used again.
     */
    void resetGpuResources(ResetType);

    void addOp(InstancedOp* op) { fTrackedOps.addToTail(op); }
    void removeOp(InstancedOp* op) { fTrackedOps.remove(op); }
    int addOpParams(InstancedOp* op);

#ifdef SK_DEBUG
    bool isFlushing() const { return InstancedRendering::State::kFlushing == fState; }
#endif

protected:
    typedef SkTInternalLList<InstancedOp> OpList;

    InstancedRendering(GrGpu* gpu);

    const OpList& trackedOps() const { return fTrackedOps; }
    const GrBuffer* vertexBuffer() const { SkASSERT(fVertexBuffer); return fVertexBuffer.get(); }
    const GrBuffer* indexBuffer() const { SkASSERT(fIndexBuffer); return fIndexBuffer.get(); }

    virtual void onBeginFlush(GrResourceProvider*) = 0;
    virtual void onDraw(const GrPipeline&, const InstanceProcessor&, const InstancedOp*) = 0;
    virtual void onEndFlush() = 0;
    virtual void onResetGpuResources(ResetType) = 0;

private:
#ifdef SK_DEBUG
    enum class State : bool {
        kRecordingDraws,
        kFlushing
    };
#endif

    const sk_sp<GrGpu> fGpu;
    SkDEBUGCODE(State fState;)
    SkSTArray<1024, ParamsTexel, true> fParams;
    OpList fTrackedOps;
    sk_sp<const GrBuffer> fVertexBuffer;
    sk_sp<const GrBuffer> fIndexBuffer;
    sk_sp<GrBuffer> fParamsBuffer;
};

}

#endif
