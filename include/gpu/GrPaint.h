
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrPaint_DEFINED
#define GrPaint_DEFINED

#include "GrColor.h"
#include "GrStagedProcessor.h"
#include "GrProcessorDataManager.h"
#include "GrXferProcessor.h"
#include "effects/GrPorterDuffXferProcessor.h"

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

    ~GrPaint() {}

    /**
     * The initial color of the drawn primitive. Defaults to solid white.
     */
    void setColor(GrColor color) { fColor = color; }
    GrColor getColor() const { return fColor; }

    /**
     * Should primitives be anti-aliased or not. Defaults to false.
     */
    void setAntiAlias(bool aa) { fAntiAlias = aa; }
    bool isAntiAlias() const { return fAntiAlias; }

    /**
     * Should dithering be applied. Defaults to false.
     */
    void setDither(bool dither) { fDither = dither; }
    bool isDither() const { return fDither; }

    const GrXPFactory* setXPFactory(const GrXPFactory* xpFactory) {
        fXPFactory.reset(SkRef(xpFactory));
        return xpFactory;
    }

    void setPorterDuffXPFactory(SkXfermode::Mode mode) {
        fXPFactory.reset(GrPorterDuffXPFactory::Create(mode));
    }

    void setCoverageSetOpXPFactory(SkRegion::Op regionOp, bool invertCoverage = false); 

    /**
     * Appends an additional color processor to the color computation.
     */
    const GrFragmentProcessor* addColorProcessor(const GrFragmentProcessor* fp) {
        SkASSERT(fp);
        SkNEW_APPEND_TO_TARRAY(&fColorStages, GrFragmentStage, (fp));
        return fp;
    }

    /**
     * Appends an additional coverage processor to the coverage computation.
     */
    const GrFragmentProcessor* addCoverageProcessor(const GrFragmentProcessor* fp) {
        SkASSERT(fp);
        SkNEW_APPEND_TO_TARRAY(&fCoverageStages, GrFragmentStage, (fp));
        return fp;
    }

    /**
     * Helpers for adding color or coverage effects that sample a texture. The matrix is applied
     * to the src space position to compute texture coordinates.
     */
    void addColorTextureProcessor(GrTexture*, const SkMatrix&);
    void addCoverageTextureProcessor(GrTexture*, const SkMatrix&);
    void addColorTextureProcessor(GrTexture*, const SkMatrix&, const GrTextureParams&);
    void addCoverageTextureProcessor(GrTexture*, const SkMatrix&, const GrTextureParams&);

    int numColorStages() const { return fColorStages.count(); }
    int numCoverageStages() const { return fCoverageStages.count(); }
    int numTotalStages() const { return this->numColorStages() + this->numCoverageStages(); }

    const GrXPFactory* getXPFactory() const {
        if (!fXPFactory) {
            fXPFactory.reset(GrPorterDuffXPFactory::Create(SkXfermode::kSrc_Mode));
        }
        return fXPFactory.get();
    }

    const GrFragmentStage& getColorStage(int s) const { return fColorStages[s]; }
    const GrFragmentStage& getCoverageStage(int s) const { return fCoverageStages[s]; }

    GrPaint& operator=(const GrPaint& paint) {
        fAntiAlias = paint.fAntiAlias;
        fDither = paint.fDither;

        fColor = paint.fColor;

        fColorStages = paint.fColorStages;
        fCoverageStages = paint.fCoverageStages;

        fXPFactory.reset(SkRef(paint.getXPFactory()));
        fProcDataManager.reset(SkNEW_ARGS(GrProcessorDataManager, (*paint.processorDataManager())));

        return *this;
    }

    /**
     * Returns true if the paint's output color will be constant after blending. If the result is
     * true, constantColor will be updated to contain the constant color. Note that we can conflate
     * coverage and color, so the actual values written to pixels with partial coverage may still
     * not seem constant, even if this function returns true.
     */
    bool isConstantBlendedColor(GrColor* constantColor) const;

    GrProcessorDataManager* getProcessorDataManager() { return fProcDataManager.get(); }

    const GrProcessorDataManager* processorDataManager() const { return fProcDataManager.get(); }

private:
    mutable SkAutoTUnref<const GrXPFactory> fXPFactory;
    SkSTArray<4, GrFragmentStage>        fColorStages;
    SkSTArray<2, GrFragmentStage>        fCoverageStages;

    bool                                 fAntiAlias;
    bool                                 fDither;

    GrColor                              fColor;
    SkAutoTUnref<GrProcessorDataManager> fProcDataManager;
};

#endif
