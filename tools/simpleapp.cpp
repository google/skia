/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"

#include "include/docs/SkPDFDocument.h"
static void invoke_pdf() {
    SkPDF::MakeDocument(nullptr);
}

#include "modules/skottie/include/Skottie.h"
static void invoke_skottie() {
    skottie::Animation::Make(nullptr);
}

int main(int argc, char** argv) {
    if (false) {
        invoke_pdf();
    }
    if (argc < 0) {
        invoke_skottie();
    }

    auto info = SkImageInfo::MakeN32Premul(100, 100);
    auto surf = SkSurface::MakeRaster(info);
    auto canvas = surf->getCanvas();

    SkPaint paint;
    canvas->drawRect({10,10,100,100}, paint);

    auto img = surf->makeImageSnapshot();
    return 0;
}
