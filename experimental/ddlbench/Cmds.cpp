// Copyright 2021 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "experimental/ddlbench/Cmds.h"
#include "experimental/ddlbench/Fake.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"

void RectCmd::execute(FakeCanvas* c) const {
    FakePaint p;
    p.setColor(fColor);
    c->drawRect(fID, fRect, p);
}

static bool is_opaque(SkColor c) {
    return 0xFF == SkColorGetA(c);
}

void RectCmd::rasterize(uint32_t zBuffer[256][256], SkBitmap* dstBM, unsigned int z) const {
    for (int y = fRect.fTop; y < fRect.fBottom; ++y) {
        for (int x = fRect.fLeft; x < fRect.fRight; ++x) {
            if (z > zBuffer[x][y]) {
                zBuffer[x][y] = z;
                if (is_opaque(fColor)) {
                    *dstBM->getAddr32(x, y) = fColor;
                } else {
                    SkColor4f bot = SkColor4f::FromColor((SkColor)*dstBM->getAddr32(x, y));
                    SkColor4f top = SkColor4f::FromColor(fColor);
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
    p.setColor(fColor);
    c->drawRect(SkRect::Make(fRect), p);
}
