/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArithmeticMode.h"
#include "SkColorPriv.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkString.h"
#include "SkUnPreMultiply.h"
#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrCoordTransform.h"
#include "gl/GrGLProcessor.h"
#include "gl/builders/GrGLProgramBuilder.h"
#include "GrTBackendProcessorFactory.h"
#endif

static const bool gUseUnpremul = false;

class SkArithmeticMode_scalar : public SkXfermode {
public:
    static SkArithmeticMode_scalar* Create(SkScalar k1, SkScalar k2, SkScalar k3, SkScalar k4,
                                           bool enforcePMColor) {
        return SkNEW_ARGS(SkArithmeticMode_scalar, (k1, k2, k3, k4, enforcePMColor));
    }

    virtual void xfer32(SkPMColor dst[], const SkPMColor src[], int count,
                        const SkAlpha aa[]) const SK_OVERRIDE;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkArithmeticMode_scalar)

#if SK_SUPPORT_GPU
    virtual bool asFragmentProcessor(GrFragmentProcessor**,
                                     GrTexture* background) const SK_OVERRIDE;
#endif

private:
    SkArithmeticMode_scalar(SkScalar k1, SkScalar k2, SkScalar k3, SkScalar k4, bool enforcePMColor) {
        fK[0] = k1;
        fK[1] = k2;
        fK[2] = k3;
        fK[3] = k4;
        fEnforcePMColor = enforcePMColor;
    }

#ifdef SK_SUPPORT_LEGACY_DEEPFLATTENING
    SkArithmeticMode_scalar(SkReadBuffer& buffer) : INHERITED(buffer) {
        fK[0] = buffer.readScalar();
        fK[1] = buffer.readScalar();
        fK[2] = buffer.readScalar();
        fK[3] = buffer.readScalar();
        fEnforcePMColor = buffer.readBool();
    }
#endif

    virtual void flatten(SkWriteBuffer& buffer) const SK_OVERRIDE {
        buffer.writeScalar(fK[0]);
        buffer.writeScalar(fK[1]);
        buffer.writeScalar(fK[2]);
        buffer.writeScalar(fK[3]);
        buffer.writeBool(fEnforcePMColor);
    }
    SkScalar fK[4];
    bool fEnforcePMColor;

    friend class SkArithmeticMode;

    typedef SkXfermode INHERITED;
};

SkFlattenable* SkArithmeticMode_scalar::CreateProc(SkReadBuffer& buffer) {
    const SkScalar k1 = buffer.readScalar();
    const SkScalar k2 = buffer.readScalar();
    const SkScalar k3 = buffer.readScalar();
    const SkScalar k4 = buffer.readScalar();
    const bool enforcePMColor = buffer.readBool();
    return Create(k1, k2, k3, k4, enforcePMColor);
}

static int pinToByte(int value) {
    if (value < 0) {
        value = 0;
    } else if (value > 255) {
        value = 255;
    }
    return value;
}

static int arith(SkScalar k1, SkScalar k2, SkScalar k3, SkScalar k4,
                 int src, int dst) {
    SkScalar result = SkScalarMul(k1, src * dst) +
                      SkScalarMul(k2, src) +
                      SkScalarMul(k3, dst) +
                      k4;
    int res = SkScalarRoundToInt(result);
    return pinToByte(res);
}

static int blend(int src, int dst, int scale) {
    return dst + ((src - dst) * scale >> 8);
}

static bool needsUnpremul(int alpha) {
    return 0 != alpha && 0xFF != alpha;
}

