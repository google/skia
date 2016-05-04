/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <stdio.h>

#include "SkBitmap.h"
#include "SkCGUtils.h"
#include "SkForceLinking.h"
#include "SkImageEncoder.h"
#include "SkStream.h"

__SK_FORCE_IMAGE_DECODER_LINKING;

class StdOutWStream : public SkWStream {
public:
    StdOutWStream() : fBytesWritten(0) {}
    bool write(const void* buffer, size_t size) final {
        fBytesWritten += size;
        return size == fwrite(buffer, 1, size, stdout);
    }
    size_t bytesWritten() const final { return fBytesWritten; }

private:
    size_t fBytesWritten;
};

static SkStreamAsset* open_for_reading(const char* path) {
    if (!path || !path[0] || 0 == strcmp(path, "-")) {
        return new SkFILEStream(stdin, SkFILEStream::kCallerRetains_Ownership);
    }
    return SkStream::NewFromFile(path);
}

static SkWStream* open_for_writing(const char* path) {
    if (!path || !path[0] || 0 == strcmp(path, "-")) {
        return new StdOutWStream;
    }
    return new SkFILEWStream(path);
}

static bool to_png(SkWStream* o, const SkBitmap& bm) {
    return SkImageEncoder::EncodeStream(o, bm, SkImageEncoder::kPNG_Type, 100);
}

// Note: I could implement this using only MacOS|CG API calls, but
// since most of this is already done in Skia, here it is.
int main(int argc, char** argv) {
    SkBitmap bm;
    SkAutoTDelete<SkStream> in(open_for_reading(argc > 1 ? argv[1] : NULL));
    SkAutoTDelete<SkWStream> out(open_for_writing(argc > 2 ? argv[2] : NULL));
    if (SkPDFDocumentToBitmap(in.release(), &bm) && to_png(out, bm)) {
        return 0;
    } else {
        return 1;
    }
}
