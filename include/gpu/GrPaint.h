
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
#include "GrXferProcessor.h"
#include "effects/GrPorterDuffXferProcessor.h"
#include "GrFragmentProcessor.h"

#include "SkBlendMode.h"
#include "SkRefCnt.h"
#include "SkRegion.h"

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
    GrPaint();

    GrPaint(const GrPaint& paint) { *this = paint; }

    ~GrPaint() { }

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
     * Does one of the fragment processors need a field of distance vectors to the nearest edge?
     */
    bool usesDistanceVectorField() const { return fUsesDistanceVectorField; }

    /**
     * Should rendering be gamma-correct, end-to-end. Causes sRGB render targets to behave
     * as such (with linear blending), and sRGB inputs to be filtered and decoded correctly.
     */
    void setGammaCorrect(bool gammaCorrect) {
        setDisableOutputConversionToSRGB(!gammaCorrect);
        setAllowSRGBInputs(gammaCorrect);
    }

    void setXPFactory(sk_sp<GrXPFactory> xpFactory) {
        fXPFactory = std::move(xpFactory);
    }

    void setPorterDuffXPFactory(SkBlendMode mode) {
        fXPFactory = GrPorterDuffXPFactory::Make(mode);
    }

    void setCoverageSetOpXPFactory(SkRegion::Op, bool invertCoverage = false);

    /**
     * Appends an additional color processor to the color computation.
     */
    void addColorFragmentProcessor(sk_sp<GrFragmentProcessor> fp) {
        SkASSERT(fp);
        fUsesDistanceVectorField |= fp->usesDistanceVectorField();
        fColorFragmentProcessors.push_back(std::move(fp));
    }

    /**
     * Appends an additional coverage processor to the coverage computation.
     */
    void addCoverageFragmentProcessor(sk_sp<GrFragmentProcessor> fp) {
        SkASSERT(fp);
        fUsesDistanceVectorField |= fp->usesDistanceVectorField();
        fCoverageFragmentProcessors.push_back(std::move(fp));
    }

    /**
     * Helpers for adding color or coverage effects that sample a texture. The matrix is applied
     * to the src space position to compute texture coordinates.
     */
    void addColorTextureProcessor(GrTexture*, sk_sp<GrColorSpaceXform>, const SkMatrix&);
    void addCoverageTextureProcessor(GrTexture*, const SkMatrix&);
    void addColorTextureProcessor(GrTexture*, sk_sp<GrColorSpaceXform>, const SkMatrix&,
                                  const GrSamplerParams&);
    void addCoverageTextureProcessor(GrTexture*, const SkMatrix&, const GrSamplerParams&);

    int numColorFragmentProcessors() const { return fColorFragmentProcessors.count(); }
    int numCoverageFragmentProcessors() const { return fCoverageFragmentProcessors.count(); }
    int numTotalFragmentProcessors() const { return this->numColorFragmentProcessors() +
                                              this->numCoverageFragmentProcessors(); }

    GrXPFactory* getXPFactory() const {
        return fXPFactory.get();
    }

    GrFragmentProcessor* getColorFragmentProcessor(int i) const {
        return fColorFragmentProcessors[i].get();
    }
    GrFragmentProcessor* getCoverageFragmentProcessor(int i) const {
        return fCoverageFragmentProcessors[i].get();
    }

    GrPaint& operator=(const GrPaint& paint) {
        fDisableOutputConversionToSRGB = paint.fDisableOutputConversionToSRGB;
        fAllowSRGBInputs = paint.fAllowSRGBInputs;
        fUsesDistanceVectorField = paint.fUsesDistanceVectorField;

        fColor = paint.fColor;
        fColorFragmentProcessors = paint.fColorFragmentProcessors;
        fCoverageFragmentProcessors = paint.fCoverageFragmentProcessors;

        fXPFactory = paint.fXPFactory;

        return *this;
    }

    /**
     * Returns true if the paint's output color will be constant after blending. If the result is
     * true, constantColor will be updated to contain the constant color. Note that we can conflate
     * coverage and color, so the actual values written to pixels with partial coverage may still
     * not seem constant, even if this function returns true.
     */
    bool isConstantBlendedColor(GrColor* constantColor) const {
        GrColor paintColor = this->getColor();
        if (!fXPFactory && fColorFragmentProcessors.empty()) {
            if (!GrColorIsOpaque(paintColor)) {
                return false;
            }
            *constantColor = paintColor;
            return true;
        }
        return this->internalIsConstantBlendedColor(paintColor, constantColor);
    }

private:
    bool internalIsConstantBlendedColor(GrColor paintColor, GrColor* constantColor) const;

    mutable sk_sp<GrXPFactory>                fXPFactory;
    SkSTArray<4, sk_sp<GrFragmentProcessor>>  fColorFragmentProcessors;
    SkSTArray<2, sk_sp<GrFragmentProcessor>>  fCoverageFragmentProcessors;

    bool                                      fDisableOutputConversionToSRGB;
    bool                                      fAllowSRGBInputs;
    bool                                      fUsesDistanceVectorField;

    GrColor4f                                 fColor;
};

#endif