void SkArithmeticMode_scalar::xfer32(SkPMColor dst[], const SkPMColor src[],
                                 int count, const SkAlpha aaCoverage[]) const {
    SkScalar k1 = fK[0] / 255;
    SkScalar k2 = fK[1];
    SkScalar k3 = fK[2];
    SkScalar k4 = fK[3] * 255;

    for (int i = 0; i < count; ++i) {
        if ((NULL == aaCoverage) || aaCoverage[i]) {
            SkPMColor sc = src[i];
            SkPMColor dc = dst[i];

            int a, r, g, b;

            if (gUseUnpremul) {
                int sa = SkGetPackedA32(sc);
                int da = SkGetPackedA32(dc);

                int srcNeedsUnpremul = needsUnpremul(sa);
                int dstNeedsUnpremul = needsUnpremul(da);

                if (!srcNeedsUnpremul && !dstNeedsUnpremul) {
                    a = arith(k1, k2, k3, k4, sa, da);
                    r = arith(k1, k2, k3, k4, SkGetPackedR32(sc), SkGetPackedR32(dc));
                    g = arith(k1, k2, k3, k4, SkGetPackedG32(sc), SkGetPackedG32(dc));
                    b = arith(k1, k2, k3, k4, SkGetPackedB32(sc), SkGetPackedB32(dc));
                } else {
                    int sr = SkGetPackedR32(sc);
                    int sg = SkGetPackedG32(sc);
                    int sb = SkGetPackedB32(sc);
                    if (srcNeedsUnpremul) {
                        SkUnPreMultiply::Scale scale = SkUnPreMultiply::GetScale(sa);
                        sr = SkUnPreMultiply::ApplyScale(scale, sr);
                        sg = SkUnPreMultiply::ApplyScale(scale, sg);
                        sb = SkUnPreMultiply::ApplyScale(scale, sb);
                    }

                    int dr = SkGetPackedR32(dc);
                    int dg = SkGetPackedG32(dc);
                    int db = SkGetPackedB32(dc);
                    if (dstNeedsUnpremul) {
                        SkUnPreMultiply::Scale scale = SkUnPreMultiply::GetScale(da);
                        dr = SkUnPreMultiply::ApplyScale(scale, dr);
                        dg = SkUnPreMultiply::ApplyScale(scale, dg);
                        db = SkUnPreMultiply::ApplyScale(scale, db);
                    }

                    a = arith(k1, k2, k3, k4, sa, da);
                    r = arith(k1, k2, k3, k4, sr, dr);
                    g = arith(k1, k2, k3, k4, sg, dg);
                    b = arith(k1, k2, k3, k4, sb, db);
                }
            } else {
                a = arith(k1, k2, k3, k4, SkGetPackedA32(sc), SkGetPackedA32(dc));
                r = arith(k1, k2, k3, k4, SkGetPackedR32(sc), SkGetPackedR32(dc));
                g = arith(k1, k2, k3, k4, SkGetPackedG32(sc), SkGetPackedG32(dc));
                b = arith(k1, k2, k3, k4, SkGetPackedB32(sc), SkGetPackedB32(dc));
                if (fEnforcePMColor) {
                    r = SkMin32(r, a);
                    g = SkMin32(g, a);
                    b = SkMin32(b, a);
                }
            }

            // apply antialias coverage if necessary
            if (aaCoverage && 0xFF != aaCoverage[i]) {
                int scale = aaCoverage[i] + (aaCoverage[i] >> 7);
                a = blend(a, SkGetPackedA32(sc), scale);
                r = blend(r, SkGetPackedR32(sc), scale);
                g = blend(g, SkGetPackedG32(sc), scale);
                b = blend(b, SkGetPackedB32(sc), scale);
            }

            // turn the result back into premul
            if (gUseUnpremul && (0xFF != a)) {
                int scale = a + (a >> 7);
                r = SkAlphaMul(r, scale);
                g = SkAlphaMul(g, scale);
                b = SkAlphaMul(b, scale);
            }
            dst[i] = fEnforcePMColor ? SkPackARGB32(a, r, g, b) : SkPackARGB32NoCheck(a, r, g, b);
        }
    }
}

#ifndef SK_IGNORE_TO_STRING
void SkArithmeticMode_scalar::toString(SkString* str) const {
    str->append("SkArithmeticMode_scalar: ");
    for (int i = 0; i < 4; ++i) {
        str->appendScalar(fK[i]);
        str->append(" ");
    }
    str->appendS32(fEnforcePMColor ? 1 : 0);
}
#endif

///////////////////////////////////////////////////////////////////////////////

static bool fitsInBits(SkScalar x, int bits) {
    return SkScalarAbs(x) < (1 << (bits - 1));
}

#if 0 // UNUSED
static int32_t toDot8(SkScalar x) {
    return (int32_t)(x * 256);
}
#endif

SkXfermode* SkArithmeticMode::Create(SkScalar k1, SkScalar k2,
                                     SkScalar k3, SkScalar k4,
                                     bool enforcePMColor) {
    if (fitsInBits(k1, 8) && fitsInBits(k2, 16) &&
        fitsInBits(k2, 16) && fitsInBits(k2, 24)) {

#if 0 // UNUSED
        int32_t i1 = toDot8(k1);
        int32_t i2 = toDot8(k2);
        int32_t i3 = toDot8(k3);
        int32_t i4 = toDot8(k4);
        if (i1) {
            return SkNEW_ARGS(SkArithmeticMode_quad, (i1, i2, i3, i4));
        }
        if (0 == i2) {
            return SkNEW_ARGS(SkArithmeticMode_dst, (i3, i4));
        }
        if (0 == i3) {
            return SkNEW_ARGS(SkArithmeticMode_src, (i2, i4));
        }
        return SkNEW_ARGS(SkArithmeticMode_linear, (i2, i3, i4));
#endif
    }
    return SkArithmeticMode_scalar::Create(k1, k2, k3, k4, enforcePMColor);
}


