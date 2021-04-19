// Copyright 2021 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "experimental/ddlbench/Fake.h"

#include "experimental/ddlbench/Cmds.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"

//-------------------------------------------------------------------------------------------------
void FakeDevice::drawRect(int id, uint32_t z, SkIRect r, FakePaint p) {
    Key k(z, 0);

    auto tmp = new RectCmd(id, r, p.getColor());

    fSortedCmds.push_back({k, tmp});
}

void FakeDevice::finalize() {
    SkASSERT(!fFinalized);
    fFinalized = true;

    this->sort();
    for (auto f : fSortedCmds) {
        f.fCmd->rasterize(fZBuffer, &fBM, f.fKey.getZ());
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

    fStateStack.back().fDevice.drawRect(id, this->nextZ(), r, p);
}

void FakeCanvas::finalize() {
    SkASSERT(!fFinalized);
    fFinalized = true;

    for (auto& s : fStateStack) {
        s.fDevice.finalize();
    }
}

std::vector<int> FakeCanvas::getOrder() const {
    SkASSERT(fFinalized);

    std::vector<int> ops;

    for (auto& s : fStateStack) {
        s.fDevice.getOrder(&ops);
    }

    return ops;
}

