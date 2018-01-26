/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrColorSpaceXform.h"
#include "SkColorSpace.h"
#include "SkColorSpacePriv.h"
#include "SkMatrix44.h"
#include "SkSpinlock.h"
#include "glsl/GrGLSLColorSpaceXformHelper.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"

class GrColorSpaceXformCache {
public:
    using NewValueFn = std::function<sk_sp<GrColorSpaceXform>(void)>;

    GrColorSpaceXformCache() : fSequence(0) {}

    sk_sp<GrColorSpaceXform> findOrAdd(uint64_t key, NewValueFn newValue) {
        int oldest = 0;
        for (int i = 0; i < kEntryCount; ++i) {
            if (fEntries[i].fKey == key) {
                fEntries[i].fLastUse = fSequence++;
                return fEntries[i].fXform;
            }
            if (fEntries[i].fLastUse < fEntries[oldest].fLastUse) {
                oldest = i;
            }
        }
        fEntries[oldest].fKey = key;
        fEntries[oldest].fXform = newValue();
        fEntries[oldest].fLastUse = fSequence++;
        return fEntries[oldest].fXform;
    }

private:
    enum { kEntryCount = 32 };

    struct Entry {
        // The default Entry is "valid". Any 64-bit key that is the same 32-bit value repeated
        // implies no xform is necessary, so nullptr should be returned. This particular case should
        // never happen, but by initializing all entries with this data, we can avoid special cases
        // for the array not yet being full.
        Entry() : fKey(0), fXform(nullptr), fLastUse(0) {}

        uint64_t fKey;
        sk_sp<GrColorSpaceXform> fXform;
        uint64_t fLastUse;
    };

    Entry fEntries[kEntryCount];
    uint64_t fSequence;
};

GrColorSpaceXform::GrColorSpaceXform(const SkColorSpaceTransferFn& srcTransferFn,
                                     const SkMatrix44& gamutXform, uint32_t flags)
    : fSrcTransferFn(srcTransferFn), fGamutXform(gamutXform), fFlags(flags) {}

static SkSpinlock gColorSpaceXformCacheSpinlock;

