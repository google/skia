/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>

#include <GL/osmesa.h>

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

static OSMesaContext create_osmesa_context() {
    OSMesaContext osMesaContext =
        OSMesaCreateContextExt(OSMESA_BGRA, 0, 0, 0, nullptr);
    if (osMesaContext != nullptr) {
        static uint32_t buffer[16 * 16];
        OSMesaMakeCurrent(osMesaContext, &buffer, GL_UNSIGNED_BYTE, 16, 16);
    }
    return osMesaContext;
}

static sk_sp<GrContext> create_mesa_grcontext() {
    if (nullptr == OSMesaGetCurrentContext()) {
        return nullptr;
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

int main() {
    const DrawOptions options = GetDrawOptions();
    if (options.source) {
        sk_sp<SkData> data(SkData::NewFromFileName(options.source));
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
    sk_sp<SkData> rasterData, gpuData, pdfData, skpData;
    if (options.raster) {
        auto rasterSurface =
                SkSurface::MakeRaster(SkImageInfo::MakeN32Premul(options.size));
        srand(0);
        draw(rasterSurface->getCanvas());
        rasterData.reset(encode_snapshot(rasterSurface));
    }
    if (options.gpu) {
        OSMesaContext osMesaContext = create_osmesa_context();
        auto grContext = create_mesa_grcontext();
        if (!grContext) {
            fputs("Unable to get Mesa GrContext.\n", stderr);
        } else {
            auto surface = SkSurface::MakeRenderTarget(
                    grContext.get(),
                    SkBudgeted::kNo,
                    SkImageInfo::MakeN32Premul(options.size));
            if (!surface) {
                fputs("Unable to get render surface.\n", stderr);
                exit(1);
            }
            srand(0);
            draw(surface->getCanvas());
            gpuData.reset(encode_snapshot(surface));
        }
        if (osMesaContext) {
            OSMesaDestroyContext(osMesaContext);
        }
    }
    if (options.pdf) {
        SkDynamicMemoryWStream pdfStream;
        sk_sp<SkDocument> document(SkDocument::MakePDF(&pdfStream));
        srand(0);
        draw(document->beginPage(options.size.width(), options.size.height()));
        document->close();
        pdfData.reset(pdfStream.copyToData());
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
        skpData.reset(skpStream.copyToData());
    }

    printf("{\n");
    dump_output(rasterData, "Raster", !gpuData && !pdfData && !skpData);
    dump_output(gpuData, "Gpu", !pdfData && !skpData);
    dump_output(pdfData, "Pdf", !skpData);
    dump_output(skpData, "Skp");
    printf("}\n");

    return 0;
}
