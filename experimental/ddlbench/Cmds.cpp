// Copyright 2021 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "experimental/ddlbench/Cmds.h"
#include "experimental/ddlbench/Fake.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/effects/SkGradientShader.h"

void RectCmd::execute(FakeCanvas* c) const {
    FakePaint p;

    switch (fMaterialID) {
        case 1: p.setColor(fColors[0]);              break;
        case 2: p.setLinear(fColors[0], fColors[1]); break;
        case 3: p.setRadial(fColors[0], fColors[1]); break;
    }

    c->drawRect(fID, fRect, p);
}

static bool is_opaque(SkColor c) {
    return 0xFF == SkColorGetA(c);
}

SkColor Cmd::blend(float t, SkColor c0, SkColor c1) {
    SkColor4f top = SkColor4f::FromColor(c0);
    SkColor4f bot = SkColor4f::FromColor(c1);

    SkColor4f result = {
        t * bot.fR + (1.0f - t) * top.fR,
        t * bot.fG + (1.0f - t) * top.fG,
        t * bot.fB + (1.0f - t) * top.fB,
        t * bot.fA + (1.0f - t) * top.fA
    };
    return result.toSkColor();
}

void RectCmd::rasterize(uint32_t zBuffer[256][256], SkBitmap* dstBM, unsigned int z) const {
    for (int y = fRect.fTop; y < fRect.fBottom; ++y) {
        for (int x = fRect.fLeft; x < fRect.fRight; ++x) {
            if (z > zBuffer[x][y]) {
                zBuffer[x][y] = z;

                SkColor c = this->evalColor(x, y, fColors);

                if (is_opaque(c)) {
                    *dstBM->getAddr32(x, y) = c;
                } else {
                    SkColor4f bot = SkColor4f::FromColor(*dstBM->getAddr32(x, y));
                    SkColor4f top = SkColor4f::FromColor(c);
                    SkColor4f result = {
                        top.fA * top.fR + (1.0f - top.fA) * bot.fR,
                        top.fA * top.fG + (1.0f - top.fA) * bot.fG,
                        top.fA * top.fB + (1.0f - top.fA) * bot.fB,
                                 top.fA + (1.0f - top.fA) * bot.fA
                    };
                    *dstBM->getAddr32(x, y) = result.toSkColor();
                }
            }
        }
    }
}

void RectCmd::execute(SkCanvas* c) const {

    SkPaint p;
    if (fMaterialID == 1) {
        p.setColor(fColors[0]);
    } else if (fMaterialID == 2) {
        SkPoint pts[] = { { 0.0f, 0.0f, }, { 256.0f, 256.0f } };
        p.setShader(SkGradientShader::MakeLinear(pts, fColors, nullptr, 2,
                                                 SkTileMode::kClamp, 0, nullptr));
    } else {
        SkASSERT(fMaterialID == 3);

        SkColor4f colors[2] = {
            SkColor4f::FromColor(fColors[0]),
            SkColor4f::FromColor(fColors[1])
        };
        auto shader = SkGradientShader::MakeRadial(SkPoint::Make(128.0f, 128.0f),
                                                   128.0f,
                                                   colors,
                                                   nullptr,
                                                   nullptr,
                                                   2,
                                                   SkTileMode::kRepeat);
        p.setShader(std::move(shader));
    }

    c->drawRect(SkRect::Make(fRect), p);
}
