/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "fuzz/Fuzz.h"
#include "fuzz/FuzzCanvasHelpers.h"
#include "fuzz/FuzzCommon.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkSerialProcs.h"
#include "include/core/SkStream.h"
#include "include/docs/SkPDFDocument.h"
#include "include/docs/SkPDFJpegHelpers.h"
#include "include/svg/SkSVGCanvas.h"
#include "include/utils/SkNullCanvas.h"
#include "src/core/SkReadBuffer.h"
#include "src/utils/SkJSONWriter.h"
#include "tools/UrlDataManager.h"
#include "tools/debugger/DebugCanvas.h"
#include "tools/flags/CommandLineFlags.h"
#include "tools/fonts/FontToolUtils.h"

#include <iostream>
#include <utility>

#if defined(SK_GANESH)
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "tools/ganesh/GrContextFactory.h"
#endif

#if defined(SK_GL)
#include "include/gpu/ganesh/gl/GrGLFunctions.h"
#include "src/gpu/ganesh/gl/GrGLGpu.h"
#include "src/gpu/ganesh/gl/GrGLUtil.h"
#endif


static DEFINE_bool2(gpuInfo, g, false, "Display GPU information on relevant targets.");

DEF_FUZZ(NullCanvas, fuzz) {
    FuzzCanvas(fuzz, SkMakeNullCanvas().get());
}

constexpr SkISize kCanvasSize = {128, 160};

DEF_FUZZ(RasterN32Canvas, fuzz) {
    auto surface = SkSurfaces::Raster(
            SkImageInfo::MakeN32Premul(kCanvasSize.width(), kCanvasSize.height()));
    if (!surface || !surface->getCanvas()) { fuzz->signalBug(); }
    FuzzCanvas(fuzz, surface->getCanvas());
}

#if defined(SK_CODEC_DECODES_PNG_WITH_LIBPNG) && defined(SK_CODEC_ENCODES_PNG_WITH_LIBPNG)
#include "include/codec/SkPngDecoder.h"
#include "include/encode/SkPngEncoder.h"

DEF_FUZZ(RasterN32CanvasViaSerialization, fuzz) {
    SkPictureRecorder recorder;
    FuzzCanvas(fuzz, recorder.beginRecording(SkIntToScalar(kCanvasSize.width()),
                                              SkIntToScalar(kCanvasSize.height())));
    sk_sp<SkPicture> pic(recorder.finishRecordingAsPicture());
    if (!pic) { fuzz->signalBug(); }
    SkSerialProcs sProcs;
    sProcs.fImageProc = [](SkImage* img, void*) -> sk_sp<SkData> {
        return SkPngEncoder::Encode(nullptr, img, SkPngEncoder::Options{});
    };
    sk_sp<SkData> data = pic->serialize(&sProcs);
    if (!data) { fuzz->signalBug(); }
    SkReadBuffer rb(data->data(), data->size());
    SkCodecs::Register(SkPngDecoder::Decoder());
    auto deserialized = SkPicturePriv::MakeFromBuffer(rb);
    if (!deserialized) { fuzz->signalBug(); }
    auto surface = SkSurfaces::Raster(
            SkImageInfo::MakeN32Premul(kCanvasSize.width(), kCanvasSize.height()));
    SkASSERT(surface && surface->getCanvas());
    surface->getCanvas()->drawPicture(deserialized);
}

#endif

DEF_FUZZ(ImageFilter, fuzz) {
    SkBitmap bitmap;
    if (!bitmap.tryAllocN32Pixels(256, 256)) {
        SkDEBUGF("Could not allocate 256x256 bitmap in ImageFilter");
        return;
    }

    auto fil = MakeFuzzImageFilter(fuzz, 20);

    SkPaint paint;
    paint.setImageFilter(fil);
    SkCanvas canvas(bitmap);
    canvas.saveLayer(SkRect::MakeWH(500, 500), &paint);
}


//SkRandom _rand;
#define SK_ADD_RANDOM_BIT_FLIPS

