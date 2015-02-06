
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkSVGText.h"
#include "SkSVGParser.h"

const SkSVGAttribute SkSVGText::gAttributes[] = {
    SVG_ATTRIBUTE(x),
    SVG_ATTRIBUTE(y)
};

DEFINE_SVG_INFO(Text)

void SkSVGText::translate(SkSVGParser& parser, bool defState) {
    parser._startElement("text");
    INHERITED::translate(parser, defState);
    SVG_ADD_ATTRIBUTE(x);
    SVG_ADD_ATTRIBUTE(y);
    SVG_ADD_ATTRIBUTE(text);
    parser._endElement();
}


const SkSVGAttribute SkSVGTspan::gAttributes[] = {
    SVG_ATTRIBUTE(x),
    SVG_ATTRIBUTE(y)
};

DEFINE_SVG_INFO(Tspan)

void SkSVGTspan::translate(SkSVGParser& parser, bool defState) {
    INHERITED::translate(parser, defState);
}
