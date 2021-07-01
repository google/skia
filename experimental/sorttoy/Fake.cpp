// Copyright 2021 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "experimental/sorttoy/Fake.h"

#include "experimental/sorttoy/Cmds.h"
#include "experimental/sorttoy/SortKey.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"

//-------------------------------------------------------------------------------------------------
void FakeMCBlob::MCState::addClip(sk_sp<ClipCmd> clipCmd) {
    clipCmd->mutate(fTrans);
    fCmds.push_back(std::move(clipCmd));
    fCached = nullptr;
}

bool FakeMCBlob::MCState::operator==(const MCState& other) const {
    if (fTrans != other.fTrans || fCmds.size() != other.fCmds.size()) {
        return false;
    }

    for (size_t i = 0; i < fCmds.size(); ++i) {
        if (fCmds[i]->rect() != other.fCmds[i]->rect()) {
            return false;
        }
    }

    return true;
}

void FakeMCBlob::MCState::aboutToBePopped(PaintersOrder paintersOrderWhenPopped) {
    for (sk_sp<ClipCmd>& c : fCmds) {
        c->onAboutToBePopped(paintersOrderWhenPopped);
    }
}


FakeMCBlob::FakeMCBlob(const std::vector<MCState>& stack) : fID(NextID()), fStack(stack) {
    fScissor = SkIRect::MakeLTRB(-1000, -1000, 1000, 1000);

    for (MCState& s : fStack) {
        // xform the clip rects into device space to compute the scissor
        for (const sk_sp<ClipCmd>& c : s.fCmds) {
            SkASSERT(c->hasBeenMutated());
            SkIRect r = c->rect();
            r.offset(fCTM);
            if (!fScissor.intersect(r)) {
                fScissor.setEmpty();
            }
        }
        fCTM += s.getTrans();
    }
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

int FakePaint::toID() const {
    switch (fType) {
        case Type::kNormal: return kSolidMat;
        case Type::kLinear: return kLinearMat;
        case Type::kRadial: return kRadialMat;
    }
    SkUNREACHABLE;
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

void FakeDevice::drawShape(ID id, PaintersOrder paintersOrder, Shape shape, SkIRect r, FakePaint p) {
    sk_sp<FakeMCBlob> state = fTracker.snapState();
    SkASSERT(state);

    sk_sp<Cmd> tmp = sk_make_sp<DrawCmd>(id, paintersOrder, shape, r, p, std::move(state));

    fSortedCmds.push_back(std::move(tmp));
}

void FakeDevice::clipShape(ID id, PaintersOrder paintersOrder, Shape shape, SkIRect r) {
    sk_sp<ClipCmd> tmp = sk_make_sp<ClipCmd>(id, paintersOrder, shape, r);

    fTracker.clip(std::move(tmp));
}

void FakeDevice::restore(PaintersOrder paintersOrderWhenPopped) {
    fTracker.pop(paintersOrderWhenPopped);
}

void FakeDevice::finalize() {
    SkASSERT(!fFinalized);
    fFinalized = true;

    this->sort();
    for (const sk_sp<Cmd>& c : fSortedCmds) {
        c->rasterize(fZBuffer, &fBM);
    }
}

void FakeDevice::getOrder(std::vector<ID>* ops) const {
    SkASSERT(fFinalized);

    for (const sk_sp<Cmd>& c : fSortedCmds) {
        ops->push_back(c->id());
    }
}

void FakeDevice::sort() {
    // In general we want:
    //  opaque draws to occur front to back (i.e., in reverse painter's order) while minimizing
    //        state changes due to materials
    //  transparent draws to occur back to front (i.e., in painter's order)
    //
    // In both scenarios we would like to batch as much as possible.
    std::sort(fSortedCmds.begin(), fSortedCmds.end(),
              [](const sk_sp<Cmd>& a, const sk_sp<Cmd>& b) {
                    return a->getKey() < b->getKey();
                });
}

//-------------------------------------------------------------------------------------------------
void FakeCanvas::drawShape(ID id, Shape shape, SkIRect r, FakePaint p) {
    SkASSERT(!fFinalized);

    fDeviceStack.back()->drawShape(id, this->nextPaintersOrder(), shape, r, p);
}

void FakeCanvas::clipShape(ID id, Shape shape, SkIRect r) {
    SkASSERT(!fFinalized);

    fDeviceStack.back()->clipShape(id, this->nextPaintersOrder(), shape, r);
}

void FakeCanvas::finalize() {
    SkASSERT(!fFinalized);
    fFinalized = true;

    for (auto& d : fDeviceStack) {
        d->finalize();
    }
}

std::vector<ID> FakeCanvas::getOrder() const {
    SkASSERT(fFinalized);

    std::vector<ID> ops;

    for (auto& d : fDeviceStack) {
        d->getOrder(&ops);
    }

    return ops;
}
