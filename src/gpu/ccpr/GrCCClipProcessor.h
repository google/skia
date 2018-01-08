/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCClipProcessor_DEFINED
#define GrCCClipProcessor_DEFINED

#include "GrFragmentProcessor.h"
#include "ccpr/GrCoverageCountingPathRenderer.h"

class GrCCClipProcessor : public GrFragmentProcessor {
public:
    using ClipPath = GrCoverageCountingPathRenderer::ClipPath;

    enum class MustCheckBounds : bool {
        kNo = false,
        kYes = true
    };

    GrCCClipProcessor(const ClipPath*, MustCheckBounds, SkPath::FillType overrideFillType);

    const char* name() const override { return "GrCCClipProcessor"; }
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
