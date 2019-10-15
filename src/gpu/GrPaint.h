
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrPaint_DEFINED
#define GrPaint_DEFINED

#include "include/core/SkBlendMode.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkRegion.h"
#include "src/core/SkTLazy.h"
#include "src/gpu/GrColor.h"
#include "src/gpu/GrFragmentProcessor.h"

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
    ~GrPaint() = default;

    static GrPaint Clone(const GrPaint& src) { return GrPaint(src); }

    /**
     * The initial color of the drawn primitive. Defaults to solid white.
     */
    void setColor4f(const SkPMColor4f& color) { fColor = color; }
    const SkPMColor4f& getColor4f() const { return fColor; }

    void setXPFactory(const GrXPFactory* xpFactory) {
        fXPFactory = xpFactory;
        fTrivial &= !SkToBool(xpFactory);
    }

    void setPorterDuffXPFactory(SkBlendMode mode);

    void setCoverageSetOpXPFactory(SkRegion::Op, bool invertCoverage = false);

    /**
     * Appends an additional color processor to the color computation.
     */
    void addColorFragmentProcessor(std::unique_ptr<GrFragmentProcessor> fp) {
        SkASSERT(fp);
        fColorFragmentProcessors.push_back(std::move(fp));
        fTrivial = false;
    }

    /**
     * Appends an additional coverage processor to the coverage computation.
     */
    void addCoverageFragmentProcessor(std::unique_ptr<GrFragmentProcessor> fp) {
        SkASSERT(fp);
        fCoverageFragmentProcessors.push_back(std::move(fp));
        fTrivial = false;
    }

    /**
     * Helpers for adding color or coverage effects that sample a texture. The matrix is applied
     * to the src space position to compute texture coordinates.
     */
    void addColorTextureProcessor(sk_sp<GrTextureProxy>, GrColorType srcColorType, const SkMatrix&);
    void addColorTextureProcessor(sk_sp<GrTextureProxy>, GrColorType srcColorType, const SkMatrix&,
                                  const GrSamplerState&);

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
    bool isConstantBlendedColor(SkPMColor4f* constantColor) const;

    /**
     * A trivial paint is one that uses src-over and has no fragment processors.
     * It may have variable sRGB settings.
     **/
    bool isTrivial() const { return fTrivial; }

    friend void assert_alive(GrPaint& p) {
        SkASSERT(p.fAlive);
    }

private:
    // Since paint copying is expensive if there are fragment processors, we require going through
    // the Clone() method.
    GrPaint(const GrPaint&);
    GrPaint& operator=(const GrPaint&) = delete;

    friend class GrProcessorSet;

    const GrXPFactory* fXPFactory = nullptr;
    SkSTArray<4, std::unique_ptr<GrFragmentProcessor>> fColorFragmentProcessors;
    SkSTArray<2, std::unique_ptr<GrFragmentProcessor>> fCoverageFragmentProcessors;
    bool fTrivial = true;
    SkPMColor4f fColor = SK_PMColor4fWHITE;
    SkDEBUGCODE(bool fAlive = true;)  // Set false after moved from.
};

#endif
