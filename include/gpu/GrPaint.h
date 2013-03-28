
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrPaint_DEFINED
#define GrPaint_DEFINED

#include "GrColor.h"
#include "GrEffectStage.h"

#include "SkXfermode.h"

/**
 * The paint describes how color and coverage are computed at each pixel by GrContext draw
 * functions and the how color is blended with the destination pixel.
 *
 * The paint allows installation of custom color and coverage stages. New types of stages are
 * created by subclassing GrEffect.
 *
 * The primitive color computation starts with the color specified by setColor(). This color is the
 * input to the first color stage. Each color stage feeds its output to the next color stage. The
 * final color stage's output color is input to the color filter specified by
 * setXfermodeColorFilter which produces the final source color, S.
 *
 * Fractional pixel coverage follows a similar flow. The coverage is initially the value specified
 * by setCoverage(). This is input to the first coverage stage. Coverage stages are chained
 * together in the same manner as color stages. The output of the last stage is modulated by any
 * fractional coverage produced by anti-aliasing. This last step produces the final coverage, C.
 *
 * setBlendFunc() specifies blending coefficients for S (described above) and D, the initial value
 * of the destination pixel, labeled Bs and Bd respectively. The final value of the destination
 * pixel is then D' = (1-C)*D + C*(Bd*D + Bs*S).
 *
 * Note that the coverage is applied after the blend. This is why they are computed as distinct
 * values.
 *
 * TODO: Encapsulate setXfermodeColorFilter in a GrEffect and remove from GrPaint.
 */
class GrPaint {
public:
    enum {
        kMaxColorStages     = 3,
        kMaxCoverageStages  = 1,
    };

    GrPaint() { this->reset(); }

    GrPaint(const GrPaint& paint) { *this = paint; }

    ~GrPaint() {}

    /**
     * Sets the blending coefficients to use to blend the final primitive color with the
     * destination color. Defaults to kOne for src and kZero for dst (i.e. src mode).
     */
    void setBlendFunc(GrBlendCoeff srcCoeff, GrBlendCoeff dstCoeff) {
        fSrcBlendCoeff = srcCoeff;
        fDstBlendCoeff = dstCoeff;
    }
    GrBlendCoeff getSrcBlendCoeff() const { return fSrcBlendCoeff; }
    GrBlendCoeff getDstBlendCoeff() const { return fDstBlendCoeff; }

    /**
     * The initial color of the drawn primitive. Defaults to solid white.
     */
    void setColor(GrColor color) { fColor = color; }
    GrColor getColor() const { return fColor; }

    /**
     * Applies fractional coverage to the entire drawn primitive. Defaults to 0xff.
     */
    void setCoverage(uint8_t coverage) { fCoverage = coverage; }
    uint8_t getCoverage() const { return fCoverage; }

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

    /**
     * Enables a SkXfermode::Mode-based color filter applied to the primitive color. The constant
     * color passed to this function is considered the "src" color and the primitive's color is
     * considered the "dst" color. Defaults to kDst_Mode which equates to simply passing through
     * the primitive color unmodified.
     */
    void setXfermodeColorFilter(SkXfermode::Mode mode, GrColor color) {
        fColorFilterColor = color;
        fColorFilterXfermode = mode;
    }
    SkXfermode::Mode getColorFilterMode() const { return fColorFilterXfermode; }
    GrColor getColorFilterColor() const { return fColorFilterColor; }

    /**
     * Disables the SkXfermode::Mode color filter.
     */
    void resetColorFilter() {
        fColorFilterXfermode = SkXfermode::kDst_Mode;
        fColorFilterColor = GrColorPackRGBA(0xff, 0xff, 0xff, 0xff);
    }

    /**
     * Specifies a stage of the color pipeline. Usually the texture matrices of color stages apply
     * to the primitive's positions. Some GrContext calls take explicit coords as an array or a
     * rect. In this case these are the pre-matrix coords to colorStage(0).
     */
    GrEffectStage* colorStage(int i) {
        GrAssert((unsigned)i < kMaxColorStages);
        return fColorStages + i;
    }

    const GrEffectStage& getColorStage(int i) const {
        GrAssert((unsigned)i < kMaxColorStages);
        return fColorStages[i];
    }

    bool isColorStageEnabled(int i) const {
        GrAssert((unsigned)i < kMaxColorStages);
        return (NULL != fColorStages[i].getEffect());
    }

    /**
     * Specifies a stage of the coverage pipeline. Coverage stages' texture matrices are always
     * applied to the primitive's position, never to explicit texture coords.
     */
    GrEffectStage* coverageStage(int i) {
        GrAssert((unsigned)i < kMaxCoverageStages);
        return fCoverageStages + i;
    }

    const GrEffectStage& getCoverageStage(int i) const {
        GrAssert((unsigned)i < kMaxCoverageStages);
        return fCoverageStages[i];
    }

