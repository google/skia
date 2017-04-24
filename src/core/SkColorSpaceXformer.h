/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorSpaceXformer_DEFINED
#define SkColorSpaceXformer_DEFINED

#include "SkColorSpaceXform.h"
#include "SkImage.h"
#include "SkShader.h"

class SkColorSpaceXformer : public SkNoncopyable {
public:
    static std::unique_ptr<SkColorSpaceXformer> Make(sk_sp<SkColorSpace> dst);

    sk_sp<SkImage> apply(const SkImage* src);
    sk_sp<SkImage> apply(const SkBitmap& bitmap);
    sk_sp<SkColorFilter> apply(const SkColorFilter* filter);
    void apply(SkColor dst[], const SkColor src[], int n);
    SkColor apply(SkColor srgb);
    void apply(SkPaint* dst, const SkPaint& src);

    sk_sp<SkColorSpace> dst() const { return fDst; }

private:
    // Not safe to call recursively.  Only should be called from SkColorSpaceXformCanvas.
    const SkPaint* apply(const SkPaint* src);
    const SkPaint& apply(const SkPaint& src);
    friend class SkColorSpaceXformCanvas;

    // Returns true and initializes |dst| with the xformed paint if a xform is required.
    // Returns false and does not initialize |dst| otherwise.
    bool applyHelper(SkPaint* dst, const SkPaint& src);

    SkColorSpaceXformer() {}

    sk_sp<SkColorSpace>                fDst;
    std::unique_ptr<SkColorSpaceXform> fFromSRGB;
    SkPaint                            fDstPaint;
};

#endif
