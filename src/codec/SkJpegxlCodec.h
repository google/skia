/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJpegxlCodec_DEFINED
#define SkJpegxlCodec_DEFINED

#include "include/core/SkData.h"
#include "include/core/SkEncodedImageFormat.h"
#include "include/core/SkRefCnt.h"
#include "src/codec/SkScalingCodec.h"

#include <cstddef>
#include <memory>

class SkCodec;
class SkFrameHolder;
class SkJpegxlCodecPriv;
class SkStream;
struct SkEncodedInfo;
struct SkImageInfo;

/*
 *
 * This class implements the decoding for jpegxl images
 *
 */
class SkJpegxlCodec : public SkScalingCodec {
public:
    static bool IsJpegxl(const void*, size_t);

    /*
     * Assumes IsJpegxl was called and returned true
     * Takes ownership of the stream
     */
    static std::unique_ptr<SkCodec> MakeFromStream(std::unique_ptr<SkStream>, Result*);

protected:
    /* TODO(eustas): implement when downscaling is supported. */
    /* SkISize onGetScaledDimensions(float desiredScale) const override; */

    /* TODO(eustas): implement when up-/down-scaling is supported. */
    /* bool onDimensionsSupported(const SkISize&) override; */

    SkEncodedImageFormat onGetEncodedFormat() const override {
        return SkEncodedImageFormat::kJPEGXL;
    }

    Result onGetPixels(const SkImageInfo& dstInfo, void* dst, size_t rowBytes,
                       const Options& options, int* rowsDecodedPtr) override;

    /* TODO(eustas): add support for transcoded JPEG images? */
    /* bool onQueryYUVAInfo(const SkYUVAPixmapInfo::SupportedDataTypes&,
                            SkYUVAPixmapInfo*) const override; */

    /* TODO(eustas): add support for transcoded JPEG images? */
    /* Result onGetYUVAPlanes(const SkYUVAPixmaps& yuvaPixmaps) override; */

    /* TODO(eustas): implement when cropped output is supported. */
    /* bool onGetValidSubset(SkIRect* desiredSubset) const override; */

    bool onRewind() override;

    /* TODO(eustas): top-down by default; do we need something else? */
    /* SkScanlineOrder onGetScanlineOrder() const override; */
    /* int onOutputScanline(int inputScanline) const override; */

    bool conversionSupported(const SkImageInfo&, bool, bool) override;

    int onGetFrameCount() override;

    bool onGetFrameInfo(int, FrameInfo*) const override;

    int onGetRepetitionCount() override;

private:
    const SkFrameHolder* getFrameHolder() const override;

    // Result onStartScanlineDecode(
    //    const SkImageInfo& /*dstInfo*/, const Options& /*options*/) override;
    // Result onStartIncrementalDecode(
    //     const SkImageInfo& /*dstInfo*/, void*, size_t, const Options&) override;
    // Result onIncrementalDecode(int*) override;
    // bool onSkipScanlines(int /*countLines*/) override;
    // int onGetScanlines(void* /*dst*/, int /*countLines*/, size_t /*rowBytes*/) override;
    // SkSampler* getSampler(bool /*createIfNecessary*/) override;

    // Opaque codec implementation for lightweight header file.
    std::unique_ptr<SkJpegxlCodecPriv> fCodec;
    sk_sp<SkData> fData;

    bool scanFrames();
    static void imageOutCallback(
        void* opaque, size_t x, size_t y, size_t num_pixels, const void* pixels);

    SkJpegxlCodec(std::unique_ptr<SkJpegxlCodecPriv> codec,
                  SkEncodedInfo&& info,
                  std::unique_ptr<SkStream> stream,
                  sk_sp<SkData> data);

    using INHERITED = SkScalingCodec;
};

#endif
