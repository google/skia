// Copyright 2021 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "experimental/ngatoy/Cmds.h"
#include "experimental/ngatoy/Fake.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/effects/SkGradientShader.h"

// Linearly blend between c0 & c1:
//      (t == 0) -> c0
//      (t == 1) -> c1
static SkColor blend(float t, SkColor c0, SkColor c1) {
    SkASSERT(t >= 0.0f && t <= 1.0f);

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

SkColor Cmd::evalColor(int x, int y, const SkColor colors[2]) const {
    switch (fMaterialID) {
        case kSolidMat: return colors[0];
        case kLinearMat: {
            float t = SK_ScalarRoot2Over2 * x + SK_ScalarRoot2Over2 * y;
            t /= SK_ScalarSqrt2 * 256.0f;
            return blend(t, colors[0], colors[1]);
        }
        case kRadialMat: {
            x -= 128;
            y -= 128;
            float dist = sqrt(x*x + y*y) / 128.0f;
            if (dist > 1.0f) {
                return colors[0];
            } else {
                return blend(dist, colors[0], colors[1]);
            }
        }
    }
    SkUNREACHABLE;
}

//------------------------------------------------------------------------------------------------
RectCmd::RectCmd(int id, int materialID, SkIRect r, SkColor c0, SkColor c1, sk_sp<FakeMCBlob> state)
        : Cmd(id, materialID, std::move(state))
        , fRect(r) {
    fColors[0] = c0;
    fColors[1] = c1;
}

static void apply_diff(FakeCanvas* c, const FakeMCBlob& desired, const FakeMCBlob* prior) {
    int prefix = desired.determineSharedPrefix(prior);

    if (prior) {
        for (int j = prefix; j < prior->count(); ++j) {
            c->restore();
        }
    }

    for (int j = prefix; j < desired.count(); ++j) {
        desired[j].apply(c);
    }
}

void RectCmd::execute(FakeCanvas* c) const {
    FakePaint p;

    switch (fMaterialID) {
        case kSolidMat:  p.setColor(fColors[0]);              break;
        case kLinearMat: p.setLinear(fColors[0], fColors[1]); break;
        case kRadialMat: p.setRadial(fColors[0], fColors[1]); break;
    }

    apply_diff(c, *fMCState, c->snapState().get());

    c->drawRect(fID, fRect, p);
}

static void apply_diff(SkCanvas* c, const FakeMCBlob& desired, const FakeMCBlob* prior) {
    int prefix = desired.determineSharedPrefix(prior);

    if (prior) {
        for (int j = prefix; j < prior->count(); ++j) {
            c->restore();
        }
    }

    for (int j = prefix; j < desired.count(); ++j) {
        desired[j].apply(c);
    }
}

void RectCmd::execute(SkCanvas* c, const FakeMCBlob* priorState) const {

    SkPaint p;
    if (fMaterialID == kSolidMat) {
        p.setColor(fColors[0]);
    } else if (fMaterialID == kLinearMat) {
        SkPoint pts[] = { { 0.0f, 0.0f, }, { 256.0f, 256.0f } };
        p.setShader(SkGradientShader::MakeLinear(pts, fColors, nullptr, 2,
                                                 SkTileMode::kClamp, 0, nullptr));
    } else {
        SkASSERT(fMaterialID == kRadialMat);

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

    apply_diff(c, *fMCState, priorState);

    c->drawRect(SkRect::Make(fRect), p);
}

static bool is_opaque(SkColor c) {
    return 0xFF == SkColorGetA(c);
}

void RectCmd::rasterize(uint32_t zBuffer[256][256], SkBitmap* dstBM, unsigned int z) const {

    for (int y = fRect.fTop; y < fRect.fBottom; ++y) {
        for (int x = fRect.fLeft; x < fRect.fRight; ++x) {
            if (fMCState->clipped(x, y)) {
                continue;
            }

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

