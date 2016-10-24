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

const int MAX_COUNT = 400;

bool makeMatrix(Fuzz* fuzz, SkMatrix* m) {
    SkScalar scaleX, skewX, transX, skewY, scaleY, transY, persp0, persp1, persp2;
    if (!fuzz->next(&scaleX) ||
        !fuzz->next(&skewX)  ||
        !fuzz->next(&transX) ||
        !fuzz->next(&skewY)  ||
        !fuzz->next(&scaleY) ||
        !fuzz->next(&transY) ||
        !fuzz->next(&persp0) ||
        !fuzz->next(&persp1) ||
        !fuzz->next(&persp2)) {
        return false;
    }
    m->setAll(scaleX, skewX, transX, skewY, scaleY, transY, persp0, persp1, persp2);
    return true;
}

bool initGradientParams(Fuzz* fuzz, uint32_t* count, SkColor** colors, SkScalar** pos,
    SkShader::TileMode* mode) {
    if (fuzz->remaining() < sizeof(uint32_t)) {
        return false;
    }
    uint32_t t_count;
    SkColor* t_colors;
    SkScalar* t_pos;

    t_count = fuzz->nextRangeU(0, MAX_COUNT);
    if (t_count == 1) {
        t_count = 2;
    }

    if (fuzz->remaining() < (1 + t_count * (sizeof(SkColor) + sizeof(SkScalar)))) {
        return false;
    }
    t_colors = new SkColor[t_count];
    t_pos = new SkScalar[t_count];
    for (uint32_t i = 0; i < t_count; i++) {
        fuzz->next(&t_colors[i]);
        fuzz->next(&t_pos[i]);
    }

    if (t_count == 0) {
        *count = 0;
        *colors = NULL;
        *pos = NULL;
    } else {
        std::sort(t_pos, t_pos + t_count);
        t_pos[0] = 0;
        t_pos[t_count - 1] = 1;
        *count = t_count;
        *colors = t_colors;
        *pos = t_pos;
    }

    *mode = static_cast<SkShader::TileMode>(fuzz->nextRangeU(0, 3));
    return true;
}

void fuzzLinearGradient(Fuzz* fuzz) {
        SkScalar a, b, c, d;
        bool useLocalMatrix, useGlobalMatrix;
        if (!fuzz->next(&a)               ||
            !fuzz->next(&b)               ||
            !fuzz->next(&c)               ||
            !fuzz->next(&d)               ||
            !fuzz->next(&useLocalMatrix)  ||
            !fuzz->next(&useGlobalMatrix)) {
            return;
        }
        SkPoint pts[2] = {SkPoint::Make(a,b), SkPoint::Make(c, d)};

        uint32_t count;
        SkColor* colors;
        SkScalar* pos;
        SkShader::TileMode mode;
        if (!initGradientParams(fuzz, &count, &colors, &pos, &mode)) {
            return;
        }

        SkPaint p;
        uint32_t flags;
        if (!fuzz->next(&flags)) {
            return;
        }

        SkTLazy<SkMatrix> localMatrix;
        if (useLocalMatrix && !makeMatrix(fuzz, localMatrix.init())) {
            return;
        }
        p.setShader(SkGradientShader::MakeLinear(pts, colors, pos, count, mode,
            flags, localMatrix.getMaybeNull()));

        sk_sp<SkSurface> surface(SkSurface::MakeRasterN32Premul(50, 50));
        if (useGlobalMatrix) {
            SkMatrix gm;
            if (!makeMatrix(fuzz, &gm)) {
                return;
            }
            SkCanvas* c = surface->getCanvas();
            c->setMatrix(gm);
            c->drawPaint(p);
        } else {
            surface->getCanvas()->drawPaint(p);
        }
}