DEF_FUZZ(SerializedImageFilter, fuzz) {
    SkBitmap bitmap;
    if (!bitmap.tryAllocN32Pixels(256, 256)) {
        SkDEBUGF("Could not allocate 256x256 bitmap in SerializedImageFilter");
        return;
    }

    auto filter = MakeFuzzImageFilter(fuzz, 20);
    if (!filter) {
        return;
    }
    auto data = filter->serialize();
    const unsigned char* ptr = static_cast<const unsigned char*>(data->data());
    size_t len = data->size();
#ifdef SK_ADD_RANDOM_BIT_FLIPS
    unsigned char* p = const_cast<unsigned char*>(ptr);
    for (size_t i = 0; i < len; ++i, ++p) {
        uint8_t j;
        fuzz->nextRange(&j, 1, 250);
        if (j == 1) { // 0.4% of the time, flip a bit or byte
            uint8_t k;
            fuzz->nextRange(&k, 1, 10);
            if (k == 1) { // Then 10% of the time, change a whole byte
                uint8_t s;
                fuzz->nextRange(&s, 0, 2);
                switch(s) {
                case 0:
                    *p ^= 0xFF; // Flip entire byte
                    break;
                case 1:
                    *p = 0xFF; // Set all bits to 1
                    break;
                case 2:
                    *p = 0x00; // Set all bits to 0
                    break;
                }
            } else {
                uint8_t s;
                fuzz->nextRange(&s, 0, 7);
                *p ^= (1 << 7);
            }
        }
    }
#endif // SK_ADD_RANDOM_BIT_FLIPS
    auto deserializedFil = SkImageFilter::Deserialize(ptr, len);

    // uncomment below to write out a serialized image filter (to make corpus
    // for -t filter_fuzz)
    // SkString s("./serialized_filters/sf");
    // s.appendU32(_rand.nextU());
    // auto file = sk_fopen(s.c_str(), SkFILE_Flags::kWrite_SkFILE_Flag);
    // sk_fwrite(data->bytes(), data->size(), file);
    // sk_fclose(file);

    SkPaint paint;
    paint.setImageFilter(deserializedFil);

    SkCanvas canvas(bitmap);
    canvas.saveLayer(SkRect::MakeWH(256, 256), &paint);
    canvas.restore();
}

#if defined(SK_GANESH)
static void fuzz_ganesh(Fuzz* fuzz, GrDirectContext* context) {
    SkASSERT(context);
    auto surface = SkSurfaces::RenderTarget(
            context,
            skgpu::Budgeted::kNo,
            SkImageInfo::Make(kCanvasSize, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    SkASSERT(surface && surface->getCanvas());
    FuzzCanvas(fuzz, surface->getCanvas());
}

DEF_FUZZ(MockGPUCanvas, fuzz) {
    sk_gpu_test::GrContextFactory f;
    fuzz_ganesh(fuzz, f.get(skgpu::ContextType::kMock));
}
#endif

#ifdef SK_GL
static void dump_GPU_info(GrDirectContext* context) {
    const GrGLInterface* gl = static_cast<GrGLGpu*>(context->priv().getGpu())
                                    ->glInterface();
    const GrGLubyte* output;
    GR_GL_CALL_RET(gl, output, GetString(GR_GL_RENDERER));
    SkDebugf("GL_RENDERER %s\n", (const char*) output);

    GR_GL_CALL_RET(gl, output, GetString(GR_GL_VENDOR));
    SkDebugf("GL_VENDOR %s\n", (const char*) output);

    GR_GL_CALL_RET(gl, output, GetString(GR_GL_VERSION));
    SkDebugf("GL_VERSION %s\n", (const char*) output);
}

DEF_FUZZ(NativeGLCanvas, fuzz) {
    sk_gpu_test::GrContextFactory f;
    auto context = f.get(skgpu::ContextType::kGL);
    if (!context) {
        context = f.get(skgpu::ContextType::kGLES);
    }
    if (FLAGS_gpuInfo) {
        dump_GPU_info(context);
    }
    fuzz_ganesh(fuzz, context);
}
#endif

DEF_FUZZ(PDFCanvas, fuzz) {
    SkNullWStream stream;
    auto doc = SkPDF::MakeDocument(&stream, SkPDF::JPEG::MetadataWithCallbacks());
    FuzzCanvas(fuzz, doc->beginPage(SkIntToScalar(kCanvasSize.width()),
                                     SkIntToScalar(kCanvasSize.height())));
}

// not a "real" thing to fuzz, used to debug errors found while fuzzing.
DEF_FUZZ(_DumpCanvas, fuzz) {
    DebugCanvas debugCanvas(kCanvasSize.width(), kCanvasSize.height());
    FuzzCanvas(fuzz, &debugCanvas);
    std::unique_ptr<SkCanvas> nullCanvas = SkMakeNullCanvas();
    UrlDataManager dataManager(SkString("data"));
    SkDynamicMemoryWStream stream;
    SkJSONWriter writer(&stream, SkJSONWriter::Mode::kPretty);
    writer.beginObject(); // root
    debugCanvas.toJSON(writer, dataManager, nullCanvas.get());
    writer.endObject(); // root
    writer.flush();
    sk_sp<SkData> json = stream.detachAsData();
    fwrite(json->data(), json->size(), 1, stdout);
}

DEF_FUZZ(SVGCanvas, fuzz) {
    SkNullWStream stream;
    SkRect bounds = SkRect::MakeIWH(150, 150);
    std::unique_ptr<SkCanvas> canvas = SkSVGCanvas::Make(bounds, &stream);
    FuzzCanvas(fuzz, canvas.get());
}
