// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Picture_MakeFromStream, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPictureRecorder recorder;
    SkCanvas* pictureCanvas = recorder.beginRecording({0, 0, 256, 256});
    SkPaint paint;
    pictureCanvas->drawRect(SkRect::MakeWH(200, 200), paint);
    paint.setColor(SK_ColorWHITE);
    pictureCanvas->drawRect(SkRect::MakeLTRB(20, 20, 180, 180), paint);
    sk_sp<SkPicture> picture = recorder.finishRecordingAsPicture();
    SkDynamicMemoryWStream writableStream;

    // The above drawing doesn't have any images to serialize, but if such
    // serialization were necessary, one would need to register an encoder.
    SkSerialProcs sProcs;
    sProcs.fImageProc = [](SkImage* img, void*) -> SkSerialReturnType {
#if defined(SK_CODEC_ENCODES_PNG_WITH_RUST)
        return SkPngRustEncoder::Encode(nullptr, img, SkPngRustEncoder::Options{});
#else
        return SkPngEncoder::Encode(nullptr, img, SkPngEncoder::Options{});
#endif
    };
    picture->serialize(&writableStream, &sProcs);
    std::unique_ptr<SkStreamAsset> readableStream = writableStream.detachAsStream();

    SkDeserialProcs dProcs;
    // The above SKP doesn't have any image data in it, but if it did, a decoder
    // needs to be registered like this.
    dProcs.fImageDataProc =
            [](sk_sp<SkData> data, std::optional<SkAlphaType> at, void*) -> sk_sp<SkImage> {
#if defined(SK_CODEC_DECODES_PNG_WITH_RUST)
        std::unique_ptr<SkStream> stream = SkMemoryStream::Make(std::move(data));
        auto codec = SkPngRustDecoder::Decode(std::move(stream), nullptr, nullptr);
#else
        auto codec = SkPngDecoder::Decode(std::move(data), nullptr, nullptr);
#endif
        if (!codec) {
            SkDebugf("Invalid png data detected\n");
            return nullptr;
        }
        if (auto lazyImage = SkCodecs::DeferredImage(std::move(codec), at)) {
            return lazyImage->makeRasterImage(/*GrDirectContext=*/nullptr);
        }
        return nullptr;
    };
    sk_sp<SkPicture> copy = SkPicture::MakeFromStream(readableStream.get(), &dProcs);
    copy->playback(canvas);
}
}  // END FIDDLE
