/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureDomainEffect_DEFINED
#define GrTextureDomainEffect_DEFINED

#include "GrSingleTextureEffect.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLProgramDataManager.h"

class GrGLProgramBuilder;
class GrGLSLColorSpaceXformHelper;
class GrGLSLShaderBuilder;
class GrInvariantOutput;
class GrGLSLUniformHandler;
struct SkRect;

/**
 * Limits a texture's lookup coordinates to a domain. Samples outside the domain are either clamped
 * the edge of the domain or result in a vec4 of zeros (decal mode). The domain is clipped to
 * normalized texture coords ([0,1]x[0,1] square). Bilinear filtering can cause texels outside the
 * domain to affect the read value unless the caller considers this when calculating the domain.
 */
class GrTextureDomain {
public:
    enum Mode {
        // Ignore the texture domain rectangle.
        kIgnore_Mode,
        // Clamp texture coords to the domain rectangle.
        kClamp_Mode,
        // Treat the area outside the domain rectangle as fully transparent.
        kDecal_Mode,
        // Wrap texture coordinates.  NOTE: filtering may not work as expected because Bilerp will
        // read texels outside of the domain.  We could perform additional texture reads and filter
        // in the shader, but are not currently doing this for performance reasons
        kRepeat_Mode,

        kLastMode = kRepeat_Mode
    };
    static const int kModeCount = kLastMode + 1;

    static const GrTextureDomain& IgnoredDomain() {
        static const GrTextureDomain gDomain((GrTextureProxy*)nullptr,
                                             SkRect::MakeEmpty(), kIgnore_Mode);
        return gDomain;
    }

    /**
     * @param index     Pass a value >= 0 if using multiple texture domains in the same effect.
     *                  It is used to keep inserted variables from causing name collisions.
     */
    GrTextureDomain(GrTexture*, const SkRect& domain, Mode, int index = -1);

    GrTextureDomain(GrTextureProxy*, const SkRect& domain, Mode, int index = -1);

    const SkRect& domain() const { return fDomain; }
    Mode mode() const { return fMode; }

    /* Computes a domain that bounds all the texels in texelRect. Note that with bilerp enabled
       texels neighboring the domain may be read. */
    static const SkRect MakeTexelDomain(const SkIRect& texelRect) {
        return SkRect::Make(texelRect);
    }

    static const SkRect MakeTexelDomainForMode(const SkIRect& texelRect, Mode mode) {
        // For Clamp mode, inset by half a texel.
        SkScalar inset = (mode == kClamp_Mode && !texelRect.isEmpty()) ? SK_ScalarHalf : 0;
        return SkRect::MakeLTRB(texelRect.fLeft + inset, texelRect.fTop + inset,
                                texelRect.fRight - inset, texelRect.fBottom - inset);
    }

    bool operator==(const GrTextureDomain& that) const {
        return fMode == that.fMode && (kIgnore_Mode == fMode || fDomain == that.fDomain);
    }

    /**
     * A GrGLSLFragmentProcessor subclass that corresponds to a GrProcessor subclass that uses
     * GrTextureDomain should include this helper. It generates the texture domain GLSL, produces
     * the part of the effect key that reflects the texture domain code, and performs the uniform
     * uploads necessary for texture domains.
     */
    class GLDomain {
    public:
        GLDomain() {
            for (int i = 0; i < kPrevDomainCount; i++) {
                fPrevDomain[i] = SK_FloatNaN;
            }
            SkDEBUGCODE(fMode = (Mode) -1;)
        }

        /**
         * Call this from GrGLSLFragmentProcessor::emitCode() to sample the texture W.R.T. the
         * domain and mode.
         *
         * @param outcolor  name of vec4 variable to hold the sampled color.
         * @param inCoords  name of vec2 variable containing the coords to be used with the domain.
         *                  It is assumed that this is a variable and not an expression.
         * @param inModulateColor   if non-nullptr the sampled color will be modulated with this
         *                          expression before being written to outColor.
         */
        void sampleTexture(GrGLSLShaderBuilder* builder,
                           GrGLSLUniformHandler* uniformHandler,
                           const GrShaderCaps* shaderCaps,
                           const GrTextureDomain& textureDomain,
                           const char* outColor,
                           const SkString& inCoords,
                           GrGLSLFragmentProcessor::SamplerHandle sampler,
                           const char* inModulateColor = nullptr,
                           GrGLSLColorSpaceXformHelper* colorXformHelper = nullptr);

