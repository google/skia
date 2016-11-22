/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "SkColorFilter.h"

class SkOverdrawColorFilter : public SkColorFilter {
public:

    sk_sp<SkOverdrawColorFilter> Make(const SkPMColor colors[6]) {
        return sk_make_sp<SkOverdrawColorFilter>(colors);
    }

    SkOverdrawColorFilter(const SkPMColor colors[6]) {
        memcpy(fColors, colors, 6 * sizeof(SkPMColor));
    }

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(GrContext*, SkColorSpace*) const override;
#endif

    void filterSpan(const SkPMColor src[], int count, SkPMColor dst[]) const override;

private:
    SkPMColor fColors[6];

    typedef SkColorFilter INHERITED;
};

#if SK_SUPPORT_GPU

#include "GrContext.h"
#include "GrFragmentProcessor.h"
#include "GrInvariantOutput.h"
#include "GrTextureStripAtlas.h"
#include "SkGr.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"

class OverdrawFragmentProcessor : public GrFragmentProcessor {
public:
    static sk_sp<GrFragmentProcessor> Make(GrContext* context, SkPMColor colors[6]);

    const char* name() const override { return "Overdraw"; }

private:
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;
    void onGetGLSLProcessorKey(const GrGLSLCaps&, GrProcessorKeyBuilder*) const override;
    bool onIsEqual(const GrFragmentProcessor&) const override;
    void onComputeInvariantOutput(GrInvariantOutput* inout) const override;

    OverdrawFragmentProcessor(/* params */);

    typedef GrFragmentProcessor INHERITED;
};

class GLOverdrawFragmentProcessor : public GrGLSLFragmentProcessor {
public:
    void emitCode(EmitArgs&) override;

    static void GenKey(const GrProcessor&, const GrGLSLCaps&, GrProcessorKeyBuilder*) {}

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrProcessor&) override;

private:
    UniformHandle fRGBAYValuesUni;
    typedef GrGLSLFragmentProcessor INHERITED;
};

#endif // SK_SUPPORT_GPU
