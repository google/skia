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

#include "CommandLineFlags.h"
#include "SkAutoPixmapStorage.h"
#include "SkMipMap.h"
#include "SkUtils.h"

#include "fiddle_main.h"

DEFINE_double(duration, 1.0, "The total duration, in seconds, of the animation we are drawing.");
DEFINE_double(frame, 1.0, "A double value in [0, 1] that specifies the point in animation to draw.");

#include "GrBackendSurface.h"
#include "GrContextPriv.h"
#include "GrGpu.h"
#include "gl/GLTestContext.h"

// Globals externed in fiddle_main.h
sk_sp<GrTexture>      backingTexture;  // not externed
GrBackendTexture      backEndTexture;

sk_sp<GrRenderTarget> backingRenderTarget; // not externed
GrBackendRenderTarget backEndRenderTarget;

sk_sp<GrTexture>      backingTextureRenderTarget;  // not externed
GrBackendTexture      backEndTextureRenderTarget;

SkBitmap source;
sk_sp<SkImage> image;
double duration; // The total duration of the animation in seconds.
double frame;    // A value in [0, 1] of where we are in the animation.

// Global used by the local impl of SkDebugf.
std::ostringstream gTextOutput;

// Global to record the GL driver info via create_grcontext().
std::ostringstream gGLDriverInfo;

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

static sk_sp<SkData> encode_snapshot(const sk_sp<SkSurface>& surface) {
    sk_sp<SkImage> img(surface->makeImageSnapshot());
    return img ? img->encodeToData() : nullptr;
}

static SkCanvas* prepare_canvas(SkCanvas * canvas) {
    canvas->clear(SK_ColorWHITE);
    return canvas;
}

static bool setup_backend_objects(GrContext* context,
                                  const SkBitmap& bm,
                                  const DrawOptions& options) {
    if (!context) {
        return false;
    }

    auto resourceProvider = context->priv().resourceProvider();

    GrSurfaceDesc backingDesc;
    backingDesc.fFlags = kNone_GrSurfaceFlags;
    backingDesc.fWidth = bm.width();
    backingDesc.fHeight = bm.height();
    // This config must match the SkColorType used in draw.cpp in the SkImage and Surface factories
    backingDesc.fConfig = kRGBA_8888_GrPixelConfig;
    backingDesc.fSampleCnt = 1;

    if (!bm.empty()) {
        SkPixmap originalPixmap;
        SkPixmap* pixmap = &originalPixmap;
        if (!bm.peekPixels(&originalPixmap)) {
            return false;
        }

        SkAutoPixmapStorage rgbaPixmap;
        if (kN32_SkColorType != kRGBA_8888_SkColorType) {
            if (!rgbaPixmap.tryAlloc(bm.info().makeColorType(kRGBA_8888_SkColorType))) {
                return false;
            }
            if (!bm.readPixels(rgbaPixmap)) {
                return false;
            }
            pixmap = &rgbaPixmap;
        }
        int mipLevelCount = GrMipMapped::kYes == options.fMipMapping
                                    ? SkMipMap::ComputeLevelCount(bm.width(), bm.height())
                                    : 1;
        std::unique_ptr<GrMipLevel[]> texels(new GrMipLevel[mipLevelCount]);

        texels[0].fPixels = pixmap->addr();
        texels[0].fRowBytes = pixmap->rowBytes();

        for (int i = 1; i < mipLevelCount; i++) {
            texels[i].fPixels = nullptr;
            texels[i].fRowBytes = 0;
        }

        backingTexture = resourceProvider->createTexture(backingDesc, SkBudgeted::kNo, texels.get(),
                                                         mipLevelCount);
        if (!backingTexture) {
            return false;
        }

        backEndTexture = backingTexture->getBackendTexture();
        if (!backEndTexture.isValid()) {
            return false;
        }
    }

    backingDesc.fFlags = kRenderTarget_GrSurfaceFlag;
    backingDesc.fWidth = options.fOffScreenWidth;
    backingDesc.fHeight = options.fOffScreenHeight;
    backingDesc.fSampleCnt = options.fOffScreenSampleCount;

    SkAutoTMalloc<uint32_t> data(backingDesc.fWidth * backingDesc.fHeight);
    sk_memset32(data.get(), 0, backingDesc.fWidth * backingDesc.fHeight);


    {
        // This backend object should be renderable but not textureable. Given the limitations
        // of how we're creating it though it will wind up being secretly textureable.
        // We use this fact to initialize it with data but don't allow mipmaps
        GrMipLevel level0 = { data.get(), backingDesc.fWidth*sizeof(uint32_t) };

        sk_sp<GrTexture> tmp = resourceProvider->createTexture(backingDesc, SkBudgeted::kNo,
                                                               &level0, 1);
        if (!tmp || !tmp->asRenderTarget()) {
            return false;
        }

        backingRenderTarget = sk_ref_sp(tmp->asRenderTarget());

        backEndRenderTarget = backingRenderTarget->getBackendRenderTarget();
        if (!backEndRenderTarget.isValid()) {
            return false;
        }
    }

    {
        int mipLevelCount = GrMipMapped::kYes == options.fOffScreenMipMapping
                            ? SkMipMap::ComputeLevelCount(backingDesc.fWidth, backingDesc.fHeight)
                            : 1;
        std::unique_ptr<GrMipLevel[]> texels(new GrMipLevel[mipLevelCount]);

        texels[0].fPixels = data.get();
        texels[0].fRowBytes = backingDesc.fWidth*sizeof(uint32_t);

        for (int i = 1; i < mipLevelCount; i++) {
            texels[i].fPixels = nullptr;
            texels[i].fRowBytes = 0;
        }

        backingTextureRenderTarget = resourceProvider->createTexture(backingDesc, SkBudgeted::kNo,
                                                                     texels.get(), mipLevelCount);
        if (!backingTextureRenderTarget || !backingTextureRenderTarget->asRenderTarget()) {
            return false;
        }

        backEndTextureRenderTarget = backingTextureRenderTarget->getBackendTexture();
        if (!backEndTextureRenderTarget.isValid()) {
            return false;
        }
    }


    return true;
}

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
    if (options.source) {
        sk_sp<SkData> data(SkData::MakeFromFileName(options.source));
        if (!data) {
            perror(options.source);
            return 1;
        } else {
            image = SkImage::MakeFromEncoded(std::move(data));
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
        auto rasterSurface = SkSurface::MakeRaster(info);
        srand(0);
        draw(prepare_canvas(rasterSurface->getCanvas()));
        rasterData = encode_snapshot(rasterSurface);
    }
    if (options.gpu) {
        std::unique_ptr<sk_gpu_test::GLTestContext> glContext;
        sk_sp<GrContext> grContext = create_grcontext(gGLDriverInfo, &glContext);
        if (!grContext) {
            fputs("Unable to get GrContext.\n", stderr);
        } else {
            if (!setup_backend_objects(grContext.get(), source, options)) {
                fputs("Unable to create backend objects.\n", stderr);
                exit(1);
            }

            auto surface = SkSurface::MakeRenderTarget(grContext.get(), SkBudgeted::kNo, info);
            if (!surface) {
                fputs("Unable to get render surface.\n", stderr);
                exit(1);
            }
            srand(0);
            draw(prepare_canvas(surface->getCanvas()));
            gpuData = encode_snapshot(surface);
        }
    }
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
    if (options.skp) {
        SkSize size;
        size = options.size;
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
