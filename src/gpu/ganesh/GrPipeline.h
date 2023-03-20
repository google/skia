/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPipeline_DEFINED
#define GrPipeline_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/gpu/ganesh/GrColor.h"
#include "src/gpu/ganesh/GrDstProxyView.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrProcessorSet.h"
#include "src/gpu/ganesh/GrScissorState.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/GrUserStencilSettings.h"
#include "src/gpu/ganesh/GrWindowRectsState.h"
#include "src/gpu/ganesh/effects/GrPorterDuffXferProcessor.h"

class GrAppliedClip;
class GrAppliedHardClip;
struct GrGLSLBuiltinUniformHandles;
class GrGLSLProgramDataManager;
class GrOp;
class GrTextureEffect;

/**
 * This immutable object contains information needed to build a shader program and set API
 * state for a draw. It is used along with a GrGeometryProcessor and a source of geometric
 * data to draw.
 */
class GrPipeline {
public:
    ///////////////////////////////////////////////////////////////////////////
    /// @name Creation

    // Pipeline options that the caller may enable.
    // NOTE: This enum is extended later by GrPipeline::Flags.
    enum class InputFlags : uint8_t {
        kNone = 0,
        /**
         * Cause every pixel to be rasterized that is touched by the triangle anywhere (not just at
         * pixel center). Additionally, if using MSAA, the sample mask will always have 100%
         * coverage.
         * NOTE: The primitive type must be a triangle type.
         */
        kConservativeRaster = (1 << 1),
        /**
         * Draws triangles as outlines.
         */
        kWireframe = (1 << 2),
        /**
         * Modifies the vertex shader so that vertices will be positioned at pixel centers.
         */
        kSnapVerticesToPixelCenters = (1 << 3),  // This value must be last. (See kLastInputFlag.)
    };

    struct InitArgs {
        InputFlags fInputFlags = InputFlags::kNone;
        const GrCaps* fCaps = nullptr;
        GrDstProxyView fDstProxyView;
        skgpu::Swizzle fWriteSwizzle;
    };

    /**
     * Creates a simple pipeline with default settings and no processors. The provided blend mode
     * must be "Porter Duff" (<= kLastCoeffMode). If using GrScissorTest::kEnabled, the caller must
     * specify a scissor rectangle through the DynamicState struct.
     **/
    GrPipeline(GrScissorTest scissor,
               SkBlendMode blend,
               const skgpu::Swizzle& writeSwizzle,
               InputFlags flags = InputFlags::kNone)
            : GrPipeline(scissor,
                         GrPorterDuffXPFactory::MakeNoCoverageXP(blend),
                         writeSwizzle,
                         flags) {}

    GrPipeline(GrScissorTest,
               sk_sp<const GrXferProcessor>,
               const skgpu::Swizzle& writeSwizzle,
               InputFlags = InputFlags::kNone);

    GrPipeline(const InitArgs& args, sk_sp<const GrXferProcessor>, const GrAppliedHardClip&);
    GrPipeline(const InitArgs&, GrProcessorSet&&, GrAppliedClip&&);

    GrPipeline(const GrPipeline&) = delete;
    GrPipeline& operator=(const GrPipeline&) = delete;

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name GrFragmentProcessors

    int numFragmentProcessors() const { return fFragmentProcessors.count(); }
    int numColorFragmentProcessors() const { return fNumColorProcessors; }
    bool isColorFragmentProcessor(int idx) const { return idx < fNumColorProcessors; }
    bool isCoverageFragmentProcessor(int idx) const { return idx >= fNumColorProcessors; }

    bool usesLocalCoords() const {
        // The sample coords for the top level FPs are implicitly the GP's local coords.
        for (const auto& fp : fFragmentProcessors) {
            if (fp->usesSampleCoords()) {
                return true;
            }
        }
        return false;
    }

    void visitTextureEffects(const std::function<void(const GrTextureEffect&)>&) const;

    const GrXferProcessor& getXferProcessor() const {
        if (fXferProcessor) {
            return *fXferProcessor;
        } else {
            // A null xp member means the common src-over case. GrXferProcessor's ref'ing
            // mechanism is not thread safe so we do not hold a ref on this global.
            return GrPorterDuffXPFactory::SimpleSrcOverXP();
        }
    }

