/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/encode/SkPngEncoder.h"
#include "modules/skottie/include/Skottie.h"

#include <cstdio>

int main(int argc, char** argv) {
    if (argc != 2 && argc != 3) {
        std::printf("Usage: %s <lottie.json> [<nframes>]", argv[0]);
        return 1;
    }
    SkFILEStream input(argv[1]);
    if (!input.isValid()) {
        std::printf("Cannot open input file %s\n", argv[1]);
        return 1;
    }

    unsigned n = 1;
    if (argc == 3) {
        if (1 != std::sscanf(argv[2], "%u", &n)) {
            std::printf("Usage: %s <lottie.json> [<nframes>]", argv[0]);
            return 1;
        }
    }

    auto animation = skottie::Animation::Make(&input);
    if (!animation) {
        std::printf("Cannot parse input file %s\n", argv[1]);
        return 1;
    }
    SkISize surfaceSize = animation->size().toCeil();
    auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(surfaceSize, nullptr));
    if (!surface) {
        std::printf("Cannot allocate surface of size %d x %d\n",
                    surfaceSize.width(),
                    surfaceSize.height());
        return 1;
    }

    for (unsigned f = 0; f < n; ++f) {
        double t;
        SkString outFileName;
        if (n > 1) {
            t = static_cast<double>(f) / (n - 1) * animation->duration();
            outFileName = SkStringPrintf("%s.%u.png", argv[1], f);
        } else {
            t = 0.0;
            outFileName = SkStringPrintf("%s.png", argv[1]);
        }
        surface->getCanvas()->clear(SK_ColorWHITE);
        animation->seekFrameTime(t);
        animation->render(surface->getCanvas());

        SkFILEWStream out(outFileName.c_str());
        if (!out.isValid()) {
            std::printf("Cannot open output file %s\n", outFileName.c_str());
            return 1;
        }

        if (SkPixmap pm; !surface->peekPixels(&pm) || !SkPngEncoder::Encode(&out, pm, {})) {
            std::printf("Cannot encode rendering to PNG.\n");
            return 1;
        }
    }
    return 0;
}
