/*
 * Copyright 2017 Google Inc.
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
        return fFragmentProcessors.count() - fColorFragmentProcessorCnt;
    }
    int numFragmentProcessors() const { return fFragmentProcessors.count(); }

    const GrFragmentProcessor* colorFragmentProcessor(int idx) const {
        SkASSERT(idx < fColorFragmentProcessorCnt);
        return fFragmentProcessors[idx];
    }
    const GrFragmentProcessor* coverageFragmentProcessor(int idx) const {
        return fFragmentProcessors[idx + fColorFragmentProcessorCnt];
    }

    const GrXPFactory* xpFactory() const { return fXPFactory; }

    void analyzeFragmentProcessors(GrPipelineAnalysis* analysis) const {
        const GrFragmentProcessor* const* fps = fFragmentProcessors.get();
        analysis->fColorPOI.analyzeProcessors(fps, fColorFragmentProcessorCnt);
        fps += fColorFragmentProcessorCnt;
        analysis->fCoveragePOI.analyzeProcessors(fps, this->numCoverageFragmentProcessors());
    }

private:
    const GrXPFactory* fXPFactory = nullptr;
    SkAutoSTArray<4, const GrFragmentProcessor*> fFragmentProcessors;
    int fColorFragmentProcessorCnt;
};

#endif
