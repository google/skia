/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPictureShader_DEFINED
#define SkPictureShader_DEFINED

#include "include/core/SkTileMode.h"
#include "src/shaders/SkShaderBase.h"
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

    enum FilterEnum {
        kNearest,           // SkFilterMode::kNearest
        kLinear,            // SkFilterMode::kLinear
        kInheritFromPaint,

        kLastFilterEnum = kInheritFromPaint,
    };

    static sk_sp<SkShader> Make(sk_sp<SkPicture>, SkTileMode, SkTileMode, FilterEnum,
                                const SkMatrix*, const SkRect*);

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs&) const override;
#endif

protected:
    SkPictureShader(SkReadBuffer&);
    void flatten(SkWriteBuffer&) const override;
    bool onAppendStages(const SkStageRec&) const override;
    skvm::Color onProgram(skvm::Builder*, skvm::Coord device, skvm::Coord local, skvm::Color paint,
                          const SkMatrixProvider&, const SkMatrix* localM,
                          SkFilterQuality quality, const SkColorInfo& dst,
                          skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const override;

#ifdef SK_ENABLE_LEGACY_SHADERCONTEXT
    Context* onMakeContext(const ContextRec&, SkArenaAlloc*) const override;
#endif

private:
    SK_FLATTENABLE_HOOKS(SkPictureShader)

    SkPictureShader(sk_sp<SkPicture>, SkTileMode, SkTileMode, FilterEnum,
                    const SkMatrix*, const SkRect*);

    sk_sp<SkShader> refBitmapShader(const SkMatrix&, SkTCopyOnFirstWrite<SkMatrix>* localMatrix,
                                    SkColorType dstColorType, SkColorSpace* dstColorSpace,
                                    SkFilterQuality fromPaint,
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

        using INHERITED = Context;
    };

    sk_sp<SkPicture>    fPicture;
    SkRect              fTile;
    SkTileMode          fTmx, fTmy;
    FilterEnum          fFilter;

    const uint32_t            fUniqueID;
    mutable std::atomic<bool> fAddedToCache;

    using INHERITED = SkShaderBase;
};

#endif // SkPictureShader_DEFINED
