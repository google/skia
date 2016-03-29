/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkSVGMetadata.h"
#include "SkSVGParser.h"

DEFINE_SVG_NO_INFO(Metadata)

bool SkSVGMetadata::isDef() {
    return false;
}

bool SkSVGMetadata::isNotDef() {
    return false;
}

void SkSVGMetadata::translate(SkSVGParser& parser, bool defState) {
}
