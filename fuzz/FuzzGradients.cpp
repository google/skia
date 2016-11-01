/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Fuzz.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkSurface.h"
#include "SkTLazy.h"

#include <algorithm>
#include <vector>

const int MAX_COUNT = 400;

void makeMatrix(Fuzz* fuzz, SkMatrix* m) {
    m->setAll(fuzz->next<SkScalar>(), fuzz->next<SkScalar>(), fuzz->next<SkScalar>(),
              fuzz->next<SkScalar>(), fuzz->next<SkScalar>(), fuzz->next<SkScalar>(),
              fuzz->next<SkScalar>(), fuzz->next<SkScalar>(), fuzz->next<SkScalar>());
}

void initGradientParams(Fuzz* fuzz, std::vector<SkColor>* colors,
                        std::vector<SkScalar>* pos, SkShader::TileMode* mode) {
    int count = fuzz->nextRange(0, MAX_COUNT);

    *mode = static_cast<SkShader::TileMode>(fuzz->nextRange(0, 2));

    colors->clear();
    pos   ->clear();
    for (int i = 0; i < count; i++) {
        colors->push_back(fuzz->next<SkColor>());
        pos   ->push_back(fuzz->next<SkScalar>());
    }
    if (count) {
        std::sort(pos->begin(), pos->end());
        // The order matters.  If count == 1, we want pos == 0.
        (*pos)[count - 1] = 1;
        (*pos)[0]         = 0;
    }
}

void fuzzLinearGradient(Fuzz* fuzz) {
    SkPoint pts[2] = {SkPoint::Make(fuzz->next<SkScalar>(), fuzz->next<SkScalar>()),
                      SkPoint::Make(fuzz->next<SkScalar>(), fuzz->next<SkScalar>())};
    bool useLocalMatrix  = fuzz->next<bool>();
    bool useGlobalMatrix = fuzz->next<bool>();

    std::vector<SkColor> colors;
    std::vector<SkScalar> pos;
    SkShader::TileMode mode;
    initGradientParams(fuzz, &colors, &pos, &mode);

    SkPaint p;
    uint32_t flags = fuzz->next<uint32_t>();

    SkTLazy<SkMatrix> localMatrix;
    if (useLocalMatrix) {
        makeMatrix(fuzz, localMatrix.init());
    }
    p.setShader(SkGradientShader::MakeLinear(pts, colors.data(), pos.data(),
        colors.size(), mode, flags, localMatrix.getMaybeNull()));

    sk_sp<SkSurface> surface(SkSurface::MakeRasterN32Premul(50, 50));
    if (useGlobalMatrix) {
        SkMatrix gm;
        makeMatrix(fuzz, &gm);
        SkCanvas* c = surface->getCanvas();
        c->setMatrix(gm);
        c->drawPaint(p);
    } else {
        surface->getCanvas()->drawPaint(p);
    }
}

void fuzzRadialGradient(Fuzz* fuzz) {
    SkPoint center = SkPoint::Make(fuzz->next<SkScalar>(), fuzz->next<SkScalar>());
    SkScalar radius      = fuzz->next<SkScalar>();
    bool useLocalMatrix  = fuzz->next<bool>();
    bool useGlobalMatrix = fuzz->next<bool>();


    std::vector<SkColor> colors;
    std::vector<SkScalar> pos;
    SkShader::TileMode mode;
    initGradientParams(fuzz, &colors, &pos, &mode);

    SkPaint p;
    uint32_t flags = fuzz->next<uint32_t>();

    SkTLazy<SkMatrix> localMatrix;
    if (useLocalMatrix) {
        makeMatrix(fuzz, localMatrix.init());
    }
    p.setShader(SkGradientShader::MakeRadial(center, radius, colors.data(),
        pos.data(), colors.size(), mode, flags, localMatrix.getMaybeNull()));


    sk_sp<SkSurface> surface(SkSurface::MakeRasterN32Premul(50, 50));
    if (useGlobalMatrix) {
        SkMatrix gm;
        makeMatrix(fuzz, &gm);
        SkCanvas* c = surface->getCanvas();
        c->setMatrix(gm);
        c->drawPaint(p);
    } else {
        surface->getCanvas()->drawPaint(p);
    }
}

