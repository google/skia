/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCClipProcessor_DEFINED
#define GrCCClipProcessor_DEFINED

#include "src/gpu/GrFragmentProcessor.h"

class GrCCClipPath;

class GrCCClipProcessor : public GrFragmentProcessor {
public:
    enum class IsCoverageCount : bool {
        kNo = false,
        kYes = true
    };

    enum class MustCheckBounds : bool {
        kNo = false,
        kYes = true
    };

    GrCCClipProcessor(std::unique_ptr<GrFragmentProcessor>, const GrCaps&, const GrCCClipPath*,
                      IsCoverageCount, MustCheckBounds);

    const char* name() const override { return "GrCCClipProcessor"; }
    std::unique_ptr<GrFragmentProcessor> clone() const override;
    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;
    bool onIsEqual(const GrFragmentProcessor&) const override;
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

private:
    explicit GrCCClipProcessor(const GrCCClipProcessor&);

    const GrCCClipPath* const fClipPath;
    const bool fIsCoverageCount;
    const bool fMustCheckBounds;

    class Impl;

    using INHERITED = GrFragmentProcessor;
};

#endif
