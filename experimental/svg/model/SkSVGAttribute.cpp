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

void SkSVGPresentationAttributes::setFill(const SkSVGColorType& c) {
    fFill = c;
    fFillIsSet = true;
}

void SkSVGPresentationAttributes::setStroke(const SkSVGColorType& c) {
    fStroke = c;
    fStrokeIsSet = true;
}


void SkSVGPresentationAttributes::applyTo(SkSVGRenderContext* ctx) const {
    if (fFillIsSet) {
        ctx->writablePresentationContext()->setFillColor(fFill);
    }

    if (fStrokeIsSet) {
        ctx->writablePresentationContext()->setStrokeColor(fStroke);
    }
}
