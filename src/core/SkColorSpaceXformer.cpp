/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorFilter.h"
#include "SkColorSpacePriv.h"
#include "SkColorSpaceXformer.h"
#include "SkDrawLooper.h"
#include "SkGradientShader.h"
#include "SkImage.h"
#include "SkImage_Base.h"
#include "SkImageFilter.h"
#include "SkImagePriv.h"
#include "SkShaderBase.h"

SkColorSpaceXformer::SkColorSpaceXformer(sk_sp<SkColorSpace> dst)
    : fDst(std::move(dst))
    , fFromSRGBSteps(sk_srgb_singleton(), kUnpremul_SkAlphaType,
                     fDst.get()         , kUnpremul_SkAlphaType)
    , fReentryCount(0) {

    SkRasterPipeline p(&fAlloc);
    p.append(SkRasterPipeline::load_8888, &fFromSRGBSrc);
    p.append(SkRasterPipeline::swap_rb);
    fFromSRGBSteps.apply(&p, kBGRA_8888_SkColorType);
    p.append(SkRasterPipeline::swap_rb);
    p.append(SkRasterPipeline::store_8888, &fFromSRGBDst);
    fFromSRGB = p.compile();
}

SkColorSpaceXformer::~SkColorSpaceXformer() {}

std::unique_ptr<SkColorSpaceXformer> SkColorSpaceXformer::Make(sk_sp<SkColorSpace> dst) {
    return std::unique_ptr<SkColorSpaceXformer>(new SkColorSpaceXformer{std::move(dst)});
}

// So what's up with these caches?
//
// We want to cache transformed objects for a couple of reasons:
//
// 1) to avoid redundant work - the inputs are a DAG, not a tree (e.g. same SkImage drawn multiple
//    times in a SkPicture), so if we blindly recurse we could end up transforming the same objects
//    repeatedly.
//
// 2) to avoid topology changes - we want the output to remain isomorphic with the input -- this is
//    particularly important for image filters (to maintain their original DAG structure in order
//    to not defeat their own/internal caching), but also for avoiding unnecessary cloning
//    (e.g. duplicated SkImages allocated for the example in #1 above).
//
// The caching scope is naturaly bound by the lifetime of the SkColorSpaceXformer object, but
// clients may choose to not discard xformers immediately - in which case, caching indefinitely
// is problematic.  The solution is to limit the cache scope to the top level apply() call
// (i.e. we only keep cached objects alive while transforming).

class SkColorSpaceXformer::AutoCachePurge {
public:
    AutoCachePurge(SkColorSpaceXformer* xformer)
        : fXformer(xformer) {
        fXformer->fReentryCount++;
    }

    ~AutoCachePurge() {
        SkASSERT(fXformer->fReentryCount > 0);
        if (--fXformer->fReentryCount == 0) {
            fXformer->purgeCaches();
        }
    }

private:
    SkColorSpaceXformer* fXformer;
};

template <typename T>
sk_sp<T> SkColorSpaceXformer::cachedApply(const T* src, Cache<T>* cache,
                                          sk_sp<T> (*applyFunc)(const T*, SkColorSpaceXformer*)) {
    if (!src) {
        return nullptr;
    }

    auto key = sk_ref_sp(const_cast<T*>(src));
    if (auto* xformed = cache->find(key)) {
        return sk_ref_sp(xformed->get());
    }

    auto xformed = applyFunc(src, this);
    cache->set(std::move(key), xformed);

    return xformed;
}

void SkColorSpaceXformer::purgeCaches() {
    fImageCache.reset();
    fColorFilterCache.reset();
    fImageFilterCache.reset();
}

sk_sp<SkImage> SkColorSpaceXformer::apply(const SkImage* src) {
    const AutoCachePurge autoPurge(this);
    return this->cachedApply<SkImage>(src, &fImageCache,
        [](const SkImage* img, SkColorSpaceXformer* xformer) {
            return img->makeColorSpace(xformer->fDst);
        });
}

sk_sp<SkImage> SkColorSpaceXformer::apply(const SkBitmap& src) {
    const AutoCachePurge autoPurge(this);
    sk_sp<SkImage> image = SkMakeImageFromRasterBitmap(src, kNever_SkCopyPixelsMode);
    if (!image) {
        return nullptr;
    }

    sk_sp<SkImage> xformed = image->makeColorSpace(fDst);
    // We want to be sure we don't let the kNever_SkCopyPixelsMode image escape this stack frame.
    SkASSERT(xformed != image);
    return xformed;
}

sk_sp<SkColorFilter> SkColorSpaceXformer::apply(const SkColorFilter* colorFilter) {
    const AutoCachePurge autoPurge(this);
    return this->cachedApply<SkColorFilter>(colorFilter, &fColorFilterCache,
        [](const SkColorFilter* f, SkColorSpaceXformer* xformer) {
            return f->makeColorSpace(xformer);
        });
}

sk_sp<SkImageFilter> SkColorSpaceXformer::apply(const SkImageFilter* imageFilter) {
    const AutoCachePurge autoPurge(this);
    return this->cachedApply<SkImageFilter>(imageFilter, &fImageFilterCache,
        [](const SkImageFilter* f, SkColorSpaceXformer* xformer) {
            return f->makeColorSpace(xformer);
        });
}

sk_sp<SkShader> SkColorSpaceXformer::apply(const SkShader* shader) {
    const AutoCachePurge autoPurge(this);
    return as_SB(shader)->makeColorSpace(this);
}

void SkColorSpaceXformer::apply(SkColor* xformed, const SkColor* srgb, int n) {
    fFromSRGBSrc.pixels = const_cast<SkColor*>(srgb);
    fFromSRGBDst.pixels = xformed;
    fFromSRGB(0,0,n,1);
}

SkColor SkColorSpaceXformer::apply(SkColor srgb) {
    SkColor xformed;
    this->apply(&xformed, &srgb, 1);
    return xformed;
}

SkPaint SkColorSpaceXformer::apply(const SkPaint& src) {
    const AutoCachePurge autoPurge(this);

    SkPaint dst = src;

    // All SkColorSpaces have the same black point.
    if (src.getColor() & 0xffffff) {
        dst.setColor(this->apply(src.getColor()));
    }

    if (auto shader = src.getShader()) {
        dst.setShader(this->apply(shader));
    }

    if (auto cf = src.getColorFilter()) {
        dst.setColorFilter(this->apply(cf));
    }

    if (auto looper = src.getDrawLooper()) {
        dst.setDrawLooper(looper->makeColorSpace(this));
    }

    if (auto imageFilter = src.getImageFilter()) {
        dst.setImageFilter(this->apply(imageFilter));
    }

    return dst;
}

SkCanvas::Lattice SkColorSpaceXformer::apply(const SkCanvas::Lattice& lattice,
                                             SkColor* colorBuffer, int count) {
    if (count) {
        this->apply(colorBuffer, lattice.fColors, count);
        return {lattice.fXDivs, lattice.fYDivs, lattice.fRectTypes,
                lattice.fXCount, lattice.fYCount, lattice.fBounds, colorBuffer};
    }

    return lattice;
}
