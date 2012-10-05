
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrPaint_DEFINED
#define GrPaint_DEFINED

#include "GrTexture.h"
#include "GrColor.h"
#include "GrSamplerState.h"

#include "SkXfermode.h"

/**
 * The paint describes how color and coverage are computed at each pixel by GrContext draw
 * functions and the how color is blended with the destination pixel.
 *
 * The paint allows installation of custom color and coverage stages. New types of stages are
 * created by subclassing GrCustomStage.
 *
 * The primitive color computation starts with the color specified by setColor(). This color is the
 * input to the first color stage. Each color stage feeds its output to the next color stage. The
 * final color stage's output color is input to the color filter specified by
 * setXfermodeColorFilter which it turn feeds into the color matrix. The output of the color matrix
 * is the final source color, S.
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
 * TODO: Encapsulate setXfermodeColorFilter and color matrix in stages and remove from GrPaint.
 */
class GrPaint {
public:
    enum {
        kMaxColorStages     = 2,
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
     * Turns off application of a color matrix. By default the color matrix is disabled.
     */
    void disableColorMatrix() { fColorMatrixEnabled = false; }

    /**
     * Specifies and enables a 4 x 5 color matrix.
     */
    void setColorMatrix(const float matrix[20]) {
        fColorMatrixEnabled = true;
        memcpy(fColorMatrix, matrix, sizeof(fColorMatrix));
    }

    bool isColorMatrixEnabled() const { return fColorMatrixEnabled; }
    const float* getColorMatrix() const { return fColorMatrix; }

    /**
     * Disables both the matrix and SkXfermode::Mode color filters.
     */
    void resetColorFilter() {
        fColorFilterXfermode = SkXfermode::kDst_Mode;
        fColorFilterColor = GrColorPackRGBA(0xff, 0xff, 0xff, 0xff);
        fColorMatrixEnabled = false;
    }

    /**
     * Specifies a stage of the color pipeline. Usually the texture matrices of color stages apply
     * to the primitive's positions. Some GrContext calls take explicit coords as an array or a
     * rect. In this case these are the pre-matrix coords to colorSampler(0).
     */
    GrSamplerState* colorSampler(int i) {
        GrAssert((unsigned)i < kMaxColorStages);
        return fColorSamplers + i;
    }

    const GrSamplerState& getColorSampler(int i) const {
        GrAssert((unsigned)i < kMaxColorStages);
        return fColorSamplers[i];
    }

    bool isColorStageEnabled(int i) const {
        GrAssert((unsigned)i < kMaxColorStages);
        return (NULL != fColorSamplers[i].getCustomStage());
    }

    /**
     * Specifies a stage of the coverage pipeline. Coverage stages' texture matrices are always
     * applied to the primitive's position, never to explicit texture coords.
     */
    GrSamplerState* coverageSampler(int i) {
        GrAssert((unsigned)i < kMaxCoverageStages);
        return fCoverageSamplers + i;
    }

    const GrSamplerState& getCoverageSampler(int i) const {
        GrAssert((unsigned)i < kMaxCoverageStages);
        return fCoverageSamplers[i];
    }

    bool isCoverageStageEnabled(int i) const {
        GrAssert((unsigned)i < kMaxCoverageStages);
        return (NULL != fCoverageSamplers[i].getCustomStage());
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

    /**
     * Preconcats the matrix of all samplers in the mask with the inverse of a matrix. If the
     * matrix inverse cannot be computed (and there is at least one enabled stage) then false is
     * returned.
     */
    bool preConcatSamplerMatricesWithInverse(const GrMatrix& matrix) {
        GrMatrix inv;
        bool computed = false;
        for (int i = 0; i < kMaxColorStages; ++i) {
            if (this->isColorStageEnabled(i)) {
                if (!computed && !matrix.invert(&inv)) {
                    return false;
                } else {
                    computed = true;
                }
                fColorSamplers[i].preConcatMatrix(inv);
            }
        }
        for (int i = 0; i < kMaxCoverageStages; ++i) {
            if (this->isCoverageStageEnabled(i)) {
                if (!computed && !matrix.invert(&inv)) {
                    return false;
                } else {
                    computed = true;
                }
                fCoverageSamplers[i].preConcatMatrix(inv);
            }
        }
        return true;
    }

    GrPaint& operator=(const GrPaint& paint) {
        fSrcBlendCoeff = paint.fSrcBlendCoeff;
        fDstBlendCoeff = paint.fDstBlendCoeff;
        fAntiAlias = paint.fAntiAlias;
        fDither = paint.fDither;

        fColor = paint.fColor;
        fCoverage = paint.fCoverage;

        fColorFilterColor = paint.fColorFilterColor;
        fColorFilterXfermode = paint.fColorFilterXfermode;
        fColorMatrixEnabled = paint.fColorMatrixEnabled;
        if (fColorMatrixEnabled) {
            memcpy(fColorMatrix, paint.fColorMatrix, sizeof(fColorMatrix));
        }

        for (int i = 0; i < kMaxColorStages; ++i) {
            if (paint.isColorStageEnabled(i)) {
                fColorSamplers[i] = paint.fColorSamplers[i];
            }
        }
        for (int i = 0; i < kMaxCoverageStages; ++i) {
            if (paint.isCoverageStageEnabled(i)) {
                fCoverageSamplers[i] = paint.fCoverageSamplers[i];
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
        this->resetTextures();
        this->resetColorFilter();
        this->resetMasks();
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

    GrSamplerState              fColorSamplers[kMaxColorStages];
    GrSamplerState              fCoverageSamplers[kMaxCoverageStages];

    GrBlendCoeff                fSrcBlendCoeff;
    GrBlendCoeff                fDstBlendCoeff;
    bool                        fAntiAlias;
    bool                        fDither;
    bool                        fColorMatrixEnabled;

    GrColor                     fColor;
    uint8_t                     fCoverage;

    GrColor                     fColorFilterColor;
    SkXfermode::Mode            fColorFilterXfermode;
    float                       fColorMatrix[20];

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

    void resetTextures() {
        for (int i = 0; i < kMaxColorStages; ++i) {
            fColorSamplers[i].reset();
        }
    }

    void resetMasks() {
        for (int i = 0; i < kMaxCoverageStages; ++i) {
            fCoverageSamplers[i].reset();
        }
    }
};

#endif
