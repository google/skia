/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrProcessorSet_DEFINED
#define GrProcessorSet_DEFINED

#include "GrFragmentProcessor.h"
#include "GrPaint.h"
#include "GrPipeline.h"
#include "SkTemplates.h"

class GrXPFactory;

class GrProcessorSet : private SkNoncopyable {
public:
    GrProcessorSet(GrPaint&& paint);

    ~GrProcessorSet() {
        // We are deliberately not using sk_sp here because this will be updated to work with
        // "pending execution" refs.
        for (auto fp : fFragmentProcessors) {
            fp->unref();
        }
    }

    int numColorFragmentProcessors() const { return fColorFragmentProcessorCnt; }
    int numCoverageFragmentProcessors() const {
        return this->fFragmentProcessors.count() - fColorFragmentProcessorCnt;
    }
    int numFragmentProcessors() const { return fFragmentProcessors.count(); }

    const GrFragmentProcessor* colorFragmentProcessor(int idx) const {
        SkASSERT(idx < fColorFragmentProcessorCnt);
        return fFragmentProcessors[idx];
    }
    const GrFragmentProcessor* coverageFragmentProcessor(int idx) const {
        SkASSERT(idx >= fColorFragmentProcessorCnt);
        return fFragmentProcessors[idx + fColorFragmentProcessorCnt];
    }

    const GrXPFactory* xpFactory() const { return fXPFactory; }

    void analyzeFragmentProcessors(GrPipelineAnalysis* analysis) const {
        const GrFragmentProcessor* const* fps = fFragmentProcessors.get();
        analysis->fColorPOI.addProcessors(fps, fColorFragmentProcessorCnt);
        fps += fColorFragmentProcessorCnt;
        analysis->fCoveragePOI.addProcessors(fps, this->numCoverageFragmentProcessors());
    }

private:
    // This limit is far higher than would ever be needed and allows this class to be a bit more
    // compact.
    static constexpr int kMaxColorFragmentProcessors = 64;
    const GrXPFactory* fXPFactory = nullptr;
    SkAutoSTArray<4, const GrFragmentProcessor*> fFragmentProcessors;
    int8_t fColorFragmentProcessorCnt;
};

#endif