void fuzzRadialGradient(Fuzz* fuzz) {
        SkScalar a, b, radius;
        bool useLocalMatrix, useGlobalMatrix;
        if (!fuzz->next(&a)               ||
            !fuzz->next(&b)               ||
            !fuzz->next(&radius)          ||
            !fuzz->next(&useLocalMatrix)  ||
            !fuzz->next(&useGlobalMatrix)) {
            return;
        }
        SkPoint center = SkPoint::Make(a,b);

        uint32_t count;
        SkColor* colors;
        SkScalar* pos;
        SkShader::TileMode mode;
        if (!initGradientParams(fuzz, &count, &colors, &pos, &mode)) {
            return;
        }

        SkPaint p;
        uint32_t flags;
        if (!fuzz->next(&flags)) {
            return;
        }

        SkTLazy<SkMatrix> localMatrix;
        if (useLocalMatrix && !makeMatrix(fuzz, localMatrix.init())) {
            return;
        }
        p.setShader(SkGradientShader::MakeRadial(center, radius, colors, pos,
            count, mode, flags, localMatrix.getMaybeNull()));


        sk_sp<SkSurface> surface(SkSurface::MakeRasterN32Premul(50, 50));
        if (useGlobalMatrix) {
            SkMatrix gm;
            if (!makeMatrix(fuzz, &gm)) {
                return;
            }
            SkCanvas* c = surface->getCanvas();
            c->setMatrix(gm);
            c->drawPaint(p);
        } else {
            surface->getCanvas()->drawPaint(p);
        }
}

void fuzzTwoPointConicalGradient(Fuzz* fuzz) {
        SkScalar a, b, startRadius, c, d, endRadius;
        bool useLocalMatrix, useGlobalMatrix;
        if (!fuzz->next(&a)               ||
            !fuzz->next(&b)               ||
            !fuzz->next(&startRadius)     ||
            !fuzz->next(&c)               ||
            !fuzz->next(&d)               ||
            !fuzz->next(&endRadius)       ||
            !fuzz->next(&useLocalMatrix)  ||
            !fuzz->next(&useGlobalMatrix)) {
            return;
        }
        SkPoint start = SkPoint::Make(a, b);
        SkPoint end = SkPoint::Make(c, d);

        uint32_t count;
        SkColor* colors;
        SkScalar* pos;
        SkShader::TileMode mode;
        if (!initGradientParams(fuzz, &count, &colors, &pos, &mode)) {
            return;
        }

        SkPaint p;
        uint32_t flags;
        if (!fuzz->next(&flags)) {
            return;
        }

        SkTLazy<SkMatrix> localMatrix;
        if (useLocalMatrix && !makeMatrix(fuzz, localMatrix.init())) {
            return;
        }
        p.setShader(SkGradientShader::MakeTwoPointConical(start, startRadius, end,
            endRadius, colors, pos, count, mode, flags, localMatrix.getMaybeNull()));

        sk_sp<SkSurface> surface(SkSurface::MakeRasterN32Premul(50, 50));
        if (useGlobalMatrix) {
            SkMatrix gm;
            if (!makeMatrix(fuzz, &gm)) {
                return;
            }
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
        if (!fuzz->next(&cx)              ||
            !fuzz->next(&cy)              ||
            !fuzz->next(&useLocalMatrix)  ||
            !fuzz->next(&useGlobalMatrix)) {
            return;
        }

        uint32_t count;
        SkColor* colors;
        SkScalar* pos;
        SkShader::TileMode mode;
        if (!initGradientParams(fuzz, &count, &colors, &pos, &mode)) {
            return;
        }

        SkPaint p;
        if (useLocalMatrix) {
            SkMatrix m;
            if (!makeMatrix(fuzz, &m)) {
                return;
            }
            uint32_t flags;
            if (!fuzz->next(&flags)) {
                return;
            }
            p.setShader(SkGradientShader::MakeSweep(cx, cy, colors, pos, count, flags, &m));
        } else {
            p.setShader(SkGradientShader::MakeSweep(cx, cy, colors, pos, count));
        }


        sk_sp<SkSurface> surface(SkSurface::MakeRasterN32Premul(50, 50));
        if (useGlobalMatrix) {
            SkMatrix gm;
            if (!makeMatrix(fuzz, &gm)) {
                return;
            }
            SkCanvas* c = surface->getCanvas();
            c->setMatrix(gm);
            c->drawPaint(p);
        } else {
            surface->getCanvas()->drawPaint(p);
        }
}

DEF_FUZZ(Gradients, fuzz) {
    uint8_t i;
    if (!fuzz->next(&i)) {
        return;
    }

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
