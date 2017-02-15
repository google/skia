/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPictureImageGenerator_DEFINED
#define SkPictureImageGenerator_DEFINED

#include "SkImageGenerator.h"
#include "SkTLazy.h"

class SkPictureImageGenerator : SkImageGenerator {
public:
    static SkImageGenerator* Create(const SkISize&, const SkPicture*, const SkMatrix*,
                                    const SkPaint*, SkImage::BitDepth, sk_sp<SkColorSpace>);

protected:
    bool onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes, SkPMColor ctable[],
                     int* ctableCount) override;

#if SK_SUPPORT_GPU
    GrTexture* onGenerateTexture(GrContext*, const SkImageInfo&, const SkIPoint&) override;
#endif

private:
    SkPictureImageGenerator(const SkImageInfo& info, const SkPicture*, const SkMatrix*,
                            const SkPaint*);

    sk_sp<const SkPicture> fPicture;
    SkMatrix               fMatrix;
    SkTLazy<SkPaint>       fPaint;

    typedef SkImageGenerator INHERITED;
};
#endif  // SkPictureImageGenerator_DEFINED
