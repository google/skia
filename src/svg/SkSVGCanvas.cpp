/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/svg/SkSVGCanvas.h"
#include "src/core/SkMakeUnique.h"
#include "src/svg/SkSVGDevice.h"
#include "src/xml/SkXMLWriter.h"

std::unique_ptr<SkCanvas> SkSVGCanvas::Make(const SkRect& bounds, SkWStream* writer) {
    // TODO: pass full bounds to the device
    SkISize size = bounds.roundOut().size();

    auto svgDevice = SkSVGDevice::Make(size, skstd::make_unique<SkXMLStreamWriter>(writer));

    return svgDevice ? skstd::make_unique<SkCanvas>(svgDevice)
                     : nullptr;
}
