// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "SCanvas.h"
#include "include/core/SkPath.h"

int main() {
    std::unique_ptr<SkCanvas> canvas = MakeSCanvas({100, 100});

    canvas->rotate(30);

    canvas->getTotalMatrix().dump();

    SkPath path;
    path.toggleInverseFillType();

    SkPaint paint;
    paint.setColor(SK_ColorRED);

    canvas->drawPath(path, paint);
}
