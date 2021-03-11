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
    enum class MustCheckBounds : bool {
        kNo = false,
        kYes = true
    };

    GrCCClipProcessor(std::unique_ptr<GrFragmentProcessor>, const GrCaps&, const GrCCClipPath*,
                      MustCheckBounds);

    const char* name() const override { return "GrCCClipProcessor"; }
    std::unique_ptr<GrFragmentProcessor> clone() const override;
    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;
    bool onIsEqual(const GrFragmentProcessor&) const override;
    std::unique_ptr<GrGLSLFragmentProcessor> onMakeProgramImpl() const override;

private:
    explicit GrCCClipProcessor(const GrCCClipProcessor&);

    const GrCCClipPath* const fClipPath;
    const bool fMustCheckBounds;

    class Impl;

    using INHERITED = GrFragmentProcessor;
};

#endif
