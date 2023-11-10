/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <string>

#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkMemset.h"
#include "src/core/SkMipmap.h"
#include "tools/flags/CommandLineFlags.h"

#include "tools/fiddle/fiddle_main.h"

static DEFINE_double(duration, 1.0,
                     "The total duration, in seconds, of the animation we are drawing.");
static DEFINE_double(frame, 1.0,
                     "A double value in [0, 1] that specifies the point in animation to draw.");

#include "include/encode/SkPngEncoder.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrRenderTarget.h"
#include "src/gpu/ganesh/GrResourceProvider.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "tools/gpu/ManagedBackendTexture.h"
#include "tools/gpu/gl/GLTestContext.h"

#if defined(SK_FONTMGR_FONTCONFIG_AVAILABLE)
#include "include/ports/SkFontMgr_fontconfig.h"
#endif

using namespace skia_private;

// Globals externed in fiddle_main.h
GrBackendTexture backEndTexture;
GrBackendRenderTarget backEndRenderTarget;
GrBackendTexture backEndTextureRenderTarget;
SkBitmap source;
sk_sp<SkImage> image;
double duration; // The total duration of the animation in seconds.
double frame;    // A value in [0, 1] of where we are in the animation.
sk_sp<SkFontMgr> fontMgr;

// Global used by the local impl of SkDebugf.
std::ostringstream gTextOutput;

// Global to record the GL driver info via create_direct_context().
std::ostringstream gGLDriverInfo;

sk_sp<sk_gpu_test::ManagedBackendTexture> managedBackendTextureRenderTarget;
sk_sp<sk_gpu_test::ManagedBackendTexture> managedBackendTexture;
sk_sp<GrRenderTarget> backingRenderTarget;

void SkDebugf(const char * fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char formatbuffer[1024];
    int n = vsnprintf(formatbuffer, sizeof(formatbuffer), fmt, args);
    va_end(args);
    if (n>=0 && n<=int(sizeof(formatbuffer))) {
        gTextOutput.write(formatbuffer, n);
    }
}

