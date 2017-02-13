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
#include "SkBitmapProcShader.h"

class SkImageShader : public SkShader {
public:
    static sk_sp<SkShader> Make(sk_sp<SkImage>, TileMode tx, TileMode ty,
                                const SkMatrix* localMatrix);

    bool isOpaque() const override;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkImageShader)

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(const AsFPArgs&) const override;
#endif

    SkImageShader(sk_sp<SkImage>, TileMode tx, TileMode ty, const SkMatrix* localMatrix);

protected:
    void flatten(SkWriteBuffer&) const override;
    Context* onMakeContext(const ContextRec&, SkArenaAlloc* storage) const override;
#ifdef SK_SUPPORT_LEGACY_SHADER_ISABITMAP
    bool onIsABitmap(SkBitmap*, SkMatrix*, TileMode*) const override;
#endif
    SkImage* onIsAImage(SkMatrix*, TileMode*) const override;

    bool onAppendStages(SkRasterPipeline*, SkColorSpace*, SkArenaAlloc*,
                        const SkMatrix& ctm, const SkPaint&, const SkMatrix*) const override;

    sk_sp<SkImage>  fImage;
    const TileMode  fTileModeX;
    const TileMode  fTileModeY;

private:
    friend class SkShader;

    typedef SkShader INHERITED;
};

#endif
