/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/svg/SkSVGCanvas.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkTo.h"
#include "src/svg/SkSVGDevice.h"
#include "src/xml/SkXMLWriter.h"

#include <utility>

std::unique_ptr<SkCanvas> SkSVGCanvas::Make(const SkRect& bounds, SkWStream* writer,
                                            uint32_t flags) {
    // TODO: pass full bounds to the device
    const auto size = bounds.roundOut().size();
    const auto xml_flags = (flags & kNoPrettyXML_Flag) ? SkToU32(SkXMLStreamWriter::kNoPretty_Flag)
                                                       : 0;

    auto svgDevice = SkSVGDevice::Make(size,
                                       std::make_unique<SkXMLStreamWriter>(writer, xml_flags),
                                       flags);

    return svgDevice ? std::make_unique<SkCanvas>(std::move(svgDevice))
                     : nullptr;
}
