/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJpegRCodec_DEFINED
#define SkJpegRCodec_DEFINED

#include "include/codec/SkCodec.h"
#include "include/codec/SkEncodedOrigin.h"
#include "include/core/SkEncodedImageFormat.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkTypes.h"
#include "include/private/SkEncodedInfo.h"
#include "include/private/SkTemplates.h"

#ifdef SK_CODEC_DECODES_JPEGR
#if __has_include("jpegrecoverymap/recoverymap.h")
#include "jpegrecoverymap/recoverymap.h"
using namespace android::recoverymap;
#else
#include "src/codec/SkStubJpegRDecoderAPI.h"
#endif

#endif

#include <cstddef>
#include <cstdint>
#include <memory>

class SkStream;
struct SkImageInfo;

/*
 *
 * This class implements the decoding for jpeg-r images
 *
 */
class SkJpegRCodec : public SkCodec {
public:
    ~SkJpegRCodec() override;

    static bool IsJpegR(const void*, size_t);

    /*
     * Assumes IsJpegR was called and returned true
     * Takes ownership of the stream
     */
    static std::unique_ptr<SkCodec> MakeFromStream(std::unique_ptr<SkStream>, Result*);

#ifdef SK_CODEC_DECODES_JPEGR
protected:
    /*
     * Initiates the jpeg-r decode
     */
    Result onGetPixels(const SkImageInfo& dstInfo,
                       void* dst,
                       size_t dstRowBytes,
                       const Options&,
                       int*) override;

    SkEncodedImageFormat onGetEncodedFormat() const override {
        return SkEncodedImageFormat::kJPEGR;
    }

    bool conversionSupported(const SkImageInfo&, bool, bool) override;

    // TODO: Implement Color transform
    bool usesColorXform() const override { return false; }

private:
    /*
     * Read enough of the stream to initialize the SkJpegCodec.
     * Returns a bool representing success or failure.
     *
     * @param codecOut
     * If this returns true, and codecOut was not nullptr,
     * codecOut will be set to a new SkJpegCodec.
     *
     * @param recoveryMapOut
     * If this returns true, and codecOut was nullptr,
     * recoveryMapOut must be non-nullptr and recoveryMapOut will be set to a new
     * fRecoveryMap pointer.
     *
     * @param stream
     * Deleted on failure.
     * codecOut will take ownership of it in the case where we created a codec.
     * Ownership is unchanged when we set recoveryMapOut.
     */
    static Result ReadHeader(SkStream* stream, SkCodec** codecOut, RecoveryMap** recoveryMapOut);

    /*
     * Creates an instance of the decoder
     * Called only by MakeFromStream
     *
     * @param info contains properties of the encoded data
     * @param stream the encoded image data
     * @param recoveryMap holds RecoveryMap Decoder
     *                   takes ownership
     * @param origin JPEGR image origin
     * @param data JPEGR image in memory
     */
    SkJpegRCodec(SkEncodedInfo&& info,
                 std::unique_ptr<SkStream> stream,
                 RecoveryMap* recoveryMap,
                 SkEncodedOrigin origin,
                 sk_sp<SkData> data);

    std::unique_ptr<RecoveryMap> fRecoveryMap;
    sk_sp<SkData> fData;

#endif  // SK_CODEC_DECODES_JPEGR
};

#endif  // SkJpegRCodec_DEFINED
