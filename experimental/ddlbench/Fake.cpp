// Copyright 2021 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "Fake.h"

#include "Cmds.h"

#include "include/core/SkCanvas.h"

void FakeCanvas::drawRect(int id, SkRect r, FakePaint p) {
    SkASSERT(!fFinalized);

    auto tmp = new RectCmd(id, r, p.getColor());

    int pseudoZ = this->nextZ();

    Key k(pseudoZ, 0);

    fSortedCmds.push_back({k, tmp});
}

std::vector<int> FakeCanvas::getOrder() const {
    std::vector<int> ops;
    ops.reserve(fSortedCmds.size());

    for (auto f : fSortedCmds) {
        ops.push_back(f.fCmd->id());
    }

    return ops;
}

void FakeCanvas::replay(SkBitmap* dstBM) const {
    SkCanvas replayCanvas(*dstBM);

    for (auto f : fSortedCmds) {
        f.fCmd->execute(&replayCanvas);
    }
}