// Copyright 2021 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "experimental/ngatoy/Cmds.h"
#include "experimental/ngatoy/Fake.h"
#include "experimental/ngatoy/SortKey.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/effects/SkGradientShader.h"


//------------------------------------------------------------------------------------------------
SortKey SaveCmd::getKey() {
    SkASSERT(0);
    return {};
}

void SaveCmd::execute(FakeCanvas* f) const {
    f->save();
}

void SaveCmd::execute(SkCanvas* c) const {
    c->save();
}

//------------------------------------------------------------------------------------------------
SortKey RestoreCmd::getKey() {
    SkASSERT(0);
    return {};
}

void RestoreCmd::execute(FakeCanvas* f) const {
    f->restore();
}

void RestoreCmd::execute(SkCanvas* c) const {
    c->restore();
}

//------------------------------------------------------------------------------------------------
RectCmd::RectCmd(ID id,
                 SkIRect r,
                 const FakePaint& p)
    : Cmd(id)
    , fRect(r)
    , fPaint(p) {
}

RectCmd::RectCmd(ID id,
                 PaintersOrder paintersOrder,
                 SkIRect r,
                 const FakePaint& p,
                 sk_sp<FakeMCBlob> state)
    : Cmd(id)
    , fPaintersOrder(paintersOrder)
    , fRect(r)
    , fPaint(p)
    , fMCState(std::move(state)) {
}

uint32_t RectCmd::getSortZ() const {
    return fPaintersOrder.toUInt();
}

uint32_t RectCmd::getDrawZ() const {
    return fPaintersOrder.toUInt();
}

SortKey RectCmd::getKey() {
    return SortKey(fPaint.isTransparent(), this->getSortZ(), fPaint.toID());
}

void RectCmd::execute(FakeCanvas* c) const {
    c->drawRect(fID, fRect, fPaint);
}

void RectCmd::execute(SkCanvas* c) const {

    SkColor4f colors[2] = {
        SkColor4f::FromColor(fPaint.c0()),
        SkColor4f::FromColor(fPaint.c1())
    };

    SkPaint p;
    if (fPaint.toID() == kSolidMat) {
        p.setColor(fPaint.c0());
    } else if (fPaint.toID() == kLinearMat) {
        SkPoint pts[] = { { 0.0f, 0.0f, }, { 256.0f, 256.0f } };
        p.setShader(SkGradientShader::MakeLinear(pts, colors, nullptr, nullptr, 2,
                                                 SkTileMode::kClamp));
    } else {
        SkASSERT(fPaint.toID() == kRadialMat);

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

static bool is_opaque(SkColor c) {
    return 0xFF == SkColorGetA(c);
}

void RectCmd::rasterize(uint32_t zBuffer[256][256], SkBitmap* dstBM) const {

    uint32_t z = this->getDrawZ();
    SkIRect scissor = fMCState->scissor();

    for (int y = fRect.fTop; y < fRect.fBottom; ++y) {
        for (int x = fRect.fLeft; x < fRect.fRight; ++x) {
            if (z > zBuffer[x][y]) {
                if (!scissor.contains(x, y)) {
                    continue;
                }

                zBuffer[x][y] = z;

                SkColor c = fPaint.evalColor(x, y);

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

//------------------------------------------------------------------------------------------------
ClipCmd::ClipCmd(ID id, PaintersOrder paintersOrderWhenAdded, SkIRect r)
        : Cmd(id)
        , fRect(r)
        , fPaintersOrderWhenAdded(paintersOrderWhenAdded) {
}

ClipCmd::~ClipCmd() {
}

uint32_t ClipCmd::getSortZ() const {
    // Not used this iteration
    SkASSERT(0);

    SkASSERT(fPaintersOrderWhenAdded.isValid());

    return fPaintersOrderWhenAdded.toUInt();
}

uint32_t ClipCmd::getDrawZ() const {
    // Not used this iteration
    SkASSERT(0);

    return 0;
}

SortKey ClipCmd::getKey() {
    // Not used this iteration
    SkASSERT(0);

    return SortKey(false, this->getSortZ(), kInvalidMat);
}

void ClipCmd::execute(FakeCanvas* c) const {
    // This call is creating the 'real' ClipCmd for the "actual" case
    SkASSERT(!fPaintersOrderWhenAdded.isValid());

    c->clipRect(fID, fRect);
}

void ClipCmd::execute(SkCanvas* c) const {
    c->clipRect(SkRect::Make(fRect));
}

void ClipCmd::rasterize(uint32_t zBuffer[256][256], SkBitmap* /* dstBM */) const {
    // Not used this iteration
    SkASSERT(0);

    uint32_t drawZ = this->getDrawZ();

    for (int y = fRect.fTop; y < fRect.fBottom; ++y) {
        for (int x = fRect.fLeft; x < fRect.fRight; ++x) {
            if (drawZ > zBuffer[x][y]) {
                zBuffer[x][y] = drawZ;
            }
        }
    }
}

//------------------------------------------------------------------------------------------------
