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
    SkScalar mat[9];
    fuzz->nextN(mat, 9);
    m->set9(mat);
}

void initGradientParams(Fuzz* fuzz, std::vector<SkColor>* colors,
                        std::vector<SkScalar>* pos, SkShader::TileMode* mode) {
    int count;
    fuzz->nextRange(&count, 0, MAX_COUNT);

    // Use a uint8_t to conserve bytes.  This makes our "fuzzed bytes footprint"
    // smaller, which leads to more efficient fuzzing.
    uint8_t m;
    fuzz->nextRange(&m, 0, 2);
    *mode = static_cast<SkShader::TileMode>(m);

    colors->clear();
    pos   ->clear();
    for (int i = 0; i < count; i++) {
        SkColor c;
        SkScalar s;
        fuzz->next(&c, &s);
        colors->push_back(c);
        pos   ->push_back(s);
    }
    if (count) {
        std::sort(pos->begin(), pos->end());
        // The order matters.  If count == 1, we want pos == 0.
        (*pos)[count - 1] = 1;
        (*pos)[0]         = 0;
    }
}

void fuzzLinearGradient(Fuzz* fuzz) {
    SkPoint pts[2];
    fuzz->next(&pts[0].fX, &pts[0].fY, &pts[1].fX, &pts[1].fY);
    bool useLocalMatrix, useGlobalMatrix;
    fuzz->next(&useLocalMatrix, &useGlobalMatrix);

    std::vector<SkColor> colors;
    std::vector<SkScalar> pos;
    SkShader::TileMode mode;
    initGradientParams(fuzz, &colors, &pos, &mode);

    SkPaint p;
    uint32_t flags;
    fuzz->next(&flags);

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
    SkPoint center;
    fuzz->next(&center.fX, &center.fY);
    SkScalar radius;
    bool useLocalMatrix, useGlobalMatrix;
    fuzz->next(&radius, &useLocalMatrix, &useGlobalMatrix);


    std::vector<SkColor> colors;
    std::vector<SkScalar> pos;
    SkShader::TileMode mode;
    initGradientParams(fuzz, &colors, &pos, &mode);

    SkPaint p;
    uint32_t flags;
    fuzz->next(&flags);

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
    SkPoint start;
    fuzz->next(&start.fX, &start.fY);
    SkPoint end;
    fuzz->next(&end.fX, &end.fY);
    SkScalar startRadius, endRadius;
    bool useLocalMatrix, useGlobalMatrix;
    fuzz->next(&startRadius, &endRadius, &useLocalMatrix, &useGlobalMatrix);

    std::vector<SkColor> colors;
    std::vector<SkScalar> pos;
    SkShader::TileMode mode;
    initGradientParams(fuzz, &colors, &pos, &mode);

    SkPaint p;
    uint32_t flags;
    fuzz->next(&flags);

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
    SkScalar cx, cy;
    bool useLocalMatrix, useGlobalMatrix;
    fuzz->next(&cx, &cy, &useLocalMatrix, &useGlobalMatrix);

    std::vector<SkColor> colors;
    std::vector<SkScalar> pos;
    SkShader::TileMode mode;
    initGradientParams(fuzz, &colors, &pos, &mode);

    SkPaint p;
    if (useLocalMatrix) {
        SkMatrix m;
        makeMatrix(fuzz, &m);
        uint32_t flags;
        fuzz->next(&flags);

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
    uint8_t i;
    fuzz->next(&i);

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
