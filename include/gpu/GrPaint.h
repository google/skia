
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
     * Appends an additional color effect to the color computation.
     */
    const GrEffect* addColorEffect(const GrEffect* effect, int attr0 = -1, int attr1 = -1) {
        SkASSERT(NULL != effect);
        if (!effect->willUseInputColor()) {
            fColorStages.reset();
        }
        SkNEW_APPEND_TO_TARRAY(&fColorStages, GrEffectStage, (effect, attr0, attr1));
        return effect;
    }

    /**
     * Appends an additional coverage effect to the coverage computation.
     */
    const GrEffect* addCoverageEffect(const GrEffect* effect, int attr0 = -1, int attr1 = -1) {
        SkASSERT(NULL != effect);
        if (!effect->willUseInputColor()) {
            fCoverageStages.reset();
        }
        SkNEW_APPEND_TO_TARRAY(&fCoverageStages, GrEffectStage, (effect, attr0, attr1));
        return effect;
    }

    /**
     * Helpers for adding color or coverage effects that sample a texture. The matrix is applied
     * to the src space position to compute texture coordinates.
     */
    void addColorTextureEffect(GrTexture* texture, const SkMatrix& matrix);
    void addCoverageTextureEffect(GrTexture* texture, const SkMatrix& matrix);

    void addColorTextureEffect(GrTexture* texture,
                               const SkMatrix& matrix,
                               const GrTextureParams& params);
    void addCoverageTextureEffect(GrTexture* texture,
                                  const SkMatrix& matrix,
                                  const GrTextureParams& params);

    int numColorStages() const { return fColorStages.count(); }
    int numCoverageStages() const { return fCoverageStages.count(); }
    int numTotalStages() const { return this->numColorStages() + this->numCoverageStages(); }

    const GrEffectStage& getColorStage(int s) const { return fColorStages[s]; }
    const GrEffectStage& getCoverageStage(int s) const { return fCoverageStages[s]; }

    GrPaint& operator=(const GrPaint& paint) {
        fSrcBlendCoeff = paint.fSrcBlendCoeff;
        fDstBlendCoeff = paint.fDstBlendCoeff;
        fAntiAlias = paint.fAntiAlias;
        fDither = paint.fDither;

        fColor = paint.fColor;
        fCoverage = paint.fCoverage;

        fColorStages = paint.fColorStages;
        fCoverageStages = paint.fCoverageStages;

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
    }

    /**
     * Determines whether the drawing with this paint is opaque with respect to both color blending
     * and fractional coverage. It does not consider whether AA has been enabled on the paint or
     * not. Depending upon whether multisampling or coverage-based AA is in use, AA may make the
     * result only apply to the interior of primitives.
     *
     */
    bool isOpaque() const;

    /**
     * Returns true if isOpaque would return true and the paint represents a solid constant color
     * draw. If the result is true, constantColor will be updated to contain the constant color.
     */
    bool isOpaqueAndConstantColor(GrColor* constantColor) const;

private:

    /**
     * Helper for isOpaque and isOpaqueAndConstantColor.
     */
    bool getOpaqueAndKnownColor(GrColor* solidColor, uint32_t* solidColorKnownComponents) const;

    /**
     * Called when the source coord system from which geometry is rendered changes. It ensures that
     * the local coordinates seen by effects remains unchanged. oldToNew gives the transformation
     * from the previous coord system to the new coord system.
     */
    void localCoordChange(const SkMatrix& oldToNew) {
        for (int i = 0; i < fColorStages.count(); ++i) {
            fColorStages[i].localCoordChange(oldToNew);
        }
        for (int i = 0; i < fCoverageStages.count(); ++i) {
            fCoverageStages[i].localCoordChange(oldToNew);
        }
    }

    bool localCoordChangeInverse(const SkMatrix& newToOld) {
        SkMatrix oldToNew;
        bool computed = false;
        for (int i = 0; i < fColorStages.count(); ++i) {
            if (!computed && !newToOld.invert(&oldToNew)) {
                return false;
            } else {
                computed = true;
            }
            fColorStages[i].localCoordChange(oldToNew);
        }
        for (int i = 0; i < fCoverageStages.count(); ++i) {
            if (!computed && !newToOld.invert(&oldToNew)) {
                return false;
            } else {
                computed = true;
            }
            fCoverageStages[i].localCoordChange(oldToNew);
        }
        return true;
    }

    friend class GrContext; // To access above two functions
    friend class GrStencilAndCoverTextContext;  // To access above two functions

    SkSTArray<4, GrEffectStage> fColorStages;
    SkSTArray<2, GrEffectStage> fCoverageStages;

    GrBlendCoeff                fSrcBlendCoeff;
    GrBlendCoeff                fDstBlendCoeff;
    bool                        fAntiAlias;
    bool                        fDither;

    GrColor                     fColor;
    uint8_t                     fCoverage;

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
        fColorStages.reset();
        fCoverageStages.reset();
    }
};

#endif
