/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrColorProcessor_DEFINED
#define GrColorProcessor_DEFINED

#include "GrFragmentProcessor.h"

/**
 * This is a simple GrFragmentProcessor that outputs a constant color. It may do one of the
 * following with its input color: ignore it, or multiply it by the constant color, multiply its
 * alpha by the constant color and ignore the input color's r, g, and b.
 */
class GrConstColorProcessor : public GrFragmentProcessor {
public:
    enum InputMode {
        kIgnore_InputMode,
        kModulateRGBA_InputMode,
        kModulateA_InputMode,

        kLastInputMode = kModulateA_InputMode
    };
    static const int kInputModeCnt = kLastInputMode + 1;

    static std::unique_ptr<GrFragmentProcessor> Make(GrColor4f color, InputMode mode) {
        return std::unique_ptr<GrFragmentProcessor>(new GrConstColorProcessor(color, mode));
    }

    const char* name() const override { return "Color"; }

    SkString dumpInfo() const override {
        SkString str;
        str.appendf("Color: 0x%08x", fColor.toGrColor());
        return str;
    }

    std::unique_ptr<GrFragmentProcessor> clone() const override;

    GrColor4f color() const { return fColor; }

    InputMode inputMode() const { return fMode; }

private:
    static OptimizationFlags OptFlags(GrColor4f color, InputMode mode) {
        OptimizationFlags flags = kConstantOutputForConstantInput_OptimizationFlag;
        if (mode != kIgnore_InputMode) {
            flags |= kCompatibleWithCoverageAsAlpha_OptimizationFlag;
        }
        if (color.isOpaque()) {
            flags |= kPreservesOpaqueInput_OptimizationFlag;
        }
        return flags;
    }

    GrConstColorProcessor(GrColor4f color, InputMode mode)
            : INHERITED(kGrConstColorProcessor_ClassID
            , OptFlags(color, mode))
            , fColor(color)
            , fMode(mode) {
    }

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    GrColor4f constantOutputForConstantInput(GrColor4f input) const override;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    GrColor4f   fColor;
    InputMode   fMode;

    typedef GrFragmentProcessor INHERITED;
};

#endif
