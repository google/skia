/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/xform/SkShape.h"
#include "experimental/xform/SkXform.h"
#include "include/core/SkCanvas.h"

void GeoShape::draw(XContext* ctx) {
    ctx->drawRect(fRect, fPaint, this->xform());
}

void GroupShape::draw(XContext* ctx) {
    if (fArray.count() == 0) {
        return;
    }

    ctx->push(this->xform());
    for (auto s : fArray) {
        s->draw(ctx);
    }
    ctx->pop();
}
