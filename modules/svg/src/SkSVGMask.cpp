/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/svg/include/SkSVGMask.h"

bool SkSVGMask::parseAndSetAttribute(const char* n, const char* v) {
    return INHERITED::parseAndSetAttribute(n, v) ||
           this->setX(SkSVGAttributeParser::parse<SkSVGLength>("x", n, v)) ||
           this->setY(SkSVGAttributeParser::parse<SkSVGLength>("y", n, v)) ||
           this->setWidth(SkSVGAttributeParser::parse<SkSVGLength>("width", n, v)) ||
           this->setHeight(SkSVGAttributeParser::parse<SkSVGLength>("height", n, v)) ||
           this->setMaskUnits(
                SkSVGAttributeParser::parse<SkSVGObjectBoundingBoxUnits>("maskUnits", n, v)) ||
           this->setMaskContentUnits(
                SkSVGAttributeParser::parse<SkSVGObjectBoundingBoxUnits>("maskContentUnits", n, v));
}
