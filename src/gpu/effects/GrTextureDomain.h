/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureDomainEffect_DEFINED
#define GrTextureDomainEffect_DEFINED

#include "GrCoordTransform.h"
#include "GrFragmentProcessor.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLProgramDataManager.h"

class GrGLProgramBuilder;
class GrGLSLShaderBuilder;
class GrInvariantOutput;
class GrGLSLUniformHandler;
struct SkRect;

/**
 * Limits a texture's lookup coordinates to a domain. Samples outside the domain are either clamped
 * the edge of the domain or result in a half4 of zeros (decal mode). The domain is clipped to
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
                                             SkRect::MakeEmpty(), kIgnore_Mode, kIgnore_Mode);
        return gDomain;
    }

    /**
     * @param index     Pass a value >= 0 if using multiple texture domains in the same effect.
     *                  It is used to keep inserted variables from causing name collisions.
     */
    GrTextureDomain(GrTextureProxy*, const SkRect& domain, Mode modeX, Mode modeY, int index = -1);

    GrTextureDomain(const GrTextureDomain&) = default;

    const SkRect& domain() const { return fDomain; }
    Mode modeX() const { return fModeX; }
    Mode modeY() const { return fModeY; }

    /*
     * Computes a domain that bounds all the texels in texelRect, possibly insetting by half a pixel
     * depending on the mode. The mode is used for both axes.
     */
    static const SkRect MakeTexelDomain(const SkIRect& texelRect, Mode mode) {
        return MakeTexelDomain(texelRect, mode, mode);
    }

    static const SkRect MakeTexelDomain(const SkIRect& texelRect, Mode modeX, Mode modeY) {
        // For Clamp and decal modes, inset by half a texel
        SkScalar insetX = ((modeX == kClamp_Mode || modeX == kDecal_Mode) && texelRect.width() > 0)
                ? SK_ScalarHalf : 0;
        SkScalar insetY = ((modeY == kClamp_Mode || modeY == kDecal_Mode) && texelRect.height() > 0)
                ? SK_ScalarHalf : 0;
        return SkRect::MakeLTRB(texelRect.fLeft + insetX, texelRect.fTop + insetY,
                                texelRect.fRight - insetX, texelRect.fBottom - insetY);
    }

    // Convenience to determine if any axis of a texture uses an explicit decal mode or the hardware
    // clamp to border decal mode.
    static bool IsDecalSampled(GrSamplerState::WrapMode wrapX, GrSamplerState::WrapMode wrapY,
                               Mode modeX, Mode modeY) {
        return wrapX == GrSamplerState::WrapMode::kClampToBorder ||
               wrapY == GrSamplerState::WrapMode::kClampToBorder ||
               modeX == kDecal_Mode ||
               modeY == kDecal_Mode;
    }

    static bool IsDecalSampled(const GrSamplerState::WrapMode wraps[2], Mode modeX, Mode modeY) {
        return IsDecalSampled(wraps[0], wraps[1], modeX, modeY);
    }

    static bool IsDecalSampled(const GrSamplerState& sampler, Mode modeX, Mode modeY) {
        return IsDecalSampled(sampler.wrapModeX(), sampler.wrapModeY(), modeX, modeY);
    }

    bool operator==(const GrTextureDomain& that) const {
        return fModeX == that.fModeX && fModeY == that.fModeY &&
               (kIgnore_Mode == fModeX || (fDomain.fLeft == that.fDomain.fLeft &&
                                           fDomain.fRight == that.fDomain.fRight)) &&
               (kIgnore_Mode == fModeY || (fDomain.fTop == that.fDomain.fTop &&
                                           fDomain.fBottom == that.fDomain.fBottom));
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
        }

        /**
         * Call this from GrGLSLFragmentProcessor::emitCode() to sample the texture W.R.T. the
         * domain and mode.
         *
         * @param outcolor  name of half4 variable to hold the sampled color.
         * @param inCoords  name of float2 variable containing the coords to be used with the domain.
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
                           const char* inModulateColor = nullptr);

        /**
         * Call this from GrGLSLFragmentProcessor::setData() to upload uniforms necessary for the
         * texture domain. The rectangle is automatically adjusted to account for the texture's
         * origin.
         */
        void setData(const GrGLSLProgramDataManager&, const GrTextureDomain&, GrTextureProxy*,
                     const GrSamplerState& sampler);

        enum {
            kModeBits = 2, // See DomainKey().
            kDomainKeyBits = 4
        };

        /**
         * GrGLSLFragmentProcessor::GenKey() must call this and include the returned value in it's
         * computed key. The returned will be limited to the lower kDomainKeyBits bits.
         */
        static uint32_t DomainKey(const GrTextureDomain& domain) {
            GR_STATIC_ASSERT(kModeCount <= (1 << kModeBits));
            return domain.modeX() | (domain.modeY() << kModeBits);
        }

    private:
        static const int kPrevDomainCount = 4;
        SkDEBUGCODE(Mode                        fModeX;)
        SkDEBUGCODE(Mode                        fModeY;)
        SkDEBUGCODE(bool                        fHasMode = false;)
        GrGLSLProgramDataManager::UniformHandle fDomainUni;
        SkString                                fDomainName;

        // Only initialized if the domain has at least one decal axis
        GrGLSLProgramDataManager::UniformHandle fDecalUni;
        SkString                                fDecalName;

        float                                   fPrevDomain[kPrevDomainCount];
    };

