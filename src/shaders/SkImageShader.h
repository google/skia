/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageShader_DEFINED
#define SkImageShader_DEFINED

#include "SkBitmapProcShader.h"
#include "SkColorSpaceXformer.h"
#include "SkImage.h"
#include "SkShaderBase.h"

class SkImageShader : public SkShaderBase {
public:
    static sk_sp<SkShader> Make(sk_sp<SkImage>, TileMode tx, TileMode ty,
                                const SkMatrix* localMatrix);

    bool isOpaque() const override;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkImageShader)

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs&) const override;
#endif

    SkImageShader(sk_sp<SkImage>, TileMode tx, TileMode ty, const SkMatrix* localMatrix);

    static bool IsRasterPipelineOnly(const SkMatrix& ctm, SkColorType, SkAlphaType,
                                     SkShader::TileMode tx, SkShader::TileMode ty,
                                     const SkMatrix& localM);

protected:
    void flatten(SkWriteBuffer&) const override;
    Context* onMakeContext(const ContextRec&, SkArenaAlloc* storage) const override;
#ifdef SK_SUPPORT_LEGACY_SHADER_ISABITMAP
    bool onIsABitmap(SkBitmap*, SkMatrix*, TileMode*) const override;
#endif
    SkImage* onIsAImage(SkMatrix*, TileMode*) const override;

    bool onIsRasterPipelineOnly(const SkMatrix& ctm) const override;

    bool onAppendStages(const StageRec&) const override;

    sk_sp<SkShader> onMakeColorSpace(SkColorSpaceXformer* xformer) const override {
        return xformer->apply(fImage.get())->makeShader(fTileModeX, fTileModeY,
                                                        &this->getLocalMatrix());
    }

    sk_sp<SkImage>  fImage;
    const TileMode  fTileModeX;
    const TileMode  fTileModeY;

private:
    friend class SkShaderBase;

    typedef SkShaderBase INHERITED;
};

#endif
