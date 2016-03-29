/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkSVGClipPath.h"
#include "SkSVGParser.h"
#include "SkSVGUse.h"

DEFINE_SVG_NO_INFO(ClipPath)

bool SkSVGClipPath::isDef() {
    return true;
}

bool SkSVGClipPath::isNotDef() {
    return false;
}

void SkSVGClipPath::translate(SkSVGParser& parser, bool defState) {
    parser._startElement("clip");
    INHERITED::translate(parser, defState);
    SkASSERT(fChildren.count() == 1);
    SkSVGElement* child = *fChildren.begin();
    SkASSERT(child->getType() == SkSVGType_Use);
    SkSVGUse* use = (SkSVGUse*) child;
    SkSVGElement* ref = nullptr;
    const char* refStr = &use->f_xlink_href.c_str()[1];
    SkASSERT(parser.getIDs().find(refStr, &ref));
    SkASSERT(ref);
    if (ref->getType() == SkSVGType_Rect)
        parser._addAttribute("rectangle", refStr);
    else
        parser._addAttribute("path", refStr);
    parser._endElement();
}
