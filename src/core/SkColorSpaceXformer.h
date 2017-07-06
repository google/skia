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
    SkColorSpaceXformer() = default;

    sk_sp<SkColorSpace>                fDst;
    std::unique_ptr<SkColorSpaceXform> fFromSRGB;

    SkTHashMap<uint32_t, sk_sp<SkImageFilter>> fFilterCache;
};

#endif
