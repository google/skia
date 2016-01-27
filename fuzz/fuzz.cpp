/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Fuzz.h"
#include "SkCanvas.h"
#include "SkCodec.h"
#include "SkCommandLineFlags.h"
#include "SkData.h"
#include "SkForceLinking.h"
#include "SkImage.h"
#include "SkImageEncoder.h"
#include "SkMallocPixelRef.h"
#include "SkPicture.h"
#include "SkStream.h"

#include <signal.h>
#include <stdlib.h>

__SK_FORCE_IMAGE_DECODER_LINKING;

DEFINE_string2(bytes, b, "", "A path to a file.  This can be the fuzz bytes or a binary to parse.");
DEFINE_string2(name, n, "", "If --type is 'api', fuzz the API with this name.");

DEFINE_string2(type, t, "api", "How to interpret --bytes, either 'image', 'skp', or 'api'.");
DEFINE_string2(dump, d, "", "If not empty, dump 'image' or 'skp' types as a PNG with this name.");

static int printUsage(const char* name) {
    SkDebugf("Usage: %s -t <type> -b <path/to/file> [-n api-to-fuzz]\n", name);
    return 1;
}

static int fuzz_api(SkData*);
static int fuzz_img(SkData*);
static int fuzz_skp(SkData*);

int main(int argc, char** argv) {
    SkCommandLineFlags::Parse(argc, argv);

    const char* path = FLAGS_bytes.isEmpty() ? argv[0] : FLAGS_bytes[0];
    SkAutoTUnref<SkData> bytes(SkData::NewFromFileName(path));
    if (!bytes) {
        SkDebugf("Could not read %s\n", path);
        return 2;
    }

    if (!FLAGS_type.isEmpty()) {
        switch (FLAGS_type[0][0]) {
            case 'a': return fuzz_api(bytes);
            case 'i': return fuzz_img(bytes);
            case 's': return fuzz_skp(bytes);
        }
    }
    return printUsage(argv[0]);
}

int fuzz_api(SkData* bytes) {
    const char* name = FLAGS_name.isEmpty() ? "" : FLAGS_name[0];

    for (auto r = SkTRegistry<Fuzzable>::Head(); r; r = r->next()) {
        auto fuzzable = r->factory();
        if (0 == strcmp(name, fuzzable.name)) {
            SkDebugf("Fuzzing %s...\n", fuzzable.name);
            Fuzz fuzz(bytes);
            fuzzable.fn(&fuzz);
            SkDebugf("Success!");
            return 0;
        }
    }

    SkDebugf("When using --type api, please choose an API to fuzz with --name/-n:\n");
    for (auto r = SkTRegistry<Fuzzable>::Head(); r; r = r->next()) {
        auto fuzzable = r->factory();
        SkDebugf("\t%s\n", fuzzable.name);
    }
    return 1;
}

static void dump_png(SkBitmap bitmap) {
    if (!FLAGS_dump.isEmpty()) {
        SkImageEncoder::EncodeFile(FLAGS_dump[0], bitmap, SkImageEncoder::kPNG_Type, 100);
        SkDebugf("Dumped to %s\n", FLAGS_dump[0]);
    }
}

int fuzz_img(SkData* bytes) {
    SkAutoTDelete<SkCodec> codec(SkCodec::NewFromData(bytes));
    if (nullptr == codec.get()) {
        SkDebugf("Couldn't create codec.");
        return 3;
    }

    SkImageInfo decodeInfo = codec->getInfo();
    // Construct a color table for the decode if necessary
    SkAutoTUnref<SkColorTable> colorTable(nullptr);
    SkPMColor* colorPtr = nullptr;
    int* colorCountPtr = nullptr;
    int maxColors = 256;
    if (kIndex_8_SkColorType == decodeInfo.colorType()) {
        SkPMColor colors[256];
        colorTable.reset(new SkColorTable(colors, maxColors));
        colorPtr = const_cast<SkPMColor*>(colorTable->readColors());
        colorCountPtr = &maxColors;
    }

    SkBitmap bitmap;
    SkMallocPixelRef::ZeroedPRFactory zeroFactory;
    SkCodec::Options options;
    options.fZeroInitialized = SkCodec::kYes_ZeroInitialized;

    if (!bitmap.tryAllocPixels(decodeInfo, &zeroFactory, nullptr)) {
        SkDebugf("Could not allocate memory.  Image might be too large (%d x %d)",
                    decodeInfo.width(), decodeInfo.height());
        return 4;
    }

    switch (codec->getPixels(decodeInfo, bitmap.getPixels(), bitmap.rowBytes(), &options,
            colorPtr, colorCountPtr)) {
        case SkCodec::kSuccess:
            SkDebugf("Success!\n");
            break;
        case SkCodec::kIncompleteInput:
            SkDebugf("Partial Success\n");
            break;
        case SkCodec::kInvalidConversion:
            SkDebugf("Incompatible colortype conversion");
            return 5;
        default:
            // Everything else is considered a failure.
            SkDebugf("Couldn't getPixels.");
            return 6;
    }

    dump_png(bitmap);
    return 0;
}

int fuzz_skp(SkData* bytes) {
    SkMemoryStream stream(bytes);
    SkDebugf("Decoding\n");
    SkAutoTUnref<SkPicture> pic(SkPicture::CreateFromStream(&stream));
    if (!pic) {
        SkDebugf("Couldn't decode as a picture.\n");
        return 3;
    }
    SkDebugf("Rendering\n");
    SkBitmap bitmap;
    if (!FLAGS_dump.isEmpty()) {
        SkIRect size = pic->cullRect().roundOut();
        bitmap.allocN32Pixels(size.width(), size.height());
    }
    SkCanvas canvas(bitmap);
    canvas.drawPicture(pic);
    SkDebugf("Success! Decoded and rendered an SkPicture!\n");
    dump_png(bitmap);
    return 0;
}

Fuzz::Fuzz(SkData* bytes) : fBytes(SkSafeRef(bytes)), fNextByte(0) {}

void Fuzz::signalBug   () { raise(SIGSEGV); }
void Fuzz::signalBoring() { exit(0); }

template <typename T>
T Fuzz::nextT() {
    if (fNextByte + sizeof(T) > fBytes->size()) {
        this->signalBoring();
    }

    T val;
    memcpy(&val, fBytes->bytes() + fNextByte, sizeof(T));
    fNextByte += sizeof(T);
    return val;
}

uint8_t  Fuzz::nextB() { return this->nextT<uint8_t >(); }
uint32_t Fuzz::nextU() { return this->nextT<uint32_t>(); }
float    Fuzz::nextF() { return this->nextT<float   >(); }

