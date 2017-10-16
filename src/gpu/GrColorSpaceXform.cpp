/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrColorSpaceXform.h"
#include "SkColorSpace.h"
#include "SkColorSpacePriv.h"
#include "SkColorSpace_Base.h"
#include "SkMatrix44.h"
#include "SkSpinlock.h"

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

    const SkMatrix44* toXYZD50   = as_CSB(src)->toXYZD50();
    const SkMatrix44* fromXYZD50 = as_CSB(dst)->fromXYZD50();
    if (!toXYZD50 || !fromXYZD50) {
        // Unsupported colour spaces -- cannot specify gamut as a matrix
        return nullptr;
    }

    // Determine if a gamut xform is needed
    uint32_t srcHash = as_CSB(src)->toXYZD50Hash();
    uint32_t dstHash = as_CSB(dst)->toXYZD50Hash();
    if (srcHash != dstHash) {
        flags |= kApplyGamutXform_Flag;
    } else {
        SkASSERT(*toXYZD50 == *as_CSB(dst)->toXYZD50() && "Hash collision");
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

// TODO: Apply the transfer functions here, if flags are set
GrColor4f GrColorSpaceXform::apply(const GrColor4f& srcColor) {
    GrColor4f result;
    fGamutXform.mapScalars(srcColor.fRGBA, result.fRGBA);
    // We always operate on unpremul colors, so clamp to [0,1].
    for (int i = 0; i < 4; ++i) {
        result.fRGBA[i] = SkTPin(result.fRGBA[i], 0.0f, 1.0f);
    }
    return result;
}
