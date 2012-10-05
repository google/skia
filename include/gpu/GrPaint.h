
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
 * The paint describes how pixels are colored when the context draws to
 * them. TODO: Make this a "real" class with getters and setters, default
 * values, and documentation.
 */
class GrPaint {
public:
    enum {
        kMaxColorStages     = 2,
        kMaxCoverageStages  = 1,
    };

    // All the paint fields are public except textures/samplers
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

    // The coverage stage's sampler matrix is always applied to the positions
    // (i.e. no explicit texture coordinates)
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
     * Preconcats the matrix of all samplers in the mask with the inverse of a
     * matrix. If the matrix inverse cannot be computed (and there is at least
     * one enabled stage) then false is returned.
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

    // uninitialized
    GrPaint() {
    }

    GrPaint(const GrPaint& paint) {
        *this = paint;
    }

    ~GrPaint() {}

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

    // sets paint to src-over, solid white, no texture, no mask
    void reset() {
        this->resetBlend();
        this->resetOptions();
        this->resetColor();
        this->resetCoverage();
        this->resetTextures();
        this->resetColorFilter();
        this->resetMasks();
    }

    void resetColorFilter() {
        fColorFilterXfermode = SkXfermode::kDst_Mode;
        fColorFilterColor = GrColorPackRGBA(0xff, 0xff, 0xff, 0xff);
        fColorMatrixEnabled = false;
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
