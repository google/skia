// Copyright 2021 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "experimental/ngatoy/Fake.h"

#include "experimental/ngatoy/Cmds.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"


void FakeMCBlob::MCState::apply(SkCanvas* canvas) const {
    canvas->save();

    for (auto c : fRects) {
        canvas->clipIRect(c);
    }

    canvas->translate(fTrans.fX, fTrans.fY);
}

void FakeMCBlob::MCState::apply(FakeCanvas* canvas) const {
    canvas->save();

    for (auto c : fRects) {
        canvas->clipRect(c);
    }

    canvas->translate(fTrans);
}

//-------------------------------------------------------------------------------------------------
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

SkColor FakePaint::evalColor(int x, int y) const {
    switch (fType) {
        case Type::kNormal: return fColor0;
        case Type::kLinear: {
            float t = SK_ScalarRoot2Over2 * x + SK_ScalarRoot2Over2 * y;
            t /= SK_ScalarSqrt2 * 256.0f;
            return blend(t, fColor0, fColor1);
        }
        case Type::kRadial: {
            x -= 128;
            y -= 128;
            float dist = sqrt(x*x + y*y) / 128.0f;
            if (dist > 1.0f) {
                return fColor0;
            } else {
                return blend(dist, fColor0, fColor1);
            }
        }
    }
    SkUNREACHABLE;
}

//-------------------------------------------------------------------------------------------------
void FakeDevice::save() {
    fTracker.push();
}

void FakeDevice::drawRect(int id, uint32_t paintersOrder, SkIRect r, FakePaint p) {

    sk_sp<FakeMCBlob> state = fTracker.snapState();

    SortKey k(p.isTransparent(), state->id(), paintersOrder, p.toID());

    auto tmp = new RectCmd(id, paintersOrder, r, p, std::move(state));

    fSortedCmds.push_back({k, tmp});
}

void FakeDevice::clipRect(SkIRect r) {
    fTracker.clipRect(r);
}

void FakeDevice::restore() {
    fTracker.pop();
}

void FakeDevice::finalize() {
    SkASSERT(!fFinalized);
    fFinalized = true;

    this->sort();
    for (auto f : fSortedCmds) {
        f.fCmd->rasterize(fZBuffer, &fBM, f.fKey.depth());
    }
}

void FakeDevice::getOrder(std::vector<int>* ops) const {
    SkASSERT(fFinalized);

//    ops->reserve(fSortedCmds.size());

    for (auto f : fSortedCmds) {
        ops->push_back(f.fCmd->id());
    }
}

//-------------------------------------------------------------------------------------------------
void FakeCanvas::drawRect(int id, SkIRect r, FakePaint p) {
    SkASSERT(!fFinalized);

    fDeviceStack.back()->drawRect(id, this->nextZ(), r, p);
}

void FakeCanvas::clipRect(SkIRect r) {
    SkASSERT(!fFinalized);

    fDeviceStack.back()->clipRect(r);
}

void FakeCanvas::finalize() {
    SkASSERT(!fFinalized);
    fFinalized = true;

    for (auto& d : fDeviceStack) {
        d->finalize();
    }
}

std::vector<int> FakeCanvas::getOrder() const {
    SkASSERT(fFinalized);

    std::vector<int> ops;

    for (auto& d : fDeviceStack) {
        d->getOrder(&ops);
    }

    return ops;
}

