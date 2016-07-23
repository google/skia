
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrPaint_DEFINED
#define GrPaint_DEFINED

#include "GrColor.h"
#include "GrXferProcessor.h"
#include "effects/GrPorterDuffXferProcessor.h"
#include "GrFragmentProcessor.h"

#include "SkRefCnt.h"
#include "SkRegion.h"
#include "SkXfermode.h"

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
 * Fractional pixel coverage follows a similar flow. The coverage is initially the value specified
 * by setCoverage(). This is input to the first coverage stage. Coverage stages are chained
 * together in the same manner as color stages. The output of the last stage is modulated by any
 * fractional coverage produced by anti-aliasing. This last step produces the final coverage, C.
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
     * Should primitives be anti-aliased or not. Defaults to false.
     */
    void setAntiAlias(bool aa) { fAntiAlias = aa; }
    bool isAntiAlias() const { return fAntiAlias; }

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
        setDisableOutputConversionToSRGB(!gammaCorrect);
        setAllowSRGBInputs(gammaCorrect);
    }

    void setXPFactory(sk_sp<GrXPFactory> xpFactory) {
        fXPFactory = std::move(xpFactory);
    }

    void setPorterDuffXPFactory(SkXfermode::Mode mode) {
        fXPFactory = GrPorterDuffXPFactory::Make(mode);
    }

    void setCoverageSetOpXPFactory(SkRegion::Op regionOp, bool invertCoverage = false); 

    /**
     * Appends an additional color processor to the color computation.
     */
    void addColorFragmentProcessor(sk_sp<GrFragmentProcessor> fp) {
        SkASSERT(fp);
        fColorFragmentProcessors.push_back(std::move(fp));
    }

    /**
     * Appends an additional coverage processor to the coverage computation.
     */
    void addCoverageFragmentProcessor(sk_sp<GrFragmentProcessor> fp) {
        SkASSERT(fp);
        fCoverageFragmentProcessors.push_back(std::move(fp));
    }

    /**
     * Helpers for adding color or coverage effects that sample a texture. The matrix is applied
     * to the src space position to compute texture coordinates.
     */
    void addColorTextureProcessor(GrTexture*, const SkMatrix&);
    void addCoverageTextureProcessor(GrTexture*, const SkMatrix&);
    void addColorTextureProcessor(GrTexture*, const SkMatrix&, const GrTextureParams&);
    void addCoverageTextureProcessor(GrTexture*, const SkMatrix&, const GrTextureParams&);

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
        fAntiAlias = paint.fAntiAlias;
        fDisableOutputConversionToSRGB = paint.fDisableOutputConversionToSRGB;
        fAllowSRGBInputs = paint.fAllowSRGBInputs;

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
    bool isConstantBlendedColor(GrColor* constantColor) const;

private:
    mutable sk_sp<GrXPFactory>                fXPFactory;
    SkSTArray<4, sk_sp<GrFragmentProcessor>>  fColorFragmentProcessors;
    SkSTArray<2, sk_sp<GrFragmentProcessor>>  fCoverageFragmentProcessors;

    bool                                      fAntiAlias;
    bool                                      fDisableOutputConversionToSRGB;
    bool                                      fAllowSRGBInputs;

    GrColor4f                                 fColor;
};

#endif
