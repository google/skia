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
void FakeDevice::save() {
    fTracker.push();
}

void FakeDevice::drawRect(int id, uint32_t z, SkIRect r, FakePaint p) {

    int matID = p.toID();

    SkASSERT(p.c0() != SK_ColorUNUSED);
    if (matID == kSolidMat) {
        SkASSERT(p.c1() == SK_ColorUNUSED);
    } else {
        SkASSERT(p.c1() != SK_ColorUNUSED);
    }

    sk_sp<FakeMCBlob> state = fTracker.snapState();

    SortKey k(p.isTransparent(), state->id(), z, matID);

    auto tmp = new RectCmd(id, matID, r, p.c0(), p.c1(), std::move(state));

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

