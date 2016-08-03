/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSVGAttribute.h"
#include "SkSVGRenderContext.h"

SkSVGPresentationAttributes::SkSVGPresentationAttributes()
    : fFillIsSet(false)
    , fStrokeIsSet(false) { }

void SkSVGPresentationAttributes::setFill(const SkSVGColor& c) {
    fFill = c;
    fFillIsSet = true;
}

void SkSVGPresentationAttributes::setStroke(const SkSVGColor& c) {
    fStroke = c;
    fStrokeIsSet = true;
}


void SkSVGPresentationAttributes::applyTo(SkTCopyOnFirstWrite<SkSVGRenderContext>& ctx) const {
    if (fFillIsSet) {
        ctx.writable()->setFillColor(fFill);
    }

    if (fStrokeIsSet) {
        ctx.writable()->setStrokeColor(fStroke);
    }
}
