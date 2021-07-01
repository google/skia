// Copyright 2021 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "experimental/sorttoy/Cmds.h"
#include "experimental/sorttoy/Fake.h"
#include "experimental/sorttoy/SortKey.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
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
DrawCmd::DrawCmd(ID id,
                 Shape shape,
                 SkIRect r,
                 const FakePaint& p)
    : Cmd(id)
    , fShape(shape)
    , fRect(r)
    , fPaint(p) {
}

DrawCmd::DrawCmd(ID id,
                 PaintersOrder paintersOrder,
                 Shape shape,
                 SkIRect r,
                 const FakePaint& p,
                 sk_sp<FakeMCBlob> state)
    : Cmd(id)
    , fPaintersOrder(paintersOrder)
    , fShape(shape)
    , fRect(r)
    , fPaint(p)
    , fMCState(std::move(state)) {
}

static bool shared_contains(int x, int y, Shape s, SkIRect r) {
    if (s == Shape::kRect) {
        return r.contains(x, y);
    } else {
        float a = r.width() / 2.0f;   // horizontal radius
        float b = r.height() / 2.0f;  // vertical radius
        float h = 0.5f * (r.fLeft + r.fRight); // center X
        float k = 0.5f * (r.fTop + r.fBottom); // center Y

        float xTerm = x + 0.5f - h;
        float yTerm = y + 0.5f - k;

        return (xTerm * xTerm) / (a * a) + (yTerm * yTerm) / (b * b) < 1.0f;
    }
}

bool DrawCmd::contains(int x, int y) const {
    return shared_contains(x, y, fShape, fRect);
}

uint32_t DrawCmd::getSortZ() const {
    return fPaintersOrder.toUInt();
}

// Opaque and transparent draws both write their painter's index to the depth buffer
uint32_t DrawCmd::getDrawZ() const {
    return fPaintersOrder.toUInt();
}

SortKey DrawCmd::getKey() {
    return SortKey(fPaint.isTransparent(), this->getSortZ(), fPaint.toID());
}

void DrawCmd::execute(FakeCanvas* c) const {
    c->drawShape(fID, fShape, fRect, fPaint);
}

void DrawCmd::execute(SkCanvas* c) const {

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

    if (fShape == Shape::kRect) {
        c->drawRect(SkRect::Make(fRect), p);
    } else {
        c->drawOval(SkRect::Make(fRect), p);
    }
}

static bool is_opaque(SkColor c) {
    return 0xFF == SkColorGetA(c);
}

void DrawCmd::rasterize(uint32_t zBuffer[256][256], SkBitmap* dstBM) const {

    uint32_t z = this->getDrawZ();
    SkIRect scissor = fMCState->scissor();

    for (int y = fRect.fTop; y < fRect.fBottom; ++y) {
        for (int x = fRect.fLeft; x < fRect.fRight; ++x) {
            if (!scissor.contains(x, y)) {
                continue;
            }

            if (!this->contains(x, y)) {
                continue;
            }

            if (z > zBuffer[x][y]) {
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
ClipCmd::ClipCmd(ID id, Shape shape, SkIRect r)
        : Cmd(id)
        , fShape(shape)
        , fRect(r) {
}

ClipCmd::ClipCmd(ID id, PaintersOrder paintersOrderWhenAdded, Shape shape, SkIRect r)
        : Cmd(id)
        , fShape(shape)
        , fRect(r)
        , fPaintersOrderWhenAdded(paintersOrderWhenAdded) {
}

ClipCmd::~ClipCmd() {}

bool ClipCmd::contains(int x, int y) const {
    return shared_contains(x, y, fShape, fRect);
}

uint32_t ClipCmd::getSortZ() const {
    SkASSERT(fPaintersOrderWhenAdded.isValid());

    return fPaintersOrderWhenAdded.toUInt();
}

// A clip writes the painter's index corresponding to when it's "popped" off the clip stack
uint32_t ClipCmd::getDrawZ() const {
    SkASSERT(fPaintersOrderWhenPopped.isValid());

    return fPaintersOrderWhenPopped.toUInt();
}

SortKey ClipCmd::getKey() {
    return SortKey(false, this->getSortZ(), kInvalidMat);
}

void ClipCmd::onAboutToBePopped(PaintersOrder paintersOrderWhenPopped) {
    SkASSERT(!fPaintersOrderWhenPopped.isValid() && paintersOrderWhenPopped.isValid());
    fPaintersOrderWhenPopped = paintersOrderWhenPopped;
}

void ClipCmd::execute(FakeCanvas* c) const {
    // This call is creating the 'real' ClipCmd for the "actual" case
    SkASSERT(!fPaintersOrderWhenAdded.isValid() && !fPaintersOrderWhenPopped.isValid());

    c->clipShape(fID, fShape, fRect);
}

void ClipCmd::execute(SkCanvas* c) const {
    if (fShape == Shape::kRect) {
        c->clipRect(SkRect::Make(fRect));
    } else {
        c->clipRRect(SkRRect::MakeOval(SkRect::Make(fRect)));
    }
}

void ClipCmd::rasterize(uint32_t zBuffer[256][256], SkBitmap* /* dstBM */) const {
    uint32_t drawZ = this->getDrawZ();

    // TODO: limit this via the scissor!
    for (int y = 0; y < 256; ++y) {
        for (int x = 0; x < 256; ++x) {
            if (!this->contains(x, y) && drawZ > zBuffer[x][y]) {
                zBuffer[x][y] = drawZ;
            }
        }
    }
}

//------------------------------------------------------------------------------------------------
