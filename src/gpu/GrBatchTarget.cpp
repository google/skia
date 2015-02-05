/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBatchTarget.h"

#include "GrBufferAllocPool.h"
#include "GrPipeline.h"

/*
void GrBatchTarget::flush() {
    FlushBuffer::Iter iter(fFlushBuffer);
    fVertexPool->unmap();
    fIndexPool->unmap();

    while (iter.next()) {
        GrProgramDesc desc;
        BufferedFlush* bf = iter.get();
        const GrPipeline* pipeline = bf->fPipeline;
        const GrPrimitiveProcessor* primProc = bf->fPrimitiveProcessor.get();
        fGpu->buildProgramDesc(&desc, *primProc, *pipeline, bf->fBatchTracker);

        GrGpu::DrawArgs args(primProc, pipeline, &desc, &bf->fBatchTracker);
        for (int i = 0; i < bf->fDraws.count(); i++) {
            fGpu->draw(args, bf->fDraws[i]);
        }
    }
    fFlushBuffer.reset();
}*/
/*
void GrBatchTarget::flushNext(int n) {
    for (; n > 0; n--) {
        SkDEBUGCODE(bool verify =) fIter.next();
        SkASSERT(verify);
        GrProgramDesc desc;
        BufferedFlush* bf = fIter.get();
        const GrPipeline* pipeline = bf->fPipeline;
        const GrPrimitiveProcessor* primProc = bf->fPrimitiveProcessor.get();
        fGpu->buildProgramDesc(&desc, *primProc, *pipeline, bf->fBatchTracker);

        GrGpu::DrawArgs args(primProc, pipeline, &desc, &bf->fBatchTracker);
        for (int i = 0; i < bf->fDraws.count(); i++) {
            fGpu->draw(args, bf->fDraws[i]);
        }
    }
}*/
