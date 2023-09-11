/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPictureShader_DEFINED
#define SkPictureShader_DEFINED

#include "include/core/SkFlattenable.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPicture.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypes.h"
#include "src/shaders/SkShaderBase.h"

class SkArenaAlloc;
class SkColorSpace;
class SkImage;
class SkReadBuffer;
class SkShader;
class SkSurface;
class SkWriteBuffer;
enum SkColorType : int;
enum class SkFilterMode;
enum class SkTileMode;
struct SkStageRec;

/*
 * An SkPictureShader can be used to draw SkPicture-based patterns.
 *
 * The SkPicture is first rendered into a tile, which is then used to shade the area according
 * to specified tiling rules.
 */
class SkPictureShader : public SkShaderBase {
public:
    static sk_sp<SkShader> Make(sk_sp<SkPicture>, SkTileMode, SkTileMode, SkFilterMode,
                                const SkMatrix*, const SkRect*);

    SkPictureShader(sk_sp<SkPicture>, SkTileMode, SkTileMode, SkFilterMode, const SkRect*);

    ShaderType type() const override { return ShaderType::kPicture; }

    sk_sp<SkPicture> picture() const { return fPicture; }
    SkRect tile() const { return fTile; }
    SkTileMode tileModeX() const { return fTmx; }
    SkTileMode tileModeY() const { return fTmy; }
    SkFilterMode filter() const { return fFilter; }

    struct CachedImageInfo {
        bool success;
        SkSize tileScale;        // Additional scale factors to apply when sampling image.
        SkMatrix matrixForDraw;  // Matrix used to produce an image from the picture
        SkImageInfo imageInfo;
        SkSurfaceProps props;

        static CachedImageInfo Make(const SkRect& bounds,
                                    const SkMatrix& totalM,
                                    SkColorType dstColorType,
                                    SkColorSpace* dstColorSpace,
                                    const int maxTextureSize,
                                    const SkSurfaceProps& propsIn);

        sk_sp<SkImage> makeImage(sk_sp<SkSurface> surf, const SkPicture* pict) const;
    };

protected:
    SkPictureShader(SkReadBuffer&);
    void flatten(SkWriteBuffer&) const override;
    bool appendStages(const SkStageRec&, const SkShaders::MatrixRec&) const override;
#ifdef SK_ENABLE_LEGACY_SHADERCONTEXT
    Context* onMakeContext(const ContextRec&, SkArenaAlloc*) const override;
#endif

private:
    SK_FLATTENABLE_HOOKS(SkPictureShader)

    sk_sp<SkShader> rasterShader(const SkMatrix&,
                                 SkColorType dstColorType,
                                 SkColorSpace* dstColorSpace,
                                 const SkSurfaceProps& props) const;

    sk_sp<SkPicture>    fPicture;
    SkRect              fTile;
    SkTileMode          fTmx, fTmy;
    SkFilterMode fFilter;
};

#endif // SkPictureShader_DEFINED