protected:
    Mode    fModeX;
    Mode    fModeY;
    SkRect  fDomain;
    int     fIndex;
};

/**
 * A basic texture effect that uses GrTextureDomain.
 */
class GrTextureDomainEffect : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make(sk_sp<GrTextureProxy>,
                                                     const SkMatrix&,
                                                     const SkRect& domain,
                                                     GrTextureDomain::Mode mode,
                                                     GrSamplerState::Filter filterMode);

    static std::unique_ptr<GrFragmentProcessor> Make(sk_sp<GrTextureProxy>,
                                                     const SkMatrix&,
                                                     const SkRect& domain,
                                                     GrTextureDomain::Mode modeX,
                                                     GrTextureDomain::Mode modeY,
                                                     const GrSamplerState& sampler);

    const char* name() const override { return "TextureDomain"; }

    std::unique_ptr<GrFragmentProcessor> clone() const override {
        return std::unique_ptr<GrFragmentProcessor>(new GrTextureDomainEffect(*this));
    }

#ifdef SK_DEBUG
    SkString dumpInfo() const override {
        SkString str;
        str.appendf("Domain: [L: %.2f, T: %.2f, R: %.2f, B: %.2f]",
                    fTextureDomain.domain().fLeft, fTextureDomain.domain().fTop,
                    fTextureDomain.domain().fRight, fTextureDomain.domain().fBottom);
        str.append(INHERITED::dumpInfo());
        return str;
    }
#endif

private:
    GrCoordTransform fCoordTransform;
    GrTextureDomain fTextureDomain;
    TextureSampler fTextureSampler;

    GrTextureDomainEffect(sk_sp<GrTextureProxy>,
                          const SkMatrix&,
                          const SkRect& domain,
                          GrTextureDomain::Mode modeX,
                          GrTextureDomain::Mode modeY,
                          const GrSamplerState&);

    explicit GrTextureDomainEffect(const GrTextureDomainEffect&);

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    const TextureSampler& onTextureSampler(int) const override { return fTextureSampler; }

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    typedef GrFragmentProcessor INHERITED;
};

class GrDeviceSpaceTextureDecalFragmentProcessor : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make(sk_sp<GrTextureProxy>,
                                                     const SkIRect& subset,
                                                     const SkIPoint& deviceSpaceOffset);

    const char* name() const override { return "GrDeviceSpaceTextureDecalFragmentProcessor"; }

#ifdef SK_DEBUG
    SkString dumpInfo() const override {
        SkString str;
        str.appendf("Domain: [L: %.2f, T: %.2f, R: %.2f, B: %.2f] Offset: [%d %d]",
                    fTextureDomain.domain().fLeft, fTextureDomain.domain().fTop,
                    fTextureDomain.domain().fRight, fTextureDomain.domain().fBottom,
                    fDeviceSpaceOffset.fX, fDeviceSpaceOffset.fY);
        str.append(INHERITED::dumpInfo());
        return str;
    }
#endif

    std::unique_ptr<GrFragmentProcessor> clone() const override;

private:
    TextureSampler fTextureSampler;
    GrTextureDomain fTextureDomain;
    SkIPoint fDeviceSpaceOffset;

    GrDeviceSpaceTextureDecalFragmentProcessor(sk_sp<GrTextureProxy>,
                                               const SkIRect&, const SkIPoint&);
    GrDeviceSpaceTextureDecalFragmentProcessor(const GrDeviceSpaceTextureDecalFragmentProcessor&);

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    // Since we always use decal mode, there is no need for key data.
    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}

    bool onIsEqual(const GrFragmentProcessor& fp) const override;

    const TextureSampler& onTextureSampler(int) const override { return fTextureSampler; }

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    typedef GrFragmentProcessor INHERITED;
};
#endif
