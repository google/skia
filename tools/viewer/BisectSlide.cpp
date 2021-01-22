/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/viewer/BisectSlide.h"

#include "include/core/SkPicture.h"
#include "include/core/SkStream.h"
#include "src/utils/SkOSPath.h"

#include <utility>

#ifdef SK_XML
#include "modules/svg/include/SkSVGDOM.h"
#include "src/xml/SkDOM.h"
#endif

sk_sp<BisectSlide> BisectSlide::Create(const char filepath[]) {
    SkFILEStream stream(filepath);
    if (!stream.isValid()) {
        SkDebugf("BISECT: invalid input file at \"%s\"\n", filepath);
        return nullptr;
    }

    sk_sp<BisectSlide> bisect(new BisectSlide(filepath));
    if (bisect->fFilePath.endsWith(".svg")) {
#ifdef SK_XML
        sk_sp<SkSVGDOM> svg = SkSVGDOM::MakeFromStream(stream);
        if (!svg) {
            SkDebugf("BISECT: couldn't load svg at \"%s\"\n", filepath);
            return nullptr;
        }
        svg->setContainerSize(SkSize::Make(bisect->getDimensions()));
        svg->render(bisect.get());
#else
        return nullptr;
#endif
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

// Called through SkPicture::playback only during creation.
void BisectSlide::onDrawPath(const SkPath& path, const SkPaint& paint) {
    SkRect bounds;
    SkIRect ibounds;
    this->getTotalMatrix().mapRect(&bounds, path.getBounds());
    bounds.roundOut(&ibounds);
    fDrawBounds.join(ibounds);
    fFoundPaths.push_back() = {path, paint, this->getTotalMatrix()};
}

bool BisectSlide::onChar(SkUnichar c) {
    switch (c) {
        case 'X':
            if (!fTossedPaths.empty()) {
                using std::swap;
                swap(fFoundPaths, fTossedPaths);
                if ('X' == fTrail.back()) {
                    fTrail.pop_back();
                } else {
                    fTrail.push_back('X');
                }
            }
            return true;

        case 'x':
            if (fFoundPaths.count() > 1) {
                int midpt = (fFoundPaths.count() + 1) / 2;
                fPathHistory.emplace(fFoundPaths, fTossedPaths);
                fTossedPaths.reset(fFoundPaths.begin() + midpt, fFoundPaths.count() - midpt);
                fFoundPaths.resize_back(midpt);
                fTrail.push_back('x');
            }
            return true;

        case 'Z': {
            if (!fPathHistory.empty()) {
                fFoundPaths = fPathHistory.top().first;
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
            for (const FoundPath& foundPath : fFoundPaths) {
                foundPath.fPath.dump();
            }
            return true;
    }

    return false;
}

void BisectSlide::draw(SkCanvas* canvas) {
    SkAutoCanvasRestore acr(canvas, true);
    canvas->translate(-fDrawBounds.left(), -fDrawBounds.top());

    for (const FoundPath& path : fFoundPaths) {
        SkAutoCanvasRestore acr(canvas, true);
        canvas->concat(path.fViewMatrix);
        canvas->drawPath(path.fPath, path.fPaint);
    }
}
