/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCPRClipProcessor_DEFINED
#define GrCCPRClipProcessor_DEFINED

#include "GrFragmentProcessor.h"
#include "ccpr/GrCoverageCountingPathRenderer.h"

class GrCCPRClipProcessor : public GrFragmentProcessor {
public:
    using ClipPath = GrCoverageCountingPathRenderer::ClipPath;

    enum class MustCheckBounds : bool {
        kNo = false,
        kYes = true
    };

    GrCCPRClipProcessor(const ClipPath*, MustCheckBounds, SkPath::FillType overrideFillType);

    const char* name() const override { return "GrCCPRClipProcessor"; }
    std::unique_ptr<GrFragmentProcessor> clone() const override;
    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;
    bool onIsEqual(const GrFragmentProcessor&) const override;
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

private:
    const ClipPath* const fClipPath;
    const bool fMustCheckBounds;
    const SkPath::FillType fOverrideFillType;
    const TextureSampler fAtlasAccess;

    class Impl;

    typedef GrFragmentProcessor INHERITED;
};

#endif
