/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "BisectSlide.h"

#include "SkDOM.h"
#include "SkOSPath.h"
#include "SkPicture.h"
#include "SkStream.h"
#include "../experimental/svg/model/SkSVGDOM.h"

sk_sp<BisectSlide> BisectSlide::Create(const char filepath[]) {
    SkFILEStream stream(filepath);
    if (!stream.isValid()) {
        SkDebugf("BISECT: invalid input file at \"%s\"\n", filepath);
        return nullptr;
    }

    sk_sp<BisectSlide> bisect(new BisectSlide(filepath));
    if (bisect->fFilePath.endsWith(".svg")) {
        SkDOM xml;
        if (!xml.build(stream)) {
            SkDebugf("BISECT: XML parsing failed: \"%s\"\n", filepath);
            return nullptr;
        }
        sk_sp<SkSVGDOM> svg = SkSVGDOM::MakeFromDOM(xml);
        if (!svg) {
            SkDebugf("BISECT: couldn't load svg at \"%s\"\n", filepath);
            return nullptr;
        }
        svg->setContainerSize(SkSize::Make(500, 500));
        svg->render(bisect.get());
    } else {
        sk_sp<SkPicture> skp = SkPicture::MakeFromStream(&stream);
        if (!skp) {
            SkDebugf("BISECT: couldn't load skp at \"%s\"\n", filepath);
            return nullptr;
        }
        skp->playback(bisect.get());
    }

    return bisect;
}

BisectSlide::BisectSlide(const char filepath[])
        : SkCanvas(4096, 4096, nullptr)
        , fFilePath(filepath) {
    const char* basename = strrchr(fFilePath.c_str(), SkOSPath::SEPARATOR);
    fName.printf("BISECT_%s", basename ? basename + 1 : fFilePath.c_str());
}

bool BisectSlide::onChar(SkUnichar c) {
    switch (c) {
        case 'X':
            if (!fTossedPaths.empty()) {
                SkTSwap(fPaths, fTossedPaths);
                if ('X' == fTrail.back()) {
                    fTrail.pop_back();
                } else {
                    fTrail.push_back('X');
                }
            }
            return true;
        case 'x':
            if (fPaths.count() > 1) {
                int midpt = (fPaths.count() + 1) / 2;
                fPathHistory.emplace(fPaths, fTossedPaths);
                fTossedPaths.reset(fPaths.begin() + midpt, fPaths.count() - midpt);
                fPaths.resize_back(midpt);
                fTrail.push_back('x');
            }
            return true;
        case 'Z': {
            if (!fPathHistory.empty()) {
                fPaths = fPathHistory.top().first;
                fTossedPaths = fPathHistory.top().second;
                fPathHistory.pop();
                char ch;
                do {
                    ch = fTrail.back();
                    fTrail.pop_back();
                } while (ch != 'x');
            }
            return true;
        }
        case 'D':
            SkDebugf("viewer --bisect %s", fFilePath.c_str());
            if (!fTrail.empty()) {
                SkDebugf(" ");
                for (char ch : fTrail) {
                    SkDebugf("%c", ch);
                }
            }
            SkDebugf("\n");
            for (const FoundPath& foundPath : fPaths) {
                foundPath.fPath.dump();
            }
            return true;
    }
    return false;
}

void BisectSlide::draw(SkCanvas* canvas) {
    for (const FoundPath& path : fPaths) {
        SkAutoCanvasRestore acr(canvas, true);
        canvas->concat(path.fViewMatrix);
        canvas->drawPath(path.fPath, path.fPaint);
    }
}