//////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

class GrGLArithmeticEffect : public GrGLFragmentProcessor {
public:
    GrGLArithmeticEffect(const GrBackendProcessorFactory&, const GrProcessor&);
    virtual ~GrGLArithmeticEffect();

    virtual void emitCode(GrGLProgramBuilder*,
                          const GrFragmentProcessor&,
                          const GrProcessorKey&,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

    virtual void setData(const GrGLProgramDataManager&, const GrProcessor&) SK_OVERRIDE;

    static void GenKey(const GrProcessor&, const GrGLCaps& caps, GrProcessorKeyBuilder* b);

private:
    GrGLProgramDataManager::UniformHandle fKUni;
    bool fEnforcePMColor;

    typedef GrGLFragmentProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class GrArithmeticEffect : public GrFragmentProcessor {
public:
    static GrFragmentProcessor* Create(float k1, float k2, float k3, float k4, bool enforcePMColor,
                               GrTexture* background) {
        return SkNEW_ARGS(GrArithmeticEffect, (k1, k2, k3, k4, enforcePMColor, background));
    }

    virtual ~GrArithmeticEffect();

    virtual const GrBackendFragmentProcessorFactory& getFactory() const SK_OVERRIDE;

    typedef GrGLArithmeticEffect GLProcessor;
    static const char* Name() { return "Arithmetic"; }
    GrTexture* backgroundTexture() const { return fBackgroundAccess.getTexture(); }

    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

    float k1() const { return fK1; }
    float k2() const { return fK2; }
    float k3() const { return fK3; }
    float k4() const { return fK4; }
    bool enforcePMColor() const { return fEnforcePMColor; }

private:
    virtual bool onIsEqual(const GrProcessor&) const SK_OVERRIDE;

    GrArithmeticEffect(float k1, float k2, float k3, float k4, bool enforcePMColor,
                       GrTexture* background);
    float                       fK1, fK2, fK3, fK4;
    bool                        fEnforcePMColor;
    GrCoordTransform            fBackgroundTransform;
    GrTextureAccess             fBackgroundAccess;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;
    typedef GrFragmentProcessor INHERITED;

};

///////////////////////////////////////////////////////////////////////////////

GrArithmeticEffect::GrArithmeticEffect(float k1, float k2, float k3, float k4,
                                       bool enforcePMColor, GrTexture* background)
  : fK1(k1), fK2(k2), fK3(k3), fK4(k4), fEnforcePMColor(enforcePMColor) {
    if (background) {
        fBackgroundTransform.reset(kLocal_GrCoordSet, background);
        this->addCoordTransform(&fBackgroundTransform);
        fBackgroundAccess.reset(background);
        this->addTextureAccess(&fBackgroundAccess);
    } else {
        this->setWillReadDstColor();
    }
}

GrArithmeticEffect::~GrArithmeticEffect() {
}

bool GrArithmeticEffect::onIsEqual(const GrProcessor& sBase) const {
    const GrArithmeticEffect& s = sBase.cast<GrArithmeticEffect>();
    return fK1 == s.fK1 &&
           fK2 == s.fK2 &&
           fK3 == s.fK3 &&
           fK4 == s.fK4 &&
           fEnforcePMColor == s.fEnforcePMColor &&
           backgroundTexture() == s.backgroundTexture();
}

const GrBackendFragmentProcessorFactory& GrArithmeticEffect::getFactory() const {
    return GrTBackendFragmentProcessorFactory<GrArithmeticEffect>::getInstance();
}

void GrArithmeticEffect::getConstantColorComponents(GrColor* color, uint32_t* validFlags) const {
    // TODO: optimize this
    *validFlags = 0;
}

///////////////////////////////////////////////////////////////////////////////

GrGLArithmeticEffect::GrGLArithmeticEffect(const GrBackendProcessorFactory& factory,
                                           const GrProcessor&)
   : INHERITED(factory),
     fEnforcePMColor(true) {
}

GrGLArithmeticEffect::~GrGLArithmeticEffect() {
}

void GrGLArithmeticEffect::emitCode(GrGLProgramBuilder* builder,
                                    const GrFragmentProcessor& fp,
                                    const GrProcessorKey& key,
                                    const char* outputColor,
                                    const char* inputColor,
                                    const TransformedCoordsArray& coords,
                                    const TextureSamplerArray& samplers) {

    GrTexture* backgroundTex = fp.cast<GrArithmeticEffect>().backgroundTexture();
    GrGLFragmentShaderBuilder* fsBuilder = builder->getFragmentShaderBuilder();
    const char* dstColor;
    if (backgroundTex) {
        fsBuilder->codeAppend("\t\tvec4 bgColor = ");
        fsBuilder->appendTextureLookup(samplers[0], coords[0].c_str(), coords[0].getType());
        fsBuilder->codeAppendf(";\n");
        dstColor = "bgColor";
    } else {
        dstColor = fsBuilder->dstColor();
    }

    SkASSERT(dstColor);
    fKUni = builder->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                kVec4f_GrSLType, "k");
    const char* kUni = builder->getUniformCStr(fKUni);

    // We don't try to optimize for this case at all
    if (NULL == inputColor) {
        fsBuilder->codeAppendf("\t\tconst vec4 src = vec4(1);\n");
    } else {
        fsBuilder->codeAppendf("\t\tvec4 src = %s;\n", inputColor);
        if (gUseUnpremul) {
            fsBuilder->codeAppendf("\t\tsrc.rgb = clamp(src.rgb / src.a, 0.0, 1.0);\n");
        }
    }

    fsBuilder->codeAppendf("\t\tvec4 dst = %s;\n", dstColor);
    if (gUseUnpremul) {
        fsBuilder->codeAppendf("\t\tdst.rgb = clamp(dst.rgb / dst.a, 0.0, 1.0);\n");
    }

    fsBuilder->codeAppendf("\t\t%s = %s.x * src * dst + %s.y * src + %s.z * dst + %s.w;\n", outputColor, kUni, kUni, kUni, kUni);
    fsBuilder->codeAppendf("\t\t%s = clamp(%s, 0.0, 1.0);\n", outputColor, outputColor);
    if (gUseUnpremul) {
        fsBuilder->codeAppendf("\t\t%s.rgb *= %s.a;\n", outputColor, outputColor);
    } else if (fEnforcePMColor) {
        fsBuilder->codeAppendf("\t\t%s.rgb = min(%s.rgb, %s.a);\n", outputColor, outputColor, outputColor);
    }
}

void GrGLArithmeticEffect::setData(const GrGLProgramDataManager& pdman,
                                   const GrProcessor& processor) {
    const GrArithmeticEffect& arith = processor.cast<GrArithmeticEffect>();
    pdman.set4f(fKUni, arith.k1(), arith.k2(), arith.k3(), arith.k4());
    fEnforcePMColor = arith.enforcePMColor();
}

void GrGLArithmeticEffect::GenKey(const GrProcessor& processor,
                                  const GrGLCaps&, GrProcessorKeyBuilder* b) {
    const GrArithmeticEffect& arith = processor.cast<GrArithmeticEffect>();
    uint32_t key = arith.enforcePMColor() ? 1 : 0;
    if (arith.backgroundTexture()) {
        key |= 2;
    }
    b->add32(key);
}

GrFragmentProcessor* GrArithmeticEffect::TestCreate(SkRandom* rand,
                                                    GrContext*,
                                                    const GrDrawTargetCaps&,
                                                    GrTexture*[]) {
    float k1 = rand->nextF();
    float k2 = rand->nextF();
    float k3 = rand->nextF();
    float k4 = rand->nextF();
    bool enforcePMColor = rand->nextBool();

    return SkNEW_ARGS(GrArithmeticEffect, (k1, k2, k3, k4, enforcePMColor, NULL));
}

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrArithmeticEffect);

bool SkArithmeticMode_scalar::asFragmentProcessor(GrFragmentProcessor** fp,
                                                  GrTexture* background) const {
    if (fp) {
        *fp = GrArithmeticEffect::Create(SkScalarToFloat(fK[0]),
                                         SkScalarToFloat(fK[1]),
                                         SkScalarToFloat(fK[2]),
                                         SkScalarToFloat(fK[3]),
                                         fEnforcePMColor,
                                         background);
    }
    return true;
}

#endif

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkArithmeticMode)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkArithmeticMode_scalar)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END
