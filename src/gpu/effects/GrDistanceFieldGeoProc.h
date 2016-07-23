/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDistanceFieldGeoProc_DEFINED
#define GrDistanceFieldGeoProc_DEFINED

#include "GrProcessor.h"
#include "GrGeometryProcessor.h"

class GrGLDistanceFieldA8TextGeoProc;
class GrGLDistanceFieldPathGeoProc;
class GrGLDistanceFieldLCDTextGeoProc;
class GrInvariantOutput;

enum GrDistanceFieldEffectFlags {
    kSimilarity_DistanceFieldEffectFlag   = 0x01, // ctm is similarity matrix
    kScaleOnly_DistanceFieldEffectFlag    = 0x02, // ctm has only scale and translate
    kUseLCD_DistanceFieldEffectFlag       = 0x04, // use lcd text
    kBGR_DistanceFieldEffectFlag          = 0x08, // lcd display has bgr order
    kPortrait_DistanceFieldEffectFlag     = 0x10, // lcd display is in portrait mode (not used yet)
    kGammaCorrect_DistanceFieldEffectFlag = 0x20, // assume gamma-correct output (linear blending)

    kInvalid_DistanceFieldEffectFlag    = 0x80,   // invalid state (for initialization)

    kUniformScale_DistanceFieldEffectMask = kSimilarity_DistanceFieldEffectFlag |
                                            kScaleOnly_DistanceFieldEffectFlag,
    // The subset of the flags relevant to GrDistanceFieldA8TextGeoProc
    kNonLCD_DistanceFieldEffectMask       = kSimilarity_DistanceFieldEffectFlag |
                                            kScaleOnly_DistanceFieldEffectFlag |
                                            kGammaCorrect_DistanceFieldEffectFlag,
    // The subset of the flags relevant to GrDistanceFieldLCDTextGeoProc
    kLCD_DistanceFieldEffectMask          = kSimilarity_DistanceFieldEffectFlag |
                                            kScaleOnly_DistanceFieldEffectFlag |
                                            kUseLCD_DistanceFieldEffectFlag |
                                            kBGR_DistanceFieldEffectFlag |
                                            kGammaCorrect_DistanceFieldEffectFlag,
};

/**
 * The output color of this effect is a modulation of the input color and a sample from a
 * distance field texture (using a smoothed step function near 0.5).
 * It allows explicit specification of the filtering and wrap modes (GrTextureParams). The input
 * coords are a custom attribute. Gamma correction is handled via a texture LUT.
 */
class GrDistanceFieldA8TextGeoProc : public GrGeometryProcessor {
public:
#ifdef SK_GAMMA_APPLY_TO_A8
    static sk_sp<GrGeometryProcessor> Make(GrColor color, const SkMatrix& viewMatrix,
                                           GrTexture* tex, const GrTextureParams& params,
                                           float lum, uint32_t flags, bool usesLocalCoords) {
        return sk_sp<GrGeometryProcessor>(
            new GrDistanceFieldA8TextGeoProc(color, viewMatrix, tex, params, lum, flags,
                                             usesLocalCoords));
    }
#else
    static sk_sp<GrGeometryProcessor> Make(GrColor color, const SkMatrix& viewMatrix,
                                           GrTexture* tex, const GrTextureParams& params,
                                           uint32_t flags, bool usesLocalCoords) {
        return sk_sp<GrGeometryProcessor>(
            new GrDistanceFieldA8TextGeoProc(color, viewMatrix, tex, params, flags,
                                             usesLocalCoords));
    }
#endif

    virtual ~GrDistanceFieldA8TextGeoProc() {}

    const char* name() const override { return "DistanceFieldTexture"; }

    const Attribute* inPosition() const { return fInPosition; }
    const Attribute* inColor() const { return fInColor; }
    const Attribute* inTextureCoords() const { return fInTextureCoords; }
    GrColor color() const { return fColor; }
    bool colorIgnored() const { return GrColor_ILLEGAL == fColor; }
    const SkMatrix& viewMatrix() const { return fViewMatrix; }
    bool usesLocalCoords() const { return fUsesLocalCoords; }
#ifdef SK_GAMMA_APPLY_TO_A8
    float getDistanceAdjust() const { return fDistanceAdjust; }
#endif
    uint32_t getFlags() const { return fFlags; }

    void getGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override;

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrGLSLCaps&) const override;

private:
    GrDistanceFieldA8TextGeoProc(GrColor, const SkMatrix& viewMatrix,
                                 GrTexture* texture, const GrTextureParams& params,
#ifdef SK_GAMMA_APPLY_TO_A8
                                 float distanceAdjust,
#endif
                                 uint32_t flags, bool usesLocalCoords);

    GrColor          fColor;
    SkMatrix         fViewMatrix;
    GrTextureAccess  fTextureAccess;
#ifdef SK_GAMMA_APPLY_TO_A8
    float            fDistanceAdjust;
#endif
    uint32_t         fFlags;
    const Attribute* fInPosition;
    const Attribute* fInColor;
    const Attribute* fInTextureCoords;
    bool             fUsesLocalCoords;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST;

    typedef GrGeometryProcessor INHERITED;
};


