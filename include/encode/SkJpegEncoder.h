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

#include <cstddef>
#include <cstdint>
#include <memory>

class SkData;
class SkJpegEncoderMgr;
class SkPixmap;
class SkWStream;
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
         * An optional ICC profile to override the default behavior.
         *
         * The default behavior is to generate an ICC profile using a primary matrix and
         * analytic transfer function. If the color space of |src| cannot be represented
         * in this way (e.g, it is HLG or PQ), then no profile will be embedded.
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

    ~SkJpegEncoder() override;

protected:
    bool onEncodeRows(int numRows) override;

private:
    friend class SkJpegGainmapEncoder;
    SkJpegEncoder(std::unique_ptr<SkJpegEncoderMgr>, const SkPixmap& src);

    /**
     *  Create a jpeg encoder that will encode the |src| pixels and |segmentData| to the |dst|
     *  stream, followed by the data in |suffix|. |options| may be used to control the encoding
     *  behavior.
     *
     *  |segmentCount| lists the number of metadata segments to include. |segmentMarker| lists the
     *  marker type identifiers for each segment (e.g: 0xE1 for APP1), and |segmentData| lists the
     *  data for each segment.
     *
     *  |dst|, |makerTypes|, |segmentData|, and |suffix| are unowned and must remain valid for the
     *  lifetime of the object.
     *
     *  This returns nullptr on an invalid or unsupported |src|.
     */
    static constexpr size_t kSegmentDataMaxSize = 65533;
    static std::unique_ptr<SkEncoder> Make(SkWStream* dst,
                                           const SkPixmap& src,
                                           const Options& options,
                                           size_t segmentCount,
                                           uint8_t* segmentMarkers,
                                           SkData** segmentData,
                                           SkData* suffix);

    std::unique_ptr<SkJpegEncoderMgr> fEncoderMgr;
    using INHERITED = SkEncoder;
};

#endif
