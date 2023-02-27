/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJpegEncoder_DEFINED
#define SkJpegEncoder_DEFINED

#include "include/encode/SkEncoder.h"
#include "include/private/base/SkAPI.h"

#include <memory>

class SkColorSpace;
class SkData;
class SkJpegEncoderMgr;
class SkPixmap;
class SkWStream;
class SkYUVAPixmaps;
struct skcms_ICCProfile;

class SK_API SkJpegEncoder : public SkEncoder {
public:

    enum class AlphaOption {
        kIgnore,
        kBlendOnBlack,
    };

    enum class Downsample {
        /**
         *  Reduction by a factor of two in both the horizontal and vertical directions.
         */
        k420,

        /**
         *  Reduction by a factor of two in the horizontal direction.
         */
        k422,

        /**
         *  No downsampling.
         */
        k444,
    };

    struct Options {
        /**
         *  |fQuality| must be in [0, 100] where 0 corresponds to the lowest quality.
         */
        int fQuality = 100;

        /**
         *  Choose the downsampling factor for the U and V components.  This is only
         *  meaningful if the |src| is not kGray, since kGray will not be encoded as YUV.
         *  This is ignored in favor of |src|'s subsampling when |src| is an SkYUVAPixmaps.
         *
         *  Our default value matches the libjpeg-turbo default.
         */
        Downsample fDownsample = Downsample::k420;

        /**
         *  Jpegs must be opaque.  This instructs the encoder on how to handle input
         *  images with alpha.
         *
         *  The default is to ignore the alpha channel and treat the image as opaque.
         *  Another option is to blend the pixels onto a black background before encoding.
         *  In the second case, the encoder supports linear or legacy blending.
         */
        AlphaOption fAlphaOption = AlphaOption::kIgnore;

        /**
         *  Optional XMP metadata.
         */
        const SkData* xmpMetadata = nullptr;

        /**
         *  An optional ICC profile to override the default behavior.
         *
         *  The default behavior is to generate an ICC profile using a primary matrix and
         *  analytic transfer function. If the color space of |src| cannot be represented
         *  in this way (e.g, it is HLG or PQ), then no profile will be embedded.
         */
        const skcms_ICCProfile* fICCProfile = nullptr;
        const char* fICCProfileDescription = nullptr;
    };

    /**
     *  Encode the |src| pixels to the |dst| stream.
     *  |options| may be used to control the encoding behavior.
     *
     *  Returns true on success.  Returns false on an invalid or unsupported |src|.
     */
    static bool Encode(SkWStream* dst, const SkPixmap& src, const Options& options);
    static bool Encode(SkWStream* dst,
                       const SkYUVAPixmaps& src,
                       const SkColorSpace* srcColorSpace,
                       const Options& options);

    /**
     *  Create a jpeg encoder that will encode the |src| pixels to the |dst| stream.
     *  |options| may be used to control the encoding behavior.
     *
     *  |dst| is unowned but must remain valid for the lifetime of the object.
     *
     *  This returns nullptr on an invalid or unsupported |src|.
     */
    static std::unique_ptr<SkEncoder> Make(SkWStream* dst, const SkPixmap& src,
                                           const Options& options);
    static std::unique_ptr<SkEncoder> Make(SkWStream* dst,
                                           const SkYUVAPixmaps& src,
                                           const SkColorSpace* srcColorSpace,
                                           const Options& options);

    ~SkJpegEncoder() override;

protected:
    bool onEncodeRows(int numRows) override;

private:
    SkJpegEncoder(std::unique_ptr<SkJpegEncoderMgr>, const SkPixmap& src);
    SkJpegEncoder(std::unique_ptr<SkJpegEncoderMgr>, const SkYUVAPixmaps* srcYUVA);

    static std::unique_ptr<SkEncoder> Make(SkWStream* dst,
                                           const SkPixmap* src,
                                           const SkYUVAPixmaps* srcYUVA,
                                           const SkColorSpace* srcYUVAColorSpace,
                                           const Options& options);

    std::unique_ptr<SkJpegEncoderMgr> fEncoderMgr;
    const SkYUVAPixmaps* fSrcYUVA = nullptr;
    using INHERITED = SkEncoder;
};

#endif
