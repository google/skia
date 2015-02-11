/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBatchBuffer_DEFINED
#define GrBatchBuffer_DEFINED

#include "GrPendingProgramElement.h"
#include "GrPipeline.h"
#include "GrGpu.h"
#include "GrTRecorder.h"

/*
 * GrBatch instances use this object to allocate space for their geometry and to issue the draws
 * that render their batch.
 */

class GrIndexBufferAllocPool;
class GrVertexBufferAllocPool;

class GrBatchTarget : public SkNoncopyable {
public:
    GrBatchTarget(GrGpu* gpu,
                  GrVertexBufferAllocPool* vpool,
                  GrIndexBufferAllocPool* ipool)
        : fGpu(gpu)
        , fVertexPool(vpool)
        , fIndexPool(ipool)
        , fFlushBuffer(kFlushBufferInitialSizeInBytes)
        , fIter(fFlushBuffer)
        , fNumberOfDraws(0) {}

    typedef GrDrawTarget::DrawInfo DrawInfo;
    void initDraw(const GrPrimitiveProcessor* primProc, const GrPipeline* pipeline) {
        GrNEW_APPEND_TO_RECORDER(fFlushBuffer, BufferedFlush, (primProc, pipeline));
        fNumberOfDraws++;
    }

    void draw(const GrDrawTarget::DrawInfo& draw) {
        fFlushBuffer.back().fDraws.push_back(draw);
    }

    // TODO this is temporary until batch is everywhere
    //void flush();
    void resetNumberOfDraws() { fNumberOfDraws = 0; }
    int numberOfDraws() const { return fNumberOfDraws; }
    void preFlush() { fIter = FlushBuffer::Iter(fFlushBuffer); }
    void flushNext(int n) {
        for (; n > 0; n--) {
            SkDEBUGCODE(bool verify =) fIter.next();
            SkASSERT(verify);
            GrProgramDesc desc;
            BufferedFlush* bf = fIter.get();
            const GrPipeline* pipeline = bf->fPipeline;
            const GrPrimitiveProcessor* primProc = bf->fPrimitiveProcessor.get();
            fGpu->buildProgramDesc(&desc, *primProc, *pipeline, bf->fBatchTracker);

            GrGpu::DrawArgs args(primProc, pipeline, &desc, &bf->fBatchTracker);

            int drawCount = bf->fDraws.count();
            const SkSTArray<1, DrawInfo, true>& draws = bf->fDraws;
            for (int i = 0; i < drawCount; i++) {
                fGpu->draw(args, draws[i]);
            }
        }
    }
    void postFlush() { SkASSERT(!fIter.next()); fFlushBuffer.reset(); }

    // TODO This goes away when everything uses batch
    GrBatchTracker* currentBatchTracker() {
        SkASSERT(!fFlushBuffer.empty());
        return &fFlushBuffer.back().fBatchTracker;
    }

    const GrDrawTargetCaps& caps() const { return *fGpu->caps(); }

    GrVertexBufferAllocPool* vertexPool() { return fVertexPool; }
    GrIndexBufferAllocPool* indexPool() { return fIndexPool; }

    const GrIndexBuffer* quadIndexBuffer() const { return fGpu->getQuadIndexBuffer(); }

private:
    GrGpu* fGpu;
    GrVertexBufferAllocPool* fVertexPool;
    GrIndexBufferAllocPool* fIndexPool;

    typedef void* TBufferAlign; // This wouldn't be enough align if a command used long double.

    struct BufferedFlush {
        BufferedFlush(const GrPrimitiveProcessor* primProc, const GrPipeline* pipeline)
            : fPrimitiveProcessor(primProc)
            , fPipeline(pipeline) {}
        typedef GrPendingProgramElement<const GrPrimitiveProcessor> ProgramPrimitiveProcessor;
        ProgramPrimitiveProcessor fPrimitiveProcessor;
        const GrPipeline* fPipeline;
        GrBatchTracker fBatchTracker;
        SkSTArray<1, DrawInfo, true> fDraws;
    };

    enum {
        kFlushBufferInitialSizeInBytes = 8 * sizeof(BufferedFlush),
    };

    typedef GrTRecorder<BufferedFlush, TBufferAlign> FlushBuffer;

    FlushBuffer fFlushBuffer;
    // TODO this is temporary
    FlushBuffer::Iter fIter;
    int fNumberOfDraws;
};

#endif
