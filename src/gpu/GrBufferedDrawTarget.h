/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBufferedDrawTarget_DEFINED
#define GrBufferedDrawTarget_DEFINED

#include "GrDrawTarget.h"
#include "GrCommandBuilder.h"
#include "SkChunkAlloc.h"

/**
 * GrBufferedDrawTarget is an implementation of GrDrawTarget that queues up draws for eventual
 * playback into a GrGpu. In theory one draw buffer could playback into another. Similarly, it is
 * the caller's responsibility to ensure that all referenced textures, buffers, and render-targets
 * are associated in the GrGpu object that the buffer is played back into.
 */
class GrBufferedDrawTarget : public GrClipTarget {
public:

    /**
     * Creates a GrBufferedDrawTarget
     *
     * @param context    the context object that owns this draw buffer.
     */
    GrBufferedDrawTarget(GrContext* context);

    ~GrBufferedDrawTarget() override;

protected:
    void appendIndicesAndTransforms(const void* indexValues, PathIndexType indexType, 
                                    const float* transformValues, PathTransformType transformType,
                                    int count, char** indicesLocation, float** xformsLocation) {
        int indexBytes = GrPathRange::PathIndexSizeInBytes(indexType);
        *indicesLocation = (char*) fPathIndexBuffer.alloc(count * indexBytes,
                                                          SkChunkAlloc::kThrow_AllocFailType);
        SkASSERT(SkIsAlign4((uintptr_t)*indicesLocation));
        memcpy(*indicesLocation, reinterpret_cast<const char*>(indexValues), count * indexBytes);

        const int xformBytes = GrPathRendering::PathTransformSize(transformType) * sizeof(float);
        *xformsLocation = NULL;

        if (0 != xformBytes) {
            *xformsLocation = (float*) fPathTransformBuffer.alloc(count * xformBytes,
                                                               SkChunkAlloc::kThrow_AllocFailType);
            SkASSERT(SkIsAlign4((uintptr_t)*xformsLocation));
            memcpy(*xformsLocation, transformValues, count * xformBytes);
        }
    }

    void onDrawBatch(GrBatch*) override;

private:
    friend class GrInOrderCommandBuilder;
    friend class GrTargetCommands;

    typedef GrTargetCommands::StateForPathDraw StateForPathDraw;

    StateForPathDraw* allocState(const GrPrimitiveProcessor* primProc = NULL) {
        void* allocation = fPipelineBuffer.alloc(sizeof(StateForPathDraw),
                                                 SkChunkAlloc::kThrow_AllocFailType);
        return SkNEW_PLACEMENT_ARGS(allocation, StateForPathDraw, (primProc));
    }

    void unallocState(StateForPathDraw* state) {
        state->unref();
        fPipelineBuffer.unalloc(state);
    }

    void onReset() override;
    void onFlush() override;

    void onDrawPath(const GrPathProcessor*,
                    const GrPath*,
                    const GrStencilSettings&,
                    const PipelineInfo&) override;
    void onDrawPaths(const GrPathProcessor*,
                     const GrPathRange*,
                     const void* indices,
                     PathIndexType,
                     const float transformValues[],
                     PathTransformType,
                     int count,
                     const GrStencilSettings&,
                     const PipelineInfo&) override;

    bool isIssued(uint32_t drawID) override { return drawID != fDrawID; }

    StateForPathDraw* SK_WARN_UNUSED_RESULT createStateForPathDraw(
        const GrPrimitiveProcessor*,
        const PipelineInfo&,
        GrPipelineOptimizations* opts);

    // TODO: Use a single allocator for commands and records
    enum {
        kPathIdxBufferMinReserve     = 2 * 64,  // 64 uint16_t's
        kPathXformBufferMinReserve   = 2 * 64,  // 64 two-float transforms
        kPipelineBufferMinReserve    = 32 * sizeof(StateForPathDraw),
    };

    // every 100 flushes we should reset our fPipelineBuffer to prevent us from holding at a
    // highwater mark
    static const int kPipelineBufferHighWaterMark = 100;

    SkAutoTDelete<GrCommandBuilder>     fCommands;
    SkChunkAlloc                        fPathIndexBuffer;
    SkChunkAlloc                        fPathTransformBuffer;
    SkChunkAlloc                        fPipelineBuffer;
    uint32_t                            fDrawID;
    SkAutoTUnref<StateForPathDraw>      fPrevState;

    typedef GrClipTarget INHERITED;
};

#endif
