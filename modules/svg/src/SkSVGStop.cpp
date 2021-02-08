/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkTPin.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "modules/svg/include/SkSVGStop.h"
#include "modules/svg/include/SkSVGValue.h"

SkSVGStop::SkSVGStop() : INHERITED(SkSVGTag::kStop) {}

bool SkSVGStop::parseAndSetAttribute(const char* n, const char* v) {
    return INHERITED::parseAndSetAttribute(n, v) ||
           this->setOffset(SkSVGAttributeParser::parse<SkSVGLength>("offset", n, v));
}
