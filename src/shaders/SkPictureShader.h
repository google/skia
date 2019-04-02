/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPictureShader_DEFINED
#define SkPictureShader_DEFINED

#include "SkShaderBase.h"
#include "SkTileMode.h"
#include <atomic>

class SkArenaAlloc;
class SkBitmap;
class SkPicture;

/*
 * An SkPictureShader can be used to draw SkPicture-based patterns.
 *
 * The SkPicture is first rendered into a tile, which is then used to shade the area according
 * to specified tiling rules.
 */
class SkPictureShader : public SkShaderBase {
public:
    ~SkPictureShader() override;

    static sk_sp<SkShader> Make(sk_sp<SkPicture>, SkTileMode, SkTileMode, const SkMatrix*,
                                const SkRect*);

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs&) const override;
#endif

protected:
    SkPictureShader(SkReadBuffer&);
    void flatten(SkWriteBuffer&) const override;
    bool onAppendStages(const SkStageRec&) const override;
#ifdef SK_ENABLE_LEGACY_SHADERCONTEXT
    Context* onMakeContext(const ContextRec&, SkArenaAlloc*) const override;
#endif

private:
    SK_FLATTENABLE_HOOKS(SkPictureShader)

    SkPictureShader(sk_sp<SkPicture>, TileMode, TileMode, const SkMatrix*, const SkRect*);

    sk_sp<SkShader> refBitmapShader(const SkMatrix&, SkTCopyOnFirstWrite<SkMatrix>* localMatrix,
                                    SkColorType dstColorType, SkColorSpace* dstColorSpace,
                                    const int maxTextureSize = 0) const;

    class PictureShaderContext : public Context {
    public:
        PictureShaderContext(
            const SkPictureShader&, const ContextRec&, sk_sp<SkShader> bitmapShader, SkArenaAlloc*);

        uint32_t getFlags() const override;

        void shadeSpan(int x, int y, SkPMColor dstC[], int count) override;

        sk_sp<SkShader>         fBitmapShader;
        SkShaderBase::Context*  fBitmapShaderContext;
        void*                   fBitmapShaderContextStorage;

        typedef Context INHERITED;
    };

    sk_sp<SkPicture>    fPicture;
    SkRect              fTile;
    TileMode            fTmx, fTmy;

    const uint32_t            fUniqueID;
    mutable std::atomic<bool> fAddedToCache;

    typedef SkShaderBase INHERITED;
};

#endif // SkPictureShader_DEFINED
