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

    bool usesDistanceVectorField() const { return SkToBool(fFlags & kUseDistanceVectorField_Flag); }
    bool disableOutputConversionToSRGB() const {
        return SkToBool(fFlags & kDisableOutputConversionToSRGB_Flag);
    }
    bool allowSRGBInputs() const { return SkToBool(fFlags & kAllowSRGBInputs_Flag); }

private:
    const GrXPFactory* fXPFactory = nullptr;
    SkAutoSTArray<4, const GrFragmentProcessor*> fFragmentProcessors;
    int fColorFragmentProcessorCnt;
    enum Flags : uint32_t {
        kUseDistanceVectorField_Flag = 0x1,
        kDisableOutputConversionToSRGB_Flag = 0x2,
        kAllowSRGBInputs_Flag = 0x4
    };
    uint32_t fFlags;
};

#endif
