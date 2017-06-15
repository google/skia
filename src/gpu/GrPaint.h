
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrPaint_DEFINED
#define GrPaint_DEFINED

#include "GrColor.h"
#include "GrColorSpaceXform.h"
#include "GrFragmentProcessor.h"
#include "SkBlendMode.h"
#include "SkRefCnt.h"
#include "SkRegion.h"
#include "SkTLazy.h"

class GrTextureProxy;
class GrXPFactory;

/**
 * The paint describes how color and coverage are computed at each pixel by GrContext draw
 * functions and the how color is blended with the destination pixel.
 *
 * The paint allows installation of custom color and coverage stages. New types of stages are
 * created by subclassing GrProcessor.
 *
 * The primitive color computation starts with the color specified by setColor(). This color is the
 * input to the first color stage. Each color stage feeds its output to the next color stage.
 *
 * Fractional pixel coverage follows a similar flow. The GrGeometryProcessor (specified elsewhere)
 * provides the initial coverage which is passed to the first coverage fragment processor, which
 * feeds its output to next coverage fragment processor.
 *
 * setXPFactory is used to control blending between the output color and dest. It also implements
 * the application of fractional coverage from the coverage pipeline.
 */
class GrPaint {
public:
    GrPaint() = default;
    explicit GrPaint(const GrPaint&) = default;
    ~GrPaint() = default;

    /**
     * The initial color of the drawn primitive. Defaults to solid white.
     */
    void setColor4f(const GrColor4f& color) { fColor = color; }
    const GrColor4f& getColor4f() const { return fColor; }

    /**
     * Legacy getter, until all code handles 4f directly.
     */
    GrColor getColor() const { return fColor.toGrColor(); }

    /**
     * Should shader output conversion from linear to sRGB be disabled.
     * Only relevant if the destination is sRGB. Defaults to false.
     */
    void setDisableOutputConversionToSRGB(bool srgb) { fDisableOutputConversionToSRGB = srgb; }
    bool getDisableOutputConversionToSRGB() const { return fDisableOutputConversionToSRGB; }

    /**
     * Should sRGB inputs be allowed to perform sRGB to linear conversion. With this flag
     * set to false, sRGB textures will be treated as linear (including filtering).
     */
    void setAllowSRGBInputs(bool allowSRGBInputs) { fAllowSRGBInputs = allowSRGBInputs; }
    bool getAllowSRGBInputs() const { return fAllowSRGBInputs; }

    /**
     * Should rendering be gamma-correct, end-to-end. Causes sRGB render targets to behave
     * as such (with linear blending), and sRGB inputs to be filtered and decoded correctly.
     */
    void setGammaCorrect(bool gammaCorrect) {
        this->setDisableOutputConversionToSRGB(!gammaCorrect);
        this->setAllowSRGBInputs(gammaCorrect);
    }

    void setXPFactory(const GrXPFactory* xpFactory) {
        fXPFactory = xpFactory;
        fTrivial &= !SkToBool(xpFactory);
    }

    void setPorterDuffXPFactory(SkBlendMode mode);

    void setCoverageSetOpXPFactory(SkRegion::Op, bool invertCoverage = false);

    /**
     * Appends an additional color processor to the color computation.
     */
    void addColorFragmentProcessor(sk_sp<GrFragmentProcessor> fp) {
        SkASSERT(fp);
        fColorFragmentProcessors.push_back(std::move(fp));
        fTrivial = false;
    }

    /**
     * Appends an additional coverage processor to the coverage computation.
     */
    void addCoverageFragmentProcessor(sk_sp<GrFragmentProcessor> fp) {
        SkASSERT(fp);
        fCoverageFragmentProcessors.push_back(std::move(fp));
        fTrivial = false;
    }

    /**
     * Helpers for adding color or coverage effects that sample a texture. The matrix is applied
     * to the src space position to compute texture coordinates.
     */
    void addColorTextureProcessor(sk_sp<GrTextureProxy>,
                                  sk_sp<GrColorSpaceXform>, const SkMatrix&);
    void addColorTextureProcessor(sk_sp<GrTextureProxy>,
                                  sk_sp<GrColorSpaceXform>, const SkMatrix&,
                                  const GrSamplerParams&);

    void addCoverageTextureProcessor(sk_sp<GrTextureProxy>, const SkMatrix&);
    void addCoverageTextureProcessor(sk_sp<GrTextureProxy>,
                                     const SkMatrix&, const GrSamplerParams&);

    int numColorFragmentProcessors() const { return fColorFragmentProcessors.count(); }
    int numCoverageFragmentProcessors() const { return fCoverageFragmentProcessors.count(); }
    int numTotalFragmentProcessors() const { return this->numColorFragmentProcessors() +
                                              this->numCoverageFragmentProcessors(); }

    const GrXPFactory* getXPFactory() const { return fXPFactory; }

    GrFragmentProcessor* getColorFragmentProcessor(int i) const {
        return fColorFragmentProcessors[i].get();
    }
    GrFragmentProcessor* getCoverageFragmentProcessor(int i) const {
        return fCoverageFragmentProcessors[i].get();
    }

    /**
     * Returns true if the paint's output color will be constant after blending. If the result is
     * true, constantColor will be updated to contain the constant color. Note that we can conflate
     * coverage and color, so the actual values written to pixels with partial coverage may still
     * not seem constant, even if this function returns true.
     */
    bool isConstantBlendedColor(GrColor* constantColor) const;

    /**
     * A trivial paint is one that uses src-over and has no fragment processors.
     * It may have variable sRGB settings.
     **/
    bool isTrivial() const { return fTrivial; }

private:
    template <bool> class MoveOrImpl;

public:
    /**
     * A temporary instance of this class can be used to select between moving an existing paint or
     * a temporary copy of an existing paint into a call site. MoveOrClone(paint, false) is a rvalue
     * reference to paint while MoveOrClone(paint, true) is a rvalue reference to a copy of paint.
     */
    using MoveOrClone = MoveOrImpl<true>;

    /**
     * A temporary instance of this class can be used to select between moving an existing or a
     * newly default constructed paint into a call site. MoveOrNew(paint, false) is a rvalue
     * reference to paint while MoveOrNew(paint, true) is a rvalue reference to a default paint.
     */
    using MoveOrNew = MoveOrImpl<false>;

private:
    GrPaint& operator=(const GrPaint&) = delete;

    friend class GrProcessorSet;

    const GrXPFactory* fXPFactory = nullptr;
    SkSTArray<4, sk_sp<GrFragmentProcessor>>  fColorFragmentProcessors;
    SkSTArray<2, sk_sp<GrFragmentProcessor>>  fCoverageFragmentProcessors;
    bool fDisableOutputConversionToSRGB = false;
    bool fAllowSRGBInputs = false;
    bool fTrivial = true;
    GrColor4f fColor = GrColor4f::OpaqueWhite();
};

/** This is the implementation of MoveOrCopy and MoveOrNew. */
template <bool COPY_IF_NEW>
class GrPaint::MoveOrImpl {
public:
    MoveOrImpl(GrPaint& paint, bool newPaint) {
        if (newPaint) {
            if (COPY_IF_NEW) {
                fStorage.init(paint);
            } else {
                fStorage.init();
            };
            fPaint = fStorage.get();
        } else {
            fPaint = &paint;
        }
    }

    operator GrPaint&&() && { return std::move(*fPaint); }
    GrPaint& paint() { return *fPaint; }

private:
    SkTLazy<GrPaint> fStorage;
    GrPaint* fPaint;
};

#endif