    // Helper functions to quickly know if this GrPipeline will access the dst as a texture or an
    // input attachment.
    bool usesDstTexture() const { return this->dstProxyView() && !this->usesDstInputAttachment(); }
    bool usesDstInputAttachment() const {
        return this->dstSampleFlags() & GrDstSampleFlags::kAsInputAttachment;
    }

    /**
     * This returns the GrSurfaceProxyView for the texture used to access the dst color. If the
     * GrXferProcessor does not use the dst color then the proxy on the GrSurfaceProxyView will be
     * nullptr.
     */
    const GrSurfaceProxyView& dstProxyView() const { return fDstProxy.proxyView(); }

    SkIPoint dstTextureOffset() const { return fDstProxy.offset(); }

    GrDstSampleFlags dstSampleFlags() const { return fDstProxy.dstSampleFlags(); }

    /** If this GrXferProcessor uses a texture to access the dst color, returns that texture. */
    GrTexture* peekDstTexture() const {
        if (!this->usesDstTexture()) {
            return nullptr;
        }

        if (GrTextureProxy* dstProxy = this->dstProxyView().asTextureProxy()) {
            return dstProxy->peekTexture();
        }

        return nullptr;
    }

    const GrFragmentProcessor& getFragmentProcessor(int idx) const {
        return *fFragmentProcessors[idx];
    }

    /// @}

    bool isScissorTestEnabled() const {
        return SkToBool(fFlags & Flags::kScissorTestEnabled);
    }

    const GrWindowRectsState& getWindowRectsState() const { return fWindowRectsState; }

    bool usesConservativeRaster() const { return fFlags & InputFlags::kConservativeRaster; }
    bool isWireframe() const { return fFlags & InputFlags::kWireframe; }
    bool snapVerticesToPixelCenters() const {
        return fFlags & InputFlags::kSnapVerticesToPixelCenters;
    }
    bool hasStencilClip() const {
        return SkToBool(fFlags & Flags::kHasStencilClip);
    }
#ifdef SK_DEBUG
    bool allProxiesInstantiated() const {
        for (int i = 0; i < fFragmentProcessors.count(); ++i) {
            if (!fFragmentProcessors[i]->isInstantiated()) {
                return false;
            }
        }
        if (this->dstProxyView().proxy()) {
            return this->dstProxyView().proxy()->isInstantiated();
        }

        return true;
    }
#endif

    GrXferBarrierType xferBarrierType(const GrCaps&) const;

    // Used by Vulkan and Metal to cache their respective pipeline objects
    void genKey(skgpu::KeyBuilder*, const GrCaps&) const;

    const skgpu::Swizzle& writeSwizzle() const { return fWriteSwizzle; }

    void visitProxies(const GrVisitProxyFunc&) const;

    void setDstTextureUniforms(const GrGLSLProgramDataManager& pdm,
                               GrGLSLBuiltinUniformHandles* fBuiltinUniformHandles) const;

private:
    inline static constexpr uint8_t kLastInputFlag =
            (uint8_t)InputFlags::kSnapVerticesToPixelCenters;

    /** This is a continuation of the public "InputFlags" enum. */
    enum class Flags : uint8_t {
        kHasStencilClip = (kLastInputFlag << 1),
        kScissorTestEnabled = (kLastInputFlag << 2),
    };

    GR_DECL_BITFIELD_CLASS_OPS_FRIENDS(Flags);

    friend bool operator&(Flags, InputFlags);

    // A pipeline can contain up to three processors: color, paint coverage, and clip coverage.
    using FragmentProcessorArray =
            skia_private::AutoSTArray<3, std::unique_ptr<const GrFragmentProcessor>>;

    GrDstProxyView fDstProxy;
    GrWindowRectsState fWindowRectsState;
    Flags fFlags;
    sk_sp<const GrXferProcessor> fXferProcessor;
    FragmentProcessorArray fFragmentProcessors;

    // This value is also the index in fFragmentProcessors where coverage processors begin.
    int fNumColorProcessors = 0;

    skgpu::Swizzle fWriteSwizzle;
};

GR_MAKE_BITFIELD_CLASS_OPS(GrPipeline::InputFlags)
GR_MAKE_BITFIELD_CLASS_OPS(GrPipeline::Flags)

inline bool operator&(GrPipeline::Flags flags, GrPipeline::InputFlags inputFlag) {
    return (flags & (GrPipeline::Flags)inputFlag);
}

#endif
