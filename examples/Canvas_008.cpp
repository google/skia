// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
namespace {
REG_FIDDLE(Canvas_008, 256, 256, true, 0);
// HASH=d93389d971f8084c4ccc7a66e4e157ee
void draw(SkCanvas* canvas) {
    SkCanvas emptyCanvas;
    SkImageInfo canvasInfo = emptyCanvas.imageInfo();
    SkImageInfo emptyInfo;
    SkDebugf("emptyInfo %c= canvasInfo\n", emptyInfo == canvasInfo ? '=' : '!');
}
}
