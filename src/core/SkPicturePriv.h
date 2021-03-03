/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPicturePriv_DEFINED
#define SkPicturePriv_DEFINED

#include "include/core/SkPicture.h"

class SkReadBuffer;
class SkWriteBuffer;

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
    static const SkBigPicture* AsSkBigPicture(const sk_sp<const SkPicture> picture) {
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

    enum Version {
        kEdgeAAQuadColor4f_Version          = 73,
        kMorphologyTakesScalar_Version      = 74,
        kVerticesUseReadBuffer_Version      = 75,
        kFilterEnumInImageShader_Version    = 76,
        kFilterOptionsInImageShader_Version = 77,
        kSerializeMipmaps_Version           = 78,
        kCubicResamplerImageShader_Version  = 79,
        kSamplingInImageShader_Version      = 80,
        kSamplingInDrawImage_Version        = 81,
        kPictureShaderFilterParam_Version   = 82,
        kMatrixImageFilterSampling_Version  = 83,
        kImageFilterImageSampling_Version   = 84,

        // Only SKPs within the min/current picture version range (inclusive) can be read.
        kMin_Version     = kEdgeAAQuadColor4f_Version,
        kCurrent_Version = kImageFilterImageSampling_Version
    };

    static_assert(SkPicturePriv::kMin_Version <= SkPicturePriv::kCubicResamplerImageShader_Version,
        "Remove SkFontDescriptor::maybeAsSkFontData, SkFontMgr::makeFromFontData, kFontAxes");
};

#endif