/**
* The output color of this effect is a modulation of the input color and a sample from a
* distance field texture (using a smoothed step function near 0.5).
* It allows explicit specification of the filtering and wrap modes (GrTextureParams). The input
* coords are a custom attribute. No gamma correct blending is applied. Used for paths only.
*/
class GrDistanceFieldPathGeoProc : public GrGeometryProcessor {
public:
    static sk_sp<GrGeometryProcessor> Make(GrColor color, const SkMatrix& viewMatrix,
                                           GrTexture* tex, const GrTextureParams& params,
                                           uint32_t flags, bool usesLocalCoords) {
        return sk_sp<GrGeometryProcessor>(
            new GrDistanceFieldPathGeoProc(color, viewMatrix, tex, params, flags, usesLocalCoords));
    }

    virtual ~GrDistanceFieldPathGeoProc() {}

    const char* name() const override { return "DistanceFieldTexture"; }

    const Attribute* inPosition() const { return fInPosition; }
    const Attribute* inColor() const { return fInColor; }
    const Attribute* inTextureCoords() const { return fInTextureCoords; }
    GrColor color() const { return fColor; }
    bool colorIgnored() const { return GrColor_ILLEGAL == fColor; }
    const SkMatrix& viewMatrix() const { return fViewMatrix; }
    uint32_t getFlags() const { return fFlags; }
    bool usesLocalCoords() const { return fUsesLocalCoords; }

    void getGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override;

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrGLSLCaps&) const override;

private:
    GrDistanceFieldPathGeoProc(GrColor, const SkMatrix& viewMatrix, GrTexture* texture,
                               const GrTextureParams& params, uint32_t flags,
                               bool usesLocalCoords);

    GrColor          fColor;
    SkMatrix         fViewMatrix;
    GrTextureAccess  fTextureAccess;
    uint32_t         fFlags;
    const Attribute* fInPosition;
    const Attribute* fInColor;
    const Attribute* fInTextureCoords;
    bool             fUsesLocalCoords;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST;

    typedef GrGeometryProcessor INHERITED;
};

/**
 * The output color of this effect is a modulation of the input color and samples from a
 * distance field texture (using a smoothed step function near 0.5), adjusted for LCD displays.
 * It allows explicit specification of the filtering and wrap modes (GrTextureParams). The input
 * coords are a custom attribute. Gamma correction is handled via a texture LUT.
 */
class GrDistanceFieldLCDTextGeoProc : public GrGeometryProcessor {
public:
    struct DistanceAdjust {
        SkScalar fR, fG, fB;
        static DistanceAdjust Make(SkScalar r, SkScalar g, SkScalar b) {
            DistanceAdjust result;
            result.fR = r; result.fG = g; result.fB = b;
            return result;
        }
        bool operator==(const DistanceAdjust& wa) const {
            return (fR == wa.fR && fG == wa.fG && fB == wa.fB);
        }
        bool operator!=(const DistanceAdjust& wa) const {
            return !(*this == wa);
        }
    };

    static sk_sp<GrGeometryProcessor> Make(GrColor color, const SkMatrix& viewMatrix,
                                           GrTexture* tex, const GrTextureParams& params,
                                           DistanceAdjust distanceAdjust, uint32_t flags,
                                           bool usesLocalCoords) {
        return sk_sp<GrGeometryProcessor>(
            new GrDistanceFieldLCDTextGeoProc(color, viewMatrix, tex, params, distanceAdjust,
                                              flags, usesLocalCoords));
    }

    virtual ~GrDistanceFieldLCDTextGeoProc() {}

    const char* name() const override { return "DistanceFieldLCDTexture"; }

    const Attribute* inPosition() const { return fInPosition; }
    const Attribute* inColor() const { return fInColor; }
    const Attribute* inTextureCoords() const { return fInTextureCoords; }
    DistanceAdjust getDistanceAdjust() const { return fDistanceAdjust; }
    GrColor color() const { return fColor; }
    bool colorIgnored() const { return GrColor_ILLEGAL == fColor; }
    const SkMatrix& viewMatrix() const { return fViewMatrix; }
    uint32_t getFlags() const { return fFlags; }
    bool usesLocalCoords() const { return fUsesLocalCoords; }

    void getGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override;

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrGLSLCaps&) const override;

private:
    GrDistanceFieldLCDTextGeoProc(GrColor, const SkMatrix& viewMatrix,
                                  GrTexture* texture, const GrTextureParams& params,
                                  DistanceAdjust wa, uint32_t flags,
                                  bool usesLocalCoords);

    GrColor          fColor;
    SkMatrix         fViewMatrix;
    GrTextureAccess  fTextureAccess;
    DistanceAdjust   fDistanceAdjust;
    uint32_t         fFlags;
    const Attribute* fInPosition;
    const Attribute* fInColor;
    const Attribute* fInTextureCoords;
    bool             fUsesLocalCoords;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST;

    typedef GrGeometryProcessor INHERITED;
};

#endif
