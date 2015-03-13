/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBatchTarget.h"

#include "GrBatchAtlas.h"
#include "GrPipeline.h"

GrBatchTarget::GrBatchTarget(GrGpu* gpu,
                             GrVertexBufferAllocPool* vpool,
                             GrIndexBufferAllocPool* ipool)
    : fGpu(gpu)
    , fVertexPool(vpool)
    , fIndexPool(ipool)
    , fFlushBuffer(kFlushBufferInitialSizeInBytes)
    , fIter(fFlushBuffer)
    , fNumberOfDraws(0)
    , fCurrentToken(0)
    , fLastFlushedToken(0)
    , fInlineUpdatesIndex(0) {
}

void GrBatchTarget::flushNext(int n)  {
    for (; n > 0; n--) {
        fLastFlushedToken++;
        SkDEBUGCODE(bool verify =) fIter.next();
        SkASSERT(verify);

        BufferedFlush* bf = fIter.get();

        // Flush all texture uploads
        int uploadCount = fInlineUploads.count();
        while (fInlineUpdatesIndex < uploadCount &&
               fInlineUploads[fInlineUpdatesIndex]->lastUploadToken() <= fLastFlushedToken) {
            fInlineUploads[fInlineUpdatesIndex++]->upload(TextureUploader(fGpu));
        }

        GrProgramDesc desc;
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