    bool isCoverageStageEnabled(int i) const {
        GrAssert((unsigned)i < kMaxCoverageStages);
        return (NULL != fCoverageStages[i].getEffect());
    }

    bool hasCoverageStage() const {
        for (int i = 0; i < kMaxCoverageStages; ++i) {
            if (this->isCoverageStageEnabled(i)) {
                return true;
            }
        }
        return false;
    }

    bool hasColorStage() const {
        for (int i = 0; i < kMaxColorStages; ++i) {
            if (this->isColorStageEnabled(i)) {
                return true;
            }
        }
        return false;
    }

    bool hasStage() const { return this->hasColorStage() || this->hasCoverageStage(); }

    GrPaint& operator=(const GrPaint& paint) {
        fSrcBlendCoeff = paint.fSrcBlendCoeff;
        fDstBlendCoeff = paint.fDstBlendCoeff;
        fAntiAlias = paint.fAntiAlias;
        fDither = paint.fDither;

        fColor = paint.fColor;
        fCoverage = paint.fCoverage;

        fColorFilterColor = paint.fColorFilterColor;
        fColorFilterXfermode = paint.fColorFilterXfermode;

        for (int i = 0; i < kMaxColorStages; ++i) {
            if (paint.isColorStageEnabled(i)) {
                fColorStages[i] = paint.fColorStages[i];
            }
        }
        for (int i = 0; i < kMaxCoverageStages; ++i) {
            if (paint.isCoverageStageEnabled(i)) {
                fCoverageStages[i] = paint.fCoverageStages[i];
            }
        }
        return *this;
    }

    /**
     * Resets the paint to the defaults.
     */
    void reset() {
        this->resetBlend();
        this->resetOptions();
        this->resetColor();
        this->resetCoverage();
        this->resetStages();
        this->resetColorFilter();
    }

    // internal use
    // GrPaint's textures and masks map to the first N stages
    // of GrDrawTarget in that order (textures followed by masks)
    enum {
        kFirstColorStage = 0,
        kFirstCoverageStage = kMaxColorStages,
        kTotalStages = kFirstColorStage + kMaxColorStages + kMaxCoverageStages,
    };

private:
    /**
     * Called when the source coord system from which geometry is rendered changes. It ensures that
     * the local coordinates seen by effects remains unchanged. oldToNew gives the transformation
     * from the previous coord system to the new coord system.
     */
    void localCoordChange(const SkMatrix& oldToNew) {
        for (int i = 0; i < kMaxColorStages; ++i) {
            if (this->isColorStageEnabled(i)) {
                fColorStages[i].localCoordChange(oldToNew);
            }
        }
        for (int i = 0; i < kMaxCoverageStages; ++i) {
            if (this->isCoverageStageEnabled(i)) {
                fCoverageStages[i].localCoordChange(oldToNew);
            }
        }
    }

    bool localCoordChangeInverse(const SkMatrix& newToOld) {
        SkMatrix oldToNew;
        bool computed = false;
        for (int i = 0; i < kMaxColorStages; ++i) {
            if (this->isColorStageEnabled(i)) {
                if (!computed && !newToOld.invert(&oldToNew)) {
                    return false;
                } else {
                    computed = true;
                }
                fColorStages[i].localCoordChange(oldToNew);
            }
        }
        for (int i = 0; i < kMaxCoverageStages; ++i) {
            if (this->isCoverageStageEnabled(i)) {
                if (!computed && !newToOld.invert(&oldToNew)) {
                    return false;
                } else {
                    computed = true;
                }
                fCoverageStages[i].localCoordChange(oldToNew);
            }
        }
        return true;
    }

    friend class GrContext; // To access above two functions

    GrEffectStage               fColorStages[kMaxColorStages];
    GrEffectStage               fCoverageStages[kMaxCoverageStages];

    GrBlendCoeff                fSrcBlendCoeff;
    GrBlendCoeff                fDstBlendCoeff;
    bool                        fAntiAlias;
    bool                        fDither;

    GrColor                     fColor;
    uint8_t                     fCoverage;

    GrColor                     fColorFilterColor;
    SkXfermode::Mode            fColorFilterXfermode;

    void resetBlend() {
        fSrcBlendCoeff = kOne_GrBlendCoeff;
        fDstBlendCoeff = kZero_GrBlendCoeff;
    }

    void resetOptions() {
        fAntiAlias = false;
        fDither = false;
    }

    void resetColor() {
        fColor = GrColorPackRGBA(0xff, 0xff, 0xff, 0xff);
    }

    void resetCoverage() {
        fCoverage = 0xff;
    }

    void resetStages() {
        for (int i = 0; i < kMaxColorStages; ++i) {
            fColorStages[i].reset();
        }
        for (int i = 0; i < kMaxCoverageStages; ++i) {
            fCoverageStages[i].reset();
        }
    }
};

#endif
