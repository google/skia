/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>
#include "SkForceLinking.h"

__SK_FORCE_IMAGE_DECODER_LINKING;

#include "fiddle_main.h"

// Globals externed in fiddle_main.h
SkBitmap source;
sk_sp<SkImage> image;

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

static void dump_output(const sk_sp<SkData>& data,
                        const char* name, bool last = true) {
    if (data) {
        printf("\t\"%s\": \"", name);
        encode_to_base64(data->data(), data->size(), stdout);
        fputs(last ? "\"\n" : "\",\n", stdout);
    }
}

static SkData* encode_snapshot(const sk_sp<SkSurface>& surface) {
    sk_sp<SkImage> img(surface->makeImageSnapshot());
    return img ? img->encode() : nullptr;
}

#if defined(__linux) && !defined(__ANDROID__)
    #include <GL/osmesa.h>
    static sk_sp<GrContext> create_grcontext() {
        // We just leak the OSMesaContext... the process will die soon anyway.
        if (OSMesaContext osMesaContext = OSMesaCreateContextExt(OSMESA_BGRA, 0, 0, 0, nullptr)) {
            static uint32_t buffer[16 * 16];
            OSMesaMakeCurrent(osMesaContext, &buffer, GL_UNSIGNED_BYTE, 16, 16);
        }

        auto osmesa_get = [](void* ctx, const char name[]) {
            SkASSERT(nullptr == ctx);
            SkASSERT(OSMesaGetCurrentContext());
            return OSMesaGetProcAddress(name);
        };
        sk_sp<const GrGLInterface> mesa(GrGLAssembleInterface(nullptr, osmesa_get));
        if (!mesa) {
            return nullptr;
        }
        return sk_sp<GrContext>(GrContext::Create(
                                        kOpenGL_GrBackend,
                                        reinterpret_cast<intptr_t>(mesa.get())));
    }
#else
    static sk_sp<GrContext> create_grcontext() { return nullptr; }
#endif

sk_sp<SkData> draw_raster(const SkImageInfo& info) {
    auto rasterSurface = SkSurface::MakeRaster(info);
    srand(0);
    draw(rasterSurface->getCanvas());
    return sk_sp<SkData>(encode_snapshot(rasterSurface));
}

sk_sp<SkData> draw_gpu(const SkImageInfo& info) {
    auto grContext = create_grcontext();
    if (!grContext) {
        fputs("Unable to get GrContext.\n", stderr);
        return nullptr;
    } else {
        auto surface = SkSurface::MakeRenderTarget(
                grContext.get(),
                SkBudgeted::kNo,
                info);
        if (!surface) {
            fputs("Unable to get render surface.\n", stderr);
            exit(1);
        }
        srand(0);
        draw(surface->getCanvas());
        return sk_sp<SkData>(encode_snapshot(surface));
    }
}

int main() {
    const DrawOptions options = GetDrawOptions();
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
            SkAssertResult(image->asLegacyBitmap(
                                   &source, SkImage::kRO_LegacyBitmapMode));
        }
    }
    sk_sp<SkData> rasterData, gpuData, pdfData, skpData, srgbData, gpusrgbData, f16Data;
    if (options.raster) {
        rasterData = draw_raster(SkImageInfo::MakeN32Premul(options.size));
    }
    if (options.gpu) {
        gpuData = draw_gpu(SkImageInfo::MakeN32Premul(options.size));
    }
    if (options.pdf) {
        SkDynamicMemoryWStream pdfStream;
        sk_sp<SkDocument> document(SkDocument::MakePDF(&pdfStream));
        if (document) {
            srand(0);
            draw(document->beginPage(options.size.width(), options.size.height()));
            document->close();
            pdfData = pdfStream.detachAsData();
        }
    }
    if (options.skp) {
        SkSize size;
        size = options.size;
        SkPictureRecorder recorder;
        srand(0);
        draw(recorder.beginRecording(size.width(), size.height()));
        auto picture = recorder.finishRecordingAsPicture();
        SkDynamicMemoryWStream skpStream;
        picture->serialize(&skpStream);
        skpData = skpStream.detachAsData();
    }

    SkImageInfo srgbInfo =
            SkImageInfo::MakeS32(options.size.width(), options.size.height(), kPremul_SkAlphaType);
    if (options.srgb) {
        srgbData = draw_raster(srgbInfo);
    }
    if (options.gpusrgb) {
        gpusrgbData = draw_gpu(srgbInfo);
    }
    if (options.f16) {
        SkImageInfo f16Info = SkImageInfo::Make(options.size.width(), options.size.height(),
                kRGBA_F16_SkColorType, kPremul_SkAlphaType,
                SkColorSpace::MakeNamed(SkColorSpace::kSRGBLinear_Named));
        f16Data = draw_raster(f16Info);
    }

    printf("{\n");
    dump_output(rasterData, "Raster", !gpuData && !pdfData && !skpData);
    dump_output(gpuData, "Gpu", !pdfData && !skpData);
    dump_output(pdfData, "Pdf", !skpData);
    dump_output(skpData, "Skp");
    printf("}\n");

    return 0;
}