static void encode_to_base64(const void* data, size_t size, FILE* out) {
    const uint8_t* input = reinterpret_cast<const uint8_t*>(data);
    const uint8_t* end = &input[size];
    static const char codes[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz0123456789+/";
    while (input != end) {
        uint8_t b = (*input & 0xFC) >> 2;
        fputc(codes[b], out);
        b = (*input & 0x03) << 4;
        ++input;
        if (input == end) {
            fputc(codes[b], out);
            fputs("==", out);
            return;
        }
        b |= (*input & 0xF0) >> 4;
        fputc(codes[b], out);
        b = (*input & 0x0F) << 2;
        ++input;
        if (input == end) {
            fputc(codes[b], out);
            fputc('=', out);
            return;
        }
        b |= (*input & 0xC0) >> 6;
        fputc(codes[b], out);
        b = *input & 0x3F;
        fputc(codes[b], out);
        ++input;
    }
}


static void dump_output(const void* data, size_t size,
                        const char* name, bool last = true) {
    printf("\t\"%s\": \"", name);
    encode_to_base64(data, size, stdout);
    fputs(last ? "\"\n" : "\",\n", stdout);
}

static void dump_output(const sk_sp<SkData>& data,
                        const char* name, bool last = true) {
    if (data) {
        dump_output(data->data(), data->size(), name, last);
    }
}

static sk_sp<SkData> encode_snapshot(GrDirectContext* ctx, const sk_sp<SkSurface>& surface) {
    sk_sp<SkImage> img(surface->makeImageSnapshot());
    return SkPngEncoder::Encode(ctx, img.get(), {});
}

static SkCanvas* prepare_canvas(SkCanvas * canvas) {
    canvas->clear(SK_ColorWHITE);
    return canvas;
}

#ifdef SK_GL
static bool setup_backend_objects(GrDirectContext* dContext,
                                  const SkBitmap& bm,
                                  const DrawOptions& options) {
    if (!dContext) {
        fputs("Context is null.\n", stderr);
        return false;
    }

    // This config must match the SkColorType used in draw.cpp in the SkImage and Surface factories
    GrBackendFormat renderableFormat = dContext->defaultBackendFormat(kRGBA_8888_SkColorType,
                                                                      GrRenderable::kYes);

    if (!bm.empty()) {
        SkPixmap originalPixmap;
        SkPixmap* pixmap = &originalPixmap;
        if (!bm.peekPixels(&originalPixmap)) {
            fputs("Unable to peekPixels.\n", stderr);
            return false;
        }

        SkAutoPixmapStorage rgbaPixmap;
        constexpr bool kRGBAIsNative = kN32_SkColorType == kRGBA_8888_SkColorType;
        if ((!kRGBAIsNative)) {
            if (!rgbaPixmap.tryAlloc(bm.info().makeColorType(kRGBA_8888_SkColorType))) {
                fputs("Unable to alloc rgbaPixmap.\n", stderr);
                return false;
            }
            if (!bm.readPixels(rgbaPixmap)) {
                fputs("Unable to read rgbaPixmap.\n", stderr);
                return false;
            }
            pixmap = &rgbaPixmap;
        }

        managedBackendTexture = sk_gpu_test::ManagedBackendTexture::MakeFromPixmap(
                dContext,
                *pixmap,
                options.fMipMapping,
                GrRenderable::kNo,
                GrProtected::kNo);
        if (!managedBackendTexture) {
            fputs("Failed to create backEndTexture.\n", stderr);
            return false;
        }
        backEndTexture = managedBackendTexture->texture();
    }

    {
        auto resourceProvider = dContext->priv().resourceProvider();

        SkISize offscreenDims = {options.fOffScreenWidth, options.fOffScreenHeight};
        AutoTMalloc<uint32_t> data(offscreenDims.area());
        SkOpts::memset32(data.get(), 0, offscreenDims.area());

        // This backend object should be renderable but not textureable. Given the limitations
        // of how we're creating it though it will wind up being secretly textureable.
        // We use this fact to initialize it with data but don't allow mipmaps
        GrMipLevel level0 = {data.get(), offscreenDims.width()*sizeof(uint32_t), nullptr};

        constexpr int kSampleCnt = 1;
        sk_sp<GrTexture> tmp =
                resourceProvider->createTexture(offscreenDims,
                                                renderableFormat,
                                                GrTextureType::k2D,
                                                GrColorType::kRGBA_8888,
                                                GrRenderable::kYes,
                                                kSampleCnt,
                                                skgpu::Budgeted::kNo,
                                                skgpu::Mipmapped::kNo,
                                                GrProtected::kNo,
                                                &level0,
                                                /*label=*/"Fiddle_SetupBackendObjects");
        if (!tmp || !tmp->asRenderTarget()) {
            fputs("GrTexture is invalid.\n", stderr);
            return false;
        }

        backingRenderTarget = sk_ref_sp(tmp->asRenderTarget());

        backEndRenderTarget = backingRenderTarget->getBackendRenderTarget();
        if (!backEndRenderTarget.isValid()) {
            fputs("BackEndRenderTarget is invalid.\n", stderr);
            return false;
        }
    }

    {
        managedBackendTextureRenderTarget = sk_gpu_test::ManagedBackendTexture::MakeWithData(
            dContext,
            options.fOffScreenWidth,
            options.fOffScreenHeight,
            renderableFormat,
            SkColors::kTransparent,
            options.fOffScreenMipMapping,
            GrRenderable::kYes,
            GrProtected::kNo);
        if (!managedBackendTextureRenderTarget) {
            fputs("Failed to create backendTextureRenderTarget.\n", stderr);
            return false;
        }
        backEndTextureRenderTarget = managedBackendTextureRenderTarget->texture();
    }

    return true;
}
#endif

int main(int argc, char** argv) {
    CommandLineFlags::Parse(argc, argv);
    duration = FLAGS_duration;
    frame = FLAGS_frame;
    DrawOptions options = GetDrawOptions();
    // If textOnly then only do one type of image, otherwise the text
    // output is duplicated for each type.
    if (options.textOnly) {
        options.raster = true;
        options.gpu = false;
        options.pdf = false;
        options.skp = false;
    }
#if defined(SK_FONTMGR_FONTCONFIG_AVAILABLE)
    fontMgr = SkFontMgr_New_FontConfig(nullptr);
#else
    fontMgr = SkFontMgr::RefEmpty();
#endif
    if (options.source) {
        sk_sp<SkData> data(SkData::MakeFromFileName(options.source));
        if (!data) {
            perror(options.source);
            return 1;
        } else {
            image = SkImages::DeferredFromEncodedData(std::move(data));
            if (!image) {
                perror("Unable to decode the source image.");
                return 1;
            }
            SkAssertResult(image->asLegacyBitmap(&source));
        }
    }
    sk_sp<SkData> rasterData, gpuData, pdfData, skpData;
    SkColorType colorType = kN32_SkColorType;
    sk_sp<SkColorSpace> colorSpace = nullptr;
    if (options.f16) {
        SkASSERT(options.srgb);
        colorType = kRGBA_F16_SkColorType;
        colorSpace = SkColorSpace::MakeSRGBLinear();
    } else if (options.srgb) {
        colorSpace = SkColorSpace::MakeSRGB();
    }
    SkImageInfo info = SkImageInfo::Make(options.size.width(), options.size.height(), colorType,
                                         kPremul_SkAlphaType, colorSpace);
    if (options.raster) {
        auto rasterSurface = SkSurfaces::Raster(info);
        srand(0);
        draw(prepare_canvas(rasterSurface->getCanvas()));
        rasterData = encode_snapshot(nullptr, rasterSurface);
    }
#ifdef SK_GL
    if (options.gpu) {
        std::unique_ptr<sk_gpu_test::GLTestContext> glContext;
        sk_sp<GrDirectContext> direct = create_direct_context(gGLDriverInfo, &glContext);
        if (!direct) {
            fputs("Unable to get GrContext.\n", stderr);
        } else {
            if (!setup_backend_objects(direct.get(), source, options)) {
                fputs("Unable to create backend objects.\n", stderr);
                exit(1);
            }

            auto surface = SkSurfaces::RenderTarget(direct.get(), skgpu::Budgeted::kNo, info);
            if (!surface) {
                fputs("Unable to get render surface.\n", stderr);
                exit(1);
            }
            srand(0);
            draw(prepare_canvas(surface->getCanvas()));
            gpuData = encode_snapshot(direct.get(), surface);
        }
    }
#endif

#ifdef SK_SUPPORT_PDF
    if (options.pdf) {
        SkDynamicMemoryWStream pdfStream;
        auto document = SkPDF::MakeDocument(&pdfStream);
        if (document) {
            srand(0);
            draw(prepare_canvas(document->beginPage(options.size.width(), options.size.height())));
            document->close();
            pdfData = pdfStream.detachAsData();
        }
    }
#endif

    if (options.skp) {
        auto size = SkSize::Make(options.size);
        SkPictureRecorder recorder;
        srand(0);
        draw(prepare_canvas(recorder.beginRecording(size.width(), size.height())));
        auto picture = recorder.finishRecordingAsPicture();
        SkDynamicMemoryWStream skpStream;
        picture->serialize(&skpStream);
        skpData = skpStream.detachAsData();
    }

    printf("{\n");
    if (!options.textOnly) {
        dump_output(rasterData, "Raster", false);
        dump_output(gpuData, "Gpu", false);
        dump_output(pdfData, "Pdf", false);
        dump_output(skpData, "Skp", false);
    } else {
        std::string textoutput = gTextOutput.str();
        dump_output(textoutput.c_str(), textoutput.length(), "Text", false);
    }
    std::string glinfo = gGLDriverInfo.str();
    dump_output(glinfo.c_str(), glinfo.length(), "GLInfo", true);
    printf("}\n");

    return 0;
}
