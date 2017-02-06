/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPictureShader_DEFINED
#define SkPictureShader_DEFINED

#include "SkShader.h"

class SkArenaAlloc;
class SkBitmap;
class SkPicture;

/*
 * An SkPictureShader can be used to draw SkPicture-based patterns.
 *
 * The SkPicture is first rendered into a tile, which is then used to shade the area according
 * to specified tiling rules.
 */
class SkPictureShader : public SkShader {
public:
    static sk_sp<SkShader> Make(sk_sp<SkPicture>, TileMode, TileMode, const SkMatrix*,
                                const SkRect*);

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkPictureShader)

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(const AsFPArgs&) const override;
#endif

protected:
    SkPictureShader(SkReadBuffer&);
    void flatten(SkWriteBuffer&) const override;
    bool onAppendStages(SkRasterPipeline*, SkColorSpace*, SkArenaAlloc*,
                        const SkMatrix&, const SkPaint&, const SkMatrix*) const override;
    Context* onMakeContext(const ContextRec&, SkArenaAlloc*) const override;

private:
    SkPictureShader(sk_sp<SkPicture>, TileMode, TileMode, const SkMatrix*, const SkRect*);

    sk_sp<SkShader> refBitmapShader(const SkMatrix&, const SkMatrix* localMatrix,
                                    SkColorSpace* dstColorSpace,
                                    const int maxTextureSize = 0) const;

    sk_sp<SkPicture>    fPicture;
    SkRect              fTile;
    TileMode            fTmx, fTmy;

    class PictureShaderContext : public SkShader::Context {
    public:
        PictureShaderContext(
            const SkPictureShader&, const ContextRec&, sk_sp<SkShader> bitmapShader, SkArenaAlloc*);

        uint32_t getFlags() const override;

        ShadeProc asAShadeProc(void** ctx) override;
        void shadeSpan(int x, int y, SkPMColor dstC[], int count) override;

        sk_sp<SkShader>     fBitmapShader;
        SkShader::Context*  fBitmapShaderContext;
        void*               fBitmapShaderContextStorage;

        typedef SkShader::Context INHERITED;
    };

    typedef SkShader INHERITED;
};

#endif // SkPictureShader_DEFINED
