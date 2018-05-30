/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cstdio>

#include "SkCanvas.h"
#include "SkDocument.h"
#include "SkImage.h"

int main(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage:\n  %s IN.JPG OUT.PDF\n\n", argv[0]);
        return 1;
    }
    const char* in_path = argv[1];
    const char* out_path = argv[2];
    auto image = SkImage::MakeFromEncoded(SkData::MakeFromFileName(in_path));
    if (!image) {
        fprintf(stderr, "Unable to decode file '%s'.\n", in_path);
        return 2;
    }
    SkFILEWStream outStream(out_path);
    if (!outStream.isValid()) {
        fprintf(stderr, "Unable to open file '%s'.\n", out_path);
        return 3;
    }
    SkTime::DateTime now;
    SkTime::GetDateTime(&now);
    SkDocument::PDFMetadata opts;
    opts.fCreation.fDateTime = now;
    opts.fCreation.fEnabled = true;
    opts.fModified.fDateTime = now;
    opts.fModified.fEnabled = true;
    opts.fCreator = "Skia/jpeg_to_pdf";
    auto doc = SkDocument::MakePDF(&outStream, opts);
    if (!doc) {
        fprintf(stderr, "SkPDF module missing.\n");
        return 4;
    }
    doc->beginPage(image->width(), image->height())->drawImage(image.get(), 0,0);
    return 0;
}
