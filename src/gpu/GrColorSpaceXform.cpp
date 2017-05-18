/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrColorSpaceXform.h"
#include "SkColorSpace.h"
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

GrColorSpaceXform::GrColorSpaceXform(const SkMatrix44& srcToDst)
    : fSrcToDst(srcToDst) {}

static SkSpinlock gColorSpaceXformCacheSpinlock;

sk_sp<GrColorSpaceXform> GrColorSpaceXform::Make(const SkColorSpace* src, const SkColorSpace* dst) {
    if (!src || !dst) {
        // Invalid
        return nullptr;
    }

    if (src == dst) {
        // Quick equality check - no conversion needed in this case
        return nullptr;
    }

    const SkMatrix44* toXYZD50   = as_CSB(src)->toXYZD50();
    const SkMatrix44* fromXYZD50 = as_CSB(dst)->fromXYZD50();
    if (!toXYZD50 || !fromXYZD50) {
        // unsupported colour spaces -- cannot specify gamut as a matrix
        return nullptr;
    }

    uint32_t srcHash = as_CSB(src)->toXYZD50Hash();
    uint32_t dstHash = as_CSB(dst)->toXYZD50Hash();
    if (srcHash == dstHash) {
        // Identical gamut - no conversion needed in this case
        SkASSERT(*toXYZD50 == *as_CSB(dst)->toXYZD50() && "Hash collision");
        return nullptr;
    }

    auto deferredResult = [fromXYZD50, toXYZD50]() {
        SkMatrix44 srcToDst(SkMatrix44::kUninitialized_Constructor);
        srcToDst.setConcat(*fromXYZD50, *toXYZD50);
        return sk_make_sp<GrColorSpaceXform>(srcToDst);
    };

    if (gColorSpaceXformCacheSpinlock.tryAcquire()) {
        static GrColorSpaceXformCache* gCache;
        if (nullptr == gCache) {
            gCache = new GrColorSpaceXformCache();
        }

        uint64_t key = static_cast<uint64_t>(srcHash) << 32 | static_cast<uint64_t>(dstHash);
        sk_sp<GrColorSpaceXform> xform = gCache->findOrAdd(key, deferredResult);
        gColorSpaceXformCacheSpinlock.release();
        return xform;
    } else {
        // Rather than wait for the spin lock, just bypass the cache
        return deferredResult();
    }
}

bool GrColorSpaceXform::Equals(const GrColorSpaceXform* a, const GrColorSpaceXform* b) {
    if (a == b) {
        return true;
    }

    if (!a || !b) {
        return false;
    }

    return a->fSrcToDst == b->fSrcToDst;
}

GrColor4f GrColorSpaceXform::apply(const GrColor4f& srcColor) {
    GrColor4f result;
    fSrcToDst.mapScalars(srcColor.fRGBA, result.fRGBA);
    // We always operate on unpremul colors, so clamp to [0,1].
    for (int i = 0; i < 4; ++i) {
        result.fRGBA[i] = SkTPin(result.fRGBA[i], 0.0f, 1.0f);
    }
    return result;
}
