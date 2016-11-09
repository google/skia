/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSVGCanvas.h"
#include "SkSVGDevice.h"
#include "SkMakeUnique.h"

std::unique_ptr<SkCanvas> SkSVGCanvas::Make(const SkRect& bounds, SkXMLWriter* writer) {
    // TODO: pass full bounds to the device
    SkISize size = bounds.roundOut().size();
    sk_sp<SkBaseDevice> device(SkSVGDevice::Create(size, writer));

    return skstd::make_unique<SkCanvas>(device.get());
}
