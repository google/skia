/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPicturePriv_DEFINED
#define SkPicturePriv_DEFINED

#include "include/core/SkFourByteTag.h"
#include "include/core/SkPicture.h"
#include "include/core/SkRefCnt.h"

#include <atomic>
#include <cstdint>

class SkBigPicture;
class SkReadBuffer;
class SkStream;
class SkWriteBuffer;
struct SkPictInfo;

class SkPicturePriv {
public:
    /**
     *  Recreate a picture that was serialized into a buffer. If the creation requires bitmap
     *  decoding, the decoder must be set on the SkReadBuffer parameter by calling
     *  SkReadBuffer::setBitmapDecoder() before calling SkPicture::MakeFromBuffer().
     *  @param buffer Serialized picture data.
     *  @return A new SkPicture representing the serialized data, or NULL if the buffer is
     *          invalid.
     */
    static sk_sp<SkPicture> MakeFromBuffer(SkReadBuffer& buffer);

    /**
     *  Serialize to a buffer.
     */
    static void Flatten(const sk_sp<const SkPicture> , SkWriteBuffer& buffer);

    // Returns NULL if this is not an SkBigPicture.
    static const SkBigPicture* AsSkBigPicture(const sk_sp<const SkPicture>& picture) {
        return picture->asSkBigPicture();
    }

    static uint64_t MakeSharedID(uint32_t pictureID) {
        uint64_t sharedID = SkSetFourByteTag('p', 'i', 'c', 't');
        return (sharedID << 32) | pictureID;
    }

    static void AddedToCache(const SkPicture* pic) {
        pic->fAddedToCache.store(true);
    }

    // V35: Store SkRect (rather then width & height) in header
    // V36: Remove (obsolete) alphatype from SkColorTable
    // V37: Added shadow only option to SkDropShadowImageFilter (last version to record CLEAR)
    // V38: Added PictureResolution option to SkPictureImageFilter
    // V39: Added FilterLevel option to SkPictureImageFilter
    // V40: Remove UniqueID serialization from SkImageFilter.
    // V41: Added serialization of SkBitmapSource's filterQuality parameter
    // V42: Added a bool to SkPictureShader serialization to indicate did-we-serialize-a-picture?
    // V43: Added DRAW_IMAGE and DRAW_IMAGE_RECT opt codes to serialized data
    // V44: Move annotations from paint to drawAnnotation
    // V45: Add invNormRotation to SkLightingShader.
    // V46: Add drawTextRSXform
    // V47: Add occluder rect to SkBlurMaskFilter
    // V48: Read and write extended SkTextBlobs.
    // V49: Gradients serialized as SkColor4f + SkColorSpace
    // V50: SkXfermode -> SkBlendMode
    // V51: more SkXfermode -> SkBlendMode
    // V52: Remove SkTextBlob::fRunCount
    // V53: SaveLayerRec clip mask
    // V54: ComposeShader can use a Mode or a Lerp
    // V55: Drop blendmode[] from MergeImageFilter
    // V56: Add TileMode in SkBlurImageFilter.
    // V57: Sweep tiling info.
    // V58: No more 2pt conical flipping.
    // V59: No more LocalSpace option on PictureImageFilter
    // V60: Remove flags in picture header
    // V61: Change SkDrawPictureRec to take two colors rather than two alphas
    // V62: Don't negate size of custom encoded images (don't write origin x,y either)
    // V63: Store image bounds (including origin) instead of just width/height to support subsets
    // V64: Remove occluder feature from blur maskFilter
    // V65: Float4 paint color
    // V66: Add saveBehind
    // V67: Blobs serialize fonts instead of paints
    // V68: Paint doesn't serialize font-related stuff
    // V69: Clean up duplicated and redundant SkImageFilter related enums
    // V70: Image filters definitions hidden, registered names updated to include "Impl"
    // V71: Unify erode and dilate image filters
    // V72: SkColorFilter_Matrix domain (rgba vs. hsla)
    // V73: Use SkColor4f in per-edge AA quad API
    // V74: MorphologyImageFilter internal radius is SkScaler
    // V75: SkVertices switched from unsafe use of SkReader32 to SkReadBuffer (like everything else)
    // V76: Add filtering enum to ImageShader
    // V77: Explicit filtering options on imageshaders
    // V78: Serialize skmipmap data for images that have it
    // V79: Cubic Resampler option on imageshader
    // V80: Smapling options on imageshader
    // V81: sampling parameters on drawImage/drawImageRect/etc.
    // V82: Add filter param to picture-shader
    // V83: SkMatrixImageFilter now takes SkSamplingOptions instead of SkFilterQuality
    // V84: SkImageFilters::Image now takes SkSamplingOptions instead of SkFilterQuality
    // V85: Remove legacy support for inheriting sampling from the paint.
    // V86: Remove support for custom data inside SkVertices
    // V87: SkPaint now holds a user-defined blend function (SkBlender), no longer has DrawLooper
    // V88: Add blender to ComposeShader and BlendImageFilter
    // V89: Deprecated SkClipOps are no longer supported
    // V90: Private API for backdrop scale factor in SaveLayerRec
    // V91: Added raw image shaders
    // V92: Added anisotropic filtering to SkSamplingOptions
    // V94: Removed local matrices from SkShaderBase. Local matrices always use SkLocalMatrixShader.
    // V95: SkImageFilters::Shader only saves SkShader, not a full SkPaint
    // V96: SkImageFilters::Magnifier updated with more complete parameters
    // V97: SkImageFilters::RuntimeShader takes a sample radius
    // V98: Merged SkImageFilters::Blend and ::Arithmetic implementations
    // V99: Remove legacy Magnifier filter
    // V100: SkImageFilters::DropShadow does not have a dedicated implementation
    // V101: Crop image filter supports all SkTileModes instead of just kDecal
    // V102: Convolution image filter uses ::Crop to apply tile mode
    // V103: Remove deprecated per-image filter crop rect
    // v104: SaveLayer supports multiple image filters
    // v105: Unclamped matrix color filter
    // v106: SaveLayer supports custom backdrop tile modes
    // v107: Combine SkColorShader and SkColorShader4

