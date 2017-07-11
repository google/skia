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

#include "SkCommandLineFlags.h"

#include "fiddle_main.h"

DEFINE_double(duration, 1.0, "The total duration, in seconds, of the animation we are drawing.");
DEFINE_double(frame, 1.0, "A double value in [0, 1] that specifies the point in animation to draw.");

// Globals externed in fiddle_main.h
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

int main(int argc, char** argv) {
    SkCommandLineFlags::Parse(argc, argv);
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
            SkAssertResult(image->asLegacyBitmap(
                                   &source, SkImage::kRO_LegacyBitmapMode));
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
        auto grContext = create_grcontext(gGLDriverInfo);
        if (!grContext) {
            fputs("Unable to get GrContext.\n", stderr);
        } else {
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
        sk_sp<SkDocument> document(SkDocument::MakePDF(&pdfStream));
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
