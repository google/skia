/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorSpaceXformer_DEFINED
#define SkColorSpaceXformer_DEFINED

#include "SkColor.h"
#include "SkRefCnt.h"
#include "SkTHash.h"

class SkBitmap;
class SkColorFilter;
class SkColorSpace;
class SkColorSpaceXform;
class SkImage;
class SkImageFilter;
class SkPaint;
class SkShader;

class SkColorSpaceXformer : public SkNoncopyable {
public:
    static std::unique_ptr<SkColorSpaceXformer> Make(sk_sp<SkColorSpace> dst);

    ~SkColorSpaceXformer();

    sk_sp<SkImage> apply(const SkImage*);
    sk_sp<SkImage> apply(const SkBitmap&);
    sk_sp<SkColorFilter> apply(const SkColorFilter*);
    sk_sp<SkImageFilter> apply(const SkImageFilter*);
    sk_sp<SkShader>      apply(const SkShader*);
    SkPaint apply(const SkPaint&);
    void apply(SkColor dst[], const SkColor src[], int n);
    SkColor apply(SkColor srgb);

    sk_sp<SkColorSpace> dst() const { return fDst; }

private:
    SkColorSpaceXformer(sk_sp<SkColorSpace> dst, std::unique_ptr<SkColorSpaceXform> fromSRGB);

    template <typename T>
    using Cache = SkTHashMap<sk_sp<T>, sk_sp<T>>;

    template <typename T>
    sk_sp<T> cachedApply(const T*, Cache<T>*, sk_sp<T> (*)(const T*, SkColorSpaceXformer*));

    void purgeCaches();

    class AutoCachePurge;

    sk_sp<SkColorSpace>                fDst;
    std::unique_ptr<SkColorSpaceXform> fFromSRGB;

    size_t fReentryCount; // tracks the number of nested apply() calls for cache purging.

    Cache<SkImage      > fImageCache;
    Cache<SkColorFilter> fColorFilterCache;
    Cache<SkImageFilter> fImageFilterCache;
};

#endif
