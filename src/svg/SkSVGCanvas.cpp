/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSVGCanvas.h"
#include "SkSVGDevice.h"

SkCanvas* SkSVGCanvas::Create(const SkRect& bounds, SkXMLWriter* writer) {
    // TODO: pass full bounds to the device
    SkISize size = bounds.roundOut().size();
    SkAutoTUnref<SkBaseDevice> device(SkSVGDevice::Create(size, writer));

    return SkNEW_ARGS(SkCanvas, (device));
}