void fuzzTwoPointConicalGradient(Fuzz* fuzz) {
    SkPoint start = SkPoint::Make(fuzz->next<SkScalar>(), fuzz->next<SkScalar>());
    SkPoint end = SkPoint::Make(fuzz->next<SkScalar>(), fuzz->next<SkScalar>());
    SkScalar startRadius = fuzz->next<SkScalar>();
    SkScalar endRadius   = fuzz->next<SkScalar>();
    bool useLocalMatrix  = fuzz->next<bool>();
    bool useGlobalMatrix = fuzz->next<bool>();

    std::vector<SkColor> colors;
    std::vector<SkScalar> pos;
    SkShader::TileMode mode;
    initGradientParams(fuzz, &colors, &pos, &mode);

    SkPaint p;
    uint32_t flags = fuzz->next<uint32_t>();

    SkTLazy<SkMatrix> localMatrix;
    if (useLocalMatrix) {
        makeMatrix(fuzz, localMatrix.init());
    }
    p.setShader(SkGradientShader::MakeTwoPointConical(start, startRadius,
        end, endRadius, colors.data(), pos.data(), colors.size(), mode,
        flags, localMatrix.getMaybeNull()));

    sk_sp<SkSurface> surface(SkSurface::MakeRasterN32Premul(50, 50));
    if (useGlobalMatrix) {
        SkMatrix gm;
        makeMatrix(fuzz, &gm);
        SkCanvas* c = surface->getCanvas();
        c->setMatrix(gm);
        c->drawPaint(p);
    } else {
        surface->getCanvas()->drawPaint(p);
    }
}

void fuzzSweepGradient(Fuzz* fuzz) {
    SkScalar cx = fuzz->next<SkScalar>();
    SkScalar cy = fuzz->next<SkScalar>();
    bool useLocalMatrix  = fuzz->next<bool>();
    bool useGlobalMatrix = fuzz->next<bool>();

    std::vector<SkColor> colors;
    std::vector<SkScalar> pos;
    SkShader::TileMode mode;
    initGradientParams(fuzz, &colors, &pos, &mode);

    SkPaint p;
    if (useLocalMatrix) {
        SkMatrix m;
        makeMatrix(fuzz, &m);
        uint32_t flags = fuzz->next<uint32_t>();

        p.setShader(SkGradientShader::MakeSweep(cx, cy, colors.data(),
            pos.data(), colors.size(), flags, &m));
    } else {
        p.setShader(SkGradientShader::MakeSweep(cx, cy, colors.data(),
            pos.data(), colors.size()));
    }

    sk_sp<SkSurface> surface(SkSurface::MakeRasterN32Premul(50, 50));
    if (useGlobalMatrix) {
        SkMatrix gm;
        makeMatrix(fuzz, &gm);
        SkCanvas* c = surface->getCanvas();
        c->setMatrix(gm);
        c->drawPaint(p);
    } else {
        surface->getCanvas()->drawPaint(p);
    }
}

DEF_FUZZ(Gradients, fuzz) {
    uint8_t i = fuzz->next<uint8_t>();

    switch(i) {
        case 0:
            SkDebugf("LinearGradient\n");
            fuzzLinearGradient(fuzz);
            return;
        case 1:
            SkDebugf("RadialGradient\n");
            fuzzRadialGradient(fuzz);
            return;
        case 2:
            SkDebugf("TwoPointConicalGradient\n");
            fuzzTwoPointConicalGradient(fuzz);
            return;
    }
    SkDebugf("SweepGradient\n");
    fuzzSweepGradient(fuzz);
    return;
}
