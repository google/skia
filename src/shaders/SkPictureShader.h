/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPictureShader_DEFINED
#define SkPictureShader_DEFINED

#include "SkAtomics.h"
#include "SkShaderBase.h"

class SkArenaAlloc;
class SkBitmap;
class SkImageGenerator;
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

    static sk_sp<SkShader> Make(sk_sp<SkPicture>, TileMode, TileMode, const SkMatrix*,
                                const SkRect*);

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkPictureShader)

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs&) const override;
#endif

protected:
    SkPictureShader(SkReadBuffer&);
    void flatten(SkWriteBuffer&) const override;
    bool onAppendStages(const StageRec&) const override;
    Context* onMakeContext(const ContextRec&, SkArenaAlloc*) const override;
    sk_sp<SkShader> onMakeColorSpace(SkColorSpaceXformer* xformer) const override;
    bool onIsRasterPipelineOnly(const SkMatrix&) const override;

private:
    SkPictureShader(sk_sp<SkPicture>, TileMode, TileMode, const SkMatrix*, const SkRect*,
                    sk_sp<SkColorSpace>);


    struct TileInfo;
    TileInfo computeTileInfo(const SkMatrix& ctm,
                             const SkMatrix* localMatrix,
                             SkColorSpace* dstCS,
                             int maxTileSize = std::numeric_limits<int>::max()) const;
    std::unique_ptr<SkImageGenerator> makeTileGenerator(const TileInfo&) const;

    sk_sp<SkShader> lockTile(const TileInfo&) const;

#if SK_SUPPORT_GPU
    sk_sp<SkShader> lockTile(const TileInfo&, const GrFPArgs&) const;
#endif

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

    // Should never be set by a public constructor.  This is only used when onMakeColorSpace()
    // forces a deferred color space xform.
    sk_sp<SkColorSpace>    fColorSpace;

    const uint32_t         fUniqueID;
    mutable SkAtomic<bool> fAddedToCache;

    typedef SkShaderBase INHERITED;
};

#endif // SkPictureShader_DEFINED
