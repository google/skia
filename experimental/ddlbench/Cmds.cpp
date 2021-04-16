// Copyright 2021 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "experimental/ddlbench/Cmds.h"
#include "experimental/ddlbench/Fake.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"

void RectCmd::execute(FakeCanvas* c) const {
    FakePaint p;
    p.setColor(fColor);
    c->drawRect(fID, fRect, p);
}

void RectCmd::execute(SkCanvas* c) const {
    SkPaint p;
    p.setColor(fColor);
    c->drawRect(fRect, p);
}