sk_sp<GrColorSpaceXform> GrColorSpaceXform::Make(const SkColorSpace* src,
                                                 GrPixelConfig srcConfig,
                                                 const SkColorSpace* dst) {
    if (!dst) {
        // No transformation is performed in legacy mode
        return nullptr;
    }

    // Treat null sources as sRGB
    if (!src) {
        if (GrPixelConfigIsFloatingPoint(srcConfig)) {
            src = SkColorSpace::MakeSRGBLinear().get();
        } else {
            src = SkColorSpace::MakeSRGB().get();
        }
    }

    uint32_t flags = 0;
    SkColorSpaceTransferFn srcTransferFn;

    // kUnknown_GrPixelConfig is a sentinel that means we don't care about transfer functions,
    // just the gamut xform.
    if (kUnknown_GrPixelConfig != srcConfig) {
        // Determine if src transfer function is needed, based on src config and color space
        if (GrPixelConfigIsSRGB(srcConfig)) {
            // Source texture is sRGB, will be converted to linear when we sample
            if (src->gammaCloseToSRGB()) {
                // Hardware linearize does the right thing
            } else if (src->gammaIsLinear()) {
                // Oops, need to undo the (extra) linearize
                flags |= kApplyInverseSRGB_Flag;
            } else if (src->isNumericalTransferFn(&srcTransferFn)) {
                // Need to undo the (extra) linearize, then apply the correct transfer function
                flags |= (kApplyInverseSRGB_Flag | kApplyTransferFn_Flag);
            } else {
                // We don't (yet) support more complex transfer functions
                return nullptr;
            }
        } else {
            // Source texture is some non-sRGB format, we consider it linearly encoded
            if (src->gammaIsLinear()) {
                // Linear sampling does the right thing
            } else if (src->isNumericalTransferFn(&srcTransferFn)) {
                // Need to manually apply some transfer function (including sRGB)
                flags |= kApplyTransferFn_Flag;
            } else {
                // We don't (yet) support more complex transfer functions
                return nullptr;
            }
        }
    }
    if (src == dst && (0 == flags)) {
        // Quick equality check - no conversion (or transfer function) needed in this case
        return nullptr;
    }

    const SkMatrix44* toXYZD50   = src->toXYZD50();
    const SkMatrix44* fromXYZD50 = dst->fromXYZD50();
    if (!toXYZD50 || !fromXYZD50) {
        // Unsupported colour spaces -- cannot specify gamut as a matrix
        return nullptr;
    }

    // Determine if a gamut xform is needed
    uint32_t srcHash = src->toXYZD50Hash();
    uint32_t dstHash = dst->toXYZD50Hash();
    if (srcHash != dstHash) {
        flags |= kApplyGamutXform_Flag;
    } else {
        SkASSERT(*toXYZD50 == *dst->toXYZD50() && "Hash collision");
    }

    if (0 == flags) {
        // Identical gamut and no transfer function - no conversion needed in this case
        return nullptr;
    }

    auto makeXform = [srcTransferFn, fromXYZD50, toXYZD50, flags]() {
        SkMatrix44 srcToDst(SkMatrix44::kUninitialized_Constructor);
        if (SkToBool(flags & kApplyGamutXform_Flag)) {
            srcToDst.setConcat(*fromXYZD50, *toXYZD50);
        } else {
            srcToDst.setIdentity();
        }
        return sk_make_sp<GrColorSpaceXform>(srcTransferFn, srcToDst, flags);
    };

    // For now, we only cache pure gamut xforms (no transfer functions)
    // TODO: Fold a hash of the transfer function into the cache key
    if ((kApplyGamutXform_Flag == flags) && gColorSpaceXformCacheSpinlock.tryAcquire()) {
        static GrColorSpaceXformCache* gCache;
        if (nullptr == gCache) {
            gCache = new GrColorSpaceXformCache();
        }

        uint64_t key = static_cast<uint64_t>(srcHash) << 32 | static_cast<uint64_t>(dstHash);
        sk_sp<GrColorSpaceXform> xform = gCache->findOrAdd(key, makeXform);
        gColorSpaceXformCacheSpinlock.release();
        return xform;
    } else {
        // If our xform has non-gamut components, or we can't get the spin lock, just build it
        return makeXform();
    }
}

bool GrColorSpaceXform::Equals(const GrColorSpaceXform* a, const GrColorSpaceXform* b) {
    if (a == b) {
        return true;
    }

    if (!a || !b || a->fFlags != b->fFlags) {
        return false;
    }

    if (SkToBool(a->fFlags & kApplyTransferFn_Flag) &&
        0 != memcmp(&a->fSrcTransferFn, &b->fSrcTransferFn, sizeof(SkColorSpaceTransferFn))) {
        return false;
    }

    if (SkToBool(a->fFlags && kApplyGamutXform_Flag) && a->fGamutXform != b->fGamutXform) {
        return false;
    }

    return true;
}

GrColor4f GrColorSpaceXform::unclampedXform(const GrColor4f& srcColor) {
    // This transform step should only happen with textures (not CPU xform of individual values)
    SkASSERT(!SkToBool(fFlags & kApplyInverseSRGB_Flag));

    GrColor4f result = srcColor;
    if (fFlags & kApplyTransferFn_Flag) {
        // Only transform RGB (not alpha)
        for (int i = 0; i < 3; ++i) {
            result.fRGBA[i] = fSrcTransferFn(result.fRGBA[i]);
        }
    }
    if (fFlags & kApplyGamutXform_Flag) {
        fGamutXform.mapScalars(result.fRGBA, result.fRGBA);
    }
    return result;
}