        /**
         * Call this from GrGLSLFragmentProcessor::setData() to upload uniforms necessary for the
         * texture domain. The rectangle is automatically adjusted to account for the texture's
         * origin.
         */
        void setData(const GrGLSLProgramDataManager& pdman, const GrTextureDomain& textureDomain,
                     GrTexture* texure);

        enum {
            kDomainKeyBits = 2, // See DomainKey().
        };

        /**
         * GrGLSLFragmentProcessor::GenKey() must call this and include the returned value in it's
         * computed key. The returned will be limited to the lower kDomainKeyBits bits.
         */
        static uint32_t DomainKey(const GrTextureDomain& domain) {
            GR_STATIC_ASSERT(kModeCount <= (1 << kDomainKeyBits));
            return domain.mode();
        }

    private:
        static const int kPrevDomainCount = 4;
        SkDEBUGCODE(Mode                        fMode;)
        GrGLSLProgramDataManager::UniformHandle fDomainUni;
        SkString                                fDomainName;
        float                                   fPrevDomain[kPrevDomainCount];
    };

protected:
    Mode    fMode;
    SkRect  fDomain;
    int     fIndex;
};

/**
 * A basic texture effect that uses GrTextureDomain.
 */
class GrTextureDomainEffect : public GrSingleTextureEffect {

public:
    static sk_sp<GrFragmentProcessor> Make(GrResourceProvider*,
                                           sk_sp<GrTextureProxy>,
                                           sk_sp<GrColorSpaceXform>,
                                           const SkMatrix&,
                                           const SkRect& domain,
                                           GrTextureDomain::Mode,
                                           GrSamplerParams::FilterMode filterMode);

    const char* name() const override { return "TextureDomain"; }

    SkString dumpInfo() const override {
        SkString str;
        str.appendf("Domain: [L: %.2f, T: %.2f, R: %.2f, B: %.2f]",
                    fTextureDomain.domain().fLeft, fTextureDomain.domain().fTop,
                    fTextureDomain.domain().fRight, fTextureDomain.domain().fBottom);
        str.append(INHERITED::dumpInfo());
        return str;
    }

private:
    GrTextureDomain fTextureDomain;

    GrTextureDomainEffect(GrResourceProvider*,
                          sk_sp<GrTextureProxy>,
                          sk_sp<GrColorSpaceXform>,
                          const SkMatrix&,
                          const SkRect& domain,
                          GrTextureDomain::Mode,
                          GrSamplerParams::FilterMode);

    static OptimizationFlags OptFlags(GrPixelConfig config, GrTextureDomain::Mode mode);

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    typedef GrSingleTextureEffect INHERITED;
};

class GrDeviceSpaceTextureDecalFragmentProcessor : public GrFragmentProcessor {
public:
    static sk_sp<GrFragmentProcessor> Make(GrResourceProvider*, sk_sp<GrTextureProxy>,
                                           const SkIRect& subset,
                                           const SkIPoint& deviceSpaceOffset);

    const char* name() const override { return "GrDeviceSpaceTextureDecalFragmentProcessor"; }

    SkString dumpInfo() const override {
        SkString str;
        str.appendf("Domain: [L: %.2f, T: %.2f, R: %.2f, B: %.2f] Offset: [%d %d]",
                    fTextureDomain.domain().fLeft, fTextureDomain.domain().fTop,
                    fTextureDomain.domain().fRight, fTextureDomain.domain().fBottom,
                    fDeviceSpaceOffset.fX, fDeviceSpaceOffset.fY);
        str.append(INHERITED::dumpInfo());
        return str;
    }

private:
    TextureSampler fTextureSampler;
    GrTextureDomain fTextureDomain;
    SkIPoint fDeviceSpaceOffset;

    GrDeviceSpaceTextureDecalFragmentProcessor(GrResourceProvider*, sk_sp<GrTextureProxy>,
                                               const SkIRect&, const SkIPoint&);

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    // Since we always use decal mode, there is no need for key data.
    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}

    bool onIsEqual(const GrFragmentProcessor& fp) const override;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    typedef GrFragmentProcessor INHERITED;
};
#endif
