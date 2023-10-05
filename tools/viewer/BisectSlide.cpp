/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/viewer/BisectSlide.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkStream.h"
#include "include/private/base/SkDebug.h"
#include "src/utils/SkOSPath.h"
#include "tools/ToolUtils.h"

#include <string>
#include <functional>
#include <utility>

sk_sp<BisectSlide> BisectSlide::Create(const char filepath[]) {
    SkFILEStream stream(filepath);
    if (!stream.isValid()) {
        SkDebugf("BISECT: invalid input file at \"%s\"\n", filepath);
        return nullptr;
    }

    sk_sp<BisectSlide> bisect(new BisectSlide(filepath));
    ToolUtils::ExtractPathsFromSKP(filepath, [&](const SkMatrix& matrix,
                                                 const SkPath& path,
                                                 const SkPaint& paint) {
        SkRect bounds;
        SkIRect ibounds;
        matrix.mapRect(&bounds, path.getBounds());
        bounds.roundOut(&ibounds);
        bisect->fDrawBounds.join(ibounds);
        bisect->fFoundPaths.push_back() = {path, paint, matrix};
    });
    return bisect;
}

BisectSlide::BisectSlide(const char filepath[])
        : fFilePath(filepath) {
    const char* basename = strrchr(fFilePath.c_str(), SkOSPath::SEPARATOR);
    fName.printf("BISECT_%s", basename ? basename + 1 : fFilePath.c_str());
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
            if (fFoundPaths.size() > 1) {
                int midpt = (fFoundPaths.size() + 1) / 2;
                fPathHistory.emplace(fFoundPaths, fTossedPaths);
                fTossedPaths.reset(fFoundPaths.begin() + midpt, fFoundPaths.size() - midpt);
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
        SkAutoCanvasRestore acr2(canvas, true);
        canvas->concat(path.fViewMatrix);
        canvas->drawPath(path.fPath, path.fPaint);
    }
}
