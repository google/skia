/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageShader_DEFINED
#define SkImageShader_DEFINED

#include "SkImage.h"
#include "SkShader.h"

class SkImageShader : public SkShader {
public:
    static sk_sp<SkShader> Make(const SkImage*, TileMode tx, TileMode ty,
                                const SkMatrix* localMatrix);

    bool isOpaque() const override;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkImageShader)

#if SK_SUPPORT_GPU
    const GrFragmentProcessor* asFragmentProcessor(GrContext*, const SkMatrix& viewM,
                                                   const SkMatrix*, SkFilterQuality) const override;
#endif

protected:
    void flatten(SkWriteBuffer&) const override;
    size_t onContextSize(const ContextRec&) const override;
    Context* onCreateContext(const ContextRec&, void* storage) const override;

    SkAutoTUnref<const SkImage> fImage;
    const TileMode              fTileModeX;
    const TileMode              fTileModeY;

private:
    SkImageShader(const SkImage*, TileMode tx, TileMode ty, const SkMatrix* localMatrix);

    typedef SkShader INHERITED;
};

#endif
