/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkSVGUse.h"
#include "SkSVGParser.h"

const SkSVGAttribute SkSVGUse::gAttributes[] = {
    SVG_ATTRIBUTE(height),
    SVG_ATTRIBUTE(width),
    SVG_ATTRIBUTE(x),
    SVG_LITERAL_ATTRIBUTE(xlink:href, f_xlink_href),
    SVG_ATTRIBUTE(y)
};

DEFINE_SVG_INFO(Use)

void SkSVGUse::translate(SkSVGParser& parser, bool defState) {
    INHERITED::translate(parser, defState);
    parser._startElement("add");
    const char* start = strchr(f_xlink_href.c_str(), '#') + 1;
    SkASSERT(start);
    parser._addAttributeLen("use", start, strlen(start) - 1);
    parser._endElement();   // clip
}
