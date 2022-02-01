/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkCodecImageGenerator_DEFINED
#define SkCodecImageGenerator_DEFINED

#include "include/codec/SkCodec.h"
#include "include/core/SkData.h"
#include "include/core/SkImageGenerator.h"
#include "include/private/SkTOptional.h"

class SkCodecImageGenerator : public SkImageGenerator {
public:
    /*
     * If this data represents an encoded image that we know how to decode,
     * return an SkCodecImageGenerator.  Otherwise return nullptr.
     */
    static std::unique_ptr<SkImageGenerator> MakeFromEncodedCodec(
            sk_sp<SkData>, skstd::optional<SkAlphaType> = skstd::nullopt);

    static std::unique_ptr<SkImageGenerator> MakeFromCodec(std::unique_ptr<SkCodec>);

    /**
     * Return a size that approximately supports the desired scale factor. The codec may not be able
     * to scale efficiently to the exact scale factor requested, so return a size that approximates
     * that scale. The returned value is the codec's suggestion for the closest valid scale that it
     * can natively support.
     *
     * This is similar to SkCodec::getScaledDimensions, but adjusts the returned dimensions based
     * on the image's EXIF orientation.
     */
    SkISize getScaledDimensions(float desiredScale) const;

    /**
     *  Decode into the given pixels, a block of memory of size at
     *  least (info.fHeight - 1) * rowBytes + (info.fWidth *
     *  bytesPerPixel)
     *
     *  Repeated calls to this function should give the same results,
     *  allowing the PixelRef to be immutable.
     *
     *  @param info A description of the format
     *         expected by the caller.  This can simply be identical
     *         to the info returned by getInfo().
     *
     *         This contract also allows the caller to specify
     *         different output-configs, which the implementation can
     *         decide to support or not.
     *
     *         A size that does not match getInfo() implies a request
     *         to scale. If the generator cannot perform this scale,
     *         it will return false.
     *
     *  @return true on success.
     */
    bool getPixels(const SkImageInfo& info, void* pixels, size_t rowBytes, const SkCodec::Options* options = nullptr);

    /**
     *  Return the number of frames in the image.
     *
     *  May require reading through the stream.
     */
    int getFrameCount() { return fCodec->getFrameCount(); }

    /**
     *  Return info about a single frame.
     *
     *  Only supported by multi-frame images. Does not read through the stream,
     *  so it should be called after getFrameCount() to parse any frames that
     *  have not already been parsed.
     */
    bool getFrameInfo(int index, SkCodec::FrameInfo* info) const {
        return fCodec->getFrameInfo(index, info);
    }

    /**
     *  Return the number of times to repeat, if this image is animated. This number does not
     *  include the first play through of each frame. For example, a repetition count of 4 means
     *  that each frame is played 5 times and then the animation stops.
     *
     *  It can return kRepetitionCountInfinite, a negative number, meaning that the animation
     *  should loop forever.
     *
     *  May require reading the stream to find the repetition count.
     *
     *  As such, future decoding calls may require a rewind.
     *
     *  For still (non-animated) image codecs, this will return 0.
     */
    int getRepetitionCount() { return fCodec->getRepetitionCount(); }

protected:
    sk_sp<SkData> onRefEncodedData() override;

    bool onGetPixels(const SkImageInfo& info,
                     void* pixels,
                     size_t rowBytes,
                     const Options& opts) override;

    bool onQueryYUVAInfo(const SkYUVAPixmapInfo::SupportedDataTypes&,
                         SkYUVAPixmapInfo*) const override;

    bool onGetYUVAPlanes(const SkYUVAPixmaps& yuvaPixmaps) override;

private:
    /*
     * Takes ownership of codec
     */
    SkCodecImageGenerator(std::unique_ptr<SkCodec>, sk_sp<SkData>, skstd::optional<SkAlphaType>);

    std::unique_ptr<SkCodec> fCodec;
    sk_sp<SkData> fData;

    using INHERITED = SkImageGenerator;
};
#endif  // SkCodecImageGenerator_DEFINED