GrColor4f GrColorSpaceXform::clampedXform(const GrColor4f& srcColor) {
    GrColor4f result = this->unclampedXform(srcColor);
    for (int i = 0; i < 4; ++i) {
        // We always operate on unpremul colors, so clamp to [0,1].
        result.fRGBA[i] = SkTPin(result.fRGBA[i], 0.0f, 1.0f);
    }
    return result;
}

//////////////////////////////////////////////////////////////////////////////

class GrGLColorSpaceXformEffect : public GrGLSLFragmentProcessor {
public:
    void emitCode(EmitArgs& args) override {
        const GrColorSpaceXformEffect& csxe = args.fFp.cast<GrColorSpaceXformEffect>();
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

        fColorSpaceHelper.emitCode(uniformHandler, csxe.colorXform());

        SkString childColor("src_color");
        this->emitChild(0, &childColor, args);

        SkString xformedColor;
        fragBuilder->appendColorGamutXform(&xformedColor, childColor.c_str(), &fColorSpaceHelper);
        fragBuilder->codeAppendf("%s = %s * %s;", args.fOutputColor, xformedColor.c_str(),
                                 args.fInputColor);
    }

private:
    void onSetData(const GrGLSLProgramDataManager& pdman,
                   const GrFragmentProcessor& processor) override {
        const GrColorSpaceXformEffect& csxe = processor.cast<GrColorSpaceXformEffect>();
        if (fColorSpaceHelper.isValid()) {
            fColorSpaceHelper.setData(pdman, csxe.colorXform());
        }
    }

    GrGLSLColorSpaceXformHelper fColorSpaceHelper;

    typedef GrGLSLFragmentProcessor INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

GrColorSpaceXformEffect::GrColorSpaceXformEffect(std::unique_ptr<GrFragmentProcessor> child,
                                                 sk_sp<GrColorSpaceXform> colorXform)
        : INHERITED(kGrColorSpaceXformEffect_ClassID, OptFlags(child.get()))
        , fColorXform(std::move(colorXform)) {
    this->registerChildProcessor(std::move(child));
}

std::unique_ptr<GrFragmentProcessor> GrColorSpaceXformEffect::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(
            new GrColorSpaceXformEffect(this->childProcessor(0).clone(), fColorXform));
}

bool GrColorSpaceXformEffect::onIsEqual(const GrFragmentProcessor& s) const {
    const GrColorSpaceXformEffect& other = s.cast<GrColorSpaceXformEffect>();
    return GrColorSpaceXform::Equals(fColorXform.get(), other.fColorXform.get());
}

void GrColorSpaceXformEffect::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                                    GrProcessorKeyBuilder* b) const {
    b->add32(GrColorSpaceXform::XformKey(fColorXform.get()));
}

GrGLSLFragmentProcessor* GrColorSpaceXformEffect::onCreateGLSLInstance() const {
    return new GrGLColorSpaceXformEffect();
}

GrFragmentProcessor::OptimizationFlags GrColorSpaceXformEffect::OptFlags(
        const GrFragmentProcessor* child) {
    // TODO: Implement constant output for constant input
    OptimizationFlags flags = kNone_OptimizationFlags;
    if (child->compatibleWithCoverageAsAlpha()) {
        flags |= kCompatibleWithCoverageAsAlpha_OptimizationFlag;
    }
    if (child->preservesOpaqueInput()) {
        flags |= kPreservesOpaqueInput_OptimizationFlag;
    }
    return flags;
}

std::unique_ptr<GrFragmentProcessor> GrColorSpaceXformEffect::Make(
        std::unique_ptr<GrFragmentProcessor> child,
        const SkColorSpace* src,
        GrPixelConfig srcConfig,
        const SkColorSpace* dst) {
    if (!child) {
        return nullptr;
    }

    auto colorXform = GrColorSpaceXform::Make(src, srcConfig, dst);
    if (colorXform) {
        return std::unique_ptr<GrFragmentProcessor>(
                new GrColorSpaceXformEffect(std::move(child), std::move(colorXform)));
    } else {
        return child;
    }
}