    enum Version {
        kPictureShaderFilterParam_Version   = 82,
        kMatrixImageFilterSampling_Version  = 83,
        kImageFilterImageSampling_Version   = 84,
        kNoFilterQualityShaders_Version     = 85,
        kVerticesRemoveCustomData_Version   = 86,
        kSkBlenderInSkPaint                 = 87,
        kBlenderInEffects                   = 88,
        kNoExpandingClipOps                 = 89,
        kBackdropScaleFactor                = 90,
        kRawImageShaders                    = 91,
        kAnisotropicFilter                  = 92,
        kBlend4fColorFilter                 = 93,
        kNoShaderLocalMatrix                = 94,
        kShaderImageFilterSerializeShader   = 95,
        kRevampMagnifierFilter              = 96,
        kRuntimeImageFilterSampleRadius     = 97,
        kCombineBlendArithmeticFilters      = 98,
        kRemoveLegacyMagnifierFilter        = 99,
        kDropShadowImageFilterComposition   = 100,
        kCropImageFilterSupportsTiling      = 101,
        kConvolutionImageFilterTilingUpdate = 102,
        kRemoveDeprecatedCropRect           = 103,
        kMultipleFiltersOnSaveLayer         = 104,
        kUnclampedMatrixColorFilter         = 105,
        kSaveLayerBackdropTileMode          = 106,
        kCombineColorShaders                = 107,
        kSerializeStableKeys                = 108,

        // Only SKPs within the min/current picture version range (inclusive) can be read.
        //
        // When updating kMin_Version also update oldestSupportedSkpVersion in
        // infra/bots/gen_tasks_logic/gen_tasks_logic.go
        //
        // Steps on how to find which oldestSupportedSkpVersion to use:
        // 1) Find the git hash when the desired kMin_Version was the kCurrent_Version from the
        //    git logs: https://skia.googlesource.com/skia/+log/main/src/core/SkPicturePriv.h
        //    Eg: https://skia.googlesource.com/skia/+/bfd330d081952424a93d51715653e4d1314d4822%5E%21/#F1
        //
        // 2) Use that git hash to find the SKP asset version number at that time here:
        //    https://skia.googlesource.com/skia/+/bfd330d081952424a93d51715653e4d1314d4822/infra/bots/assets/skp/VERSION
        //
        // 3) [Optional] Increment the SKP asset version number from step 3 and verify that it has
        //    the expected version number by downloading the asset and running skpinfo on it.
        //
        // 4) Use the incremented SKP asset version number as the oldestSupportedSkpVersion in
        //    infra/bots/gen_tasks_logic/gen_tasks_logic.go
        //
        // 5) Run `make -C infra/bots train`
        //
        // Contact the Infra Gardener if the above steps do not work for you.
        kMin_Version     = kPictureShaderFilterParam_Version,
        kCurrent_Version = kSerializeStableKeys
    };
};

bool SkPicture_StreamIsSKP(SkStream*, SkPictInfo*);

#endif
