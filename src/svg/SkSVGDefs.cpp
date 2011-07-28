
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkSVGDefs.h"

DEFINE_SVG_NO_INFO(Defs)

bool SkSVGDefs::isDef() {
    return true;
}

bool SkSVGDefs::isNotDef() {
    return false;
}

void SkSVGDefs::translate(SkSVGParser& parser, bool defState) {
    INHERITED::translate(parser, defState);
}
