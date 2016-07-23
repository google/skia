/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkSVGStop.h"
#include "SkSVGParser.h"

const SkSVGAttribute SkSVGStop::gAttributes[] = {
    SVG_ATTRIBUTE(offset)
};

DEFINE_SVG_INFO(Stop)

void SkSVGStop::translate(SkSVGParser& parser, bool defState) {
    parser._startElement("color");
    INHERITED::translate(parser, defState);
    parser._addAttribute("color", parser.getPaintLast(SkSVGPaint::kStopColor));
    parser._endElement();
}
