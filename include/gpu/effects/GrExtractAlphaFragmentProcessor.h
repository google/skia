/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrExtractAlphaFragmentProcessor_DEFINED
#define GrExtractAlphaFragmentProcessor_DEFINED

#include "GrFragmentProcessor.h"

/** This processor extracts the incoming color's alpha, ignores r, g, and b, and feeds
    the replicated alpha to it's inner processor. */
class GrExtractAlphaFragmentProcessor : public GrFragmentProcessor {
public:
    static GrFragmentProcessor* Create(const GrFragmentProcessor* processor) {
        if (!processor) {
            return nullptr;
        }
        return new GrExtractAlphaFragmentProcessor(processor);
    }

    ~GrExtractAlphaFragmentProcessor() override {}

    const char* name() const override { return "Extract Alpha"; }

private:
    GrExtractAlphaFragmentProcessor(const GrFragmentProcessor* processor) {
        this->initClassID<GrExtractAlphaFragmentProcessor>();
        this->registerChildProcessor(processor);
    }

    GrGLFragmentProcessor* onCreateGLInstance() const override;

    void onGetGLProcessorKey(const GrGLSLCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    typedef GrFragmentProcessor INHERITED;
};

#endif
