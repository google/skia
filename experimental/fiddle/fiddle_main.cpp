/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <stdio.h>

#include <GL/osmesa.h>

#include "fiddle_main.h"

// Globals externed in fiddle_main.h
SkBitmap source;
SkImage* image(nullptr);

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

static void dump_output(SkData* data, const char* name, bool last = true) {
    if (data) {
        printf("\t\"%s\": \"", name);
        encode_to_base64(data->data(), data->size(), stdout);
        fputs(last ? "\"\n" : "\",\n", stdout);
    }
}

static SkData* encode_snapshot(SkSurface* surface) {
    SkAutoTUnref<SkImage> img(surface->newImageSnapshot());
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

static GrContext* create_mesa_grcontext() {
    SkAutoTUnref<const GrGLInterface> mesa(GrGLCreateMesaInterface());
    intptr_t backend = reinterpret_cast<intptr_t>(mesa.get());
    return backend ? GrContext::Create(kOpenGL_GrBackend, backend) : nullptr;
}


int main() {
    const DrawOptions options = GetDrawOptions();
    fprintf(stderr, "%s\n", options.source);
    if (options.source) {
        SkAutoTUnref<SkData> data(SkData::NewFromFileName(options.source));
        if (!data) {
            perror(options.source);
            return 1;
        } else {
            image = SkImage::NewFromEncoded(data);
            if (!image) {
                perror("Unable to decode the source image.");
                return 1;
            }
            SkAssertResult(image->asLegacyBitmap(
                                   &source, SkImage::kRO_LegacyBitmapMode));
        }
    }
    SkAutoTUnref<SkData> rasterData, gpuData, pdfData, skpData;
    if (options.raster) {
        SkAutoTUnref<SkSurface> rasterSurface(
                SkSurface::NewRaster(SkImageInfo::MakeN32Premul(options.size)));
        draw(rasterSurface->getCanvas());
        rasterData.reset(encode_snapshot(rasterSurface));
    }
    if (options.gpu) {
        OSMesaContext osMesaContext = create_osmesa_context();
        SkAutoTUnref<GrContext> grContext(create_mesa_grcontext());
        if (!grContext) {
            fputs("Unable to get Mesa GrContext.\n", stderr);
        } else {
            SkAutoTUnref<SkSurface> surface(
                    SkSurface::NewRenderTarget(
                            grContext,
                            SkSurface::kNo_Budgeted,
                            SkImageInfo::MakeN32Premul(options.size)));
            if (!surface) {
                fputs("Unable to get render surface.\n", stderr);
                exit(1);
            }
            draw(surface->getCanvas());
            gpuData.reset(encode_snapshot(surface));
        }
        if (osMesaContext) {
            OSMesaDestroyContext(osMesaContext);
        }
    }
    if (options.pdf) {
        SkDynamicMemoryWStream pdfStream;
        SkAutoTUnref<SkDocument> document(SkDocument::CreatePDF(&pdfStream));
        draw(document->beginPage(options.size.width(), options.size.height()));
        document->close();
        pdfData.reset(pdfStream.copyToData());
    }
    if (options.skp) {
        SkSize size;
        size = options.size;
        SkPictureRecorder recorder;
        draw(recorder.beginRecording(size.width(), size.height()));
        SkAutoTUnref<SkPicture> picture(recorder.endRecordingAsPicture());
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

    SkSafeSetNull(image);
    return 0;
}
