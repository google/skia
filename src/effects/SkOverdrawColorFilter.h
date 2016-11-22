/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorFilter.h"

class SkOverdrawColorFilter : public SkColorFilter {
public:
    static constexpr int kNumColors = 6;

    static sk_sp<SkOverdrawColorFilter> Make(const SkPMColor colors[kNumColors]) {
        return sk_sp<SkOverdrawColorFilter>(new SkOverdrawColorFilter(colors));
    }

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(GrContext*, SkColorSpace*) const override;
#endif

    void filterSpan(const SkPMColor src[], int count, SkPMColor dst[]) const override;
    void toString(SkString* str) const override;
    Factory getFactory() const override { return nullptr; }

private:
    SkOverdrawColorFilter(const SkPMColor colors[kNumColors]) {
        memcpy(fColors, colors, kNumColors * sizeof(SkPMColor));
    }

    SkPMColor fColors[kNumColors];

    typedef SkColorFilter INHERITED;
};

#if SK_SUPPORT_GPU

#include "GrFragmentProcessor.h"
#include "glsl/GrGLSLFragmentProcessor.h"

class OverdrawFragmentProcessor : public GrFragmentProcessor {
public:
    static sk_sp<GrFragmentProcessor> Make(const SkPMColor* colors);

    const char* name() const override { return "Overdraw"; }

private:
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;
    void onGetGLSLProcessorKey(const GrGLSLCaps&, GrProcessorKeyBuilder*) const override {}
    bool onIsEqual(const GrFragmentProcessor&) const override;
    void onComputeInvariantOutput(GrInvariantOutput* inout) const override;

    OverdrawFragmentProcessor(const GrColor4f* colors);

    GrColor4f fColors[SkOverdrawColorFilter::kNumColors];

    typedef GrFragmentProcessor INHERITED;
};

class GLOverdrawFragmentProcessor : public GrGLSLFragmentProcessor {
public:
    GLOverdrawFragmentProcessor(const GrColor4f* colors);

    void emitCode(EmitArgs&) override;

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrProcessor&) override {}

private:
    GrColor4f fColors[SkOverdrawColorFilter::kNumColors];

    typedef GrGLSLFragmentProcessor INHERITED;
};

#endif // SK_SUPPORT_GPU
