/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkSVGFeColorMatrix.h"
#include "SkSVGParser.h"

const SkSVGAttribute SkSVGFeColorMatrix::gAttributes[] = {
    SVG_LITERAL_ATTRIBUTE(color-interpolation-filters, f_color_interpolation_filters),
    SVG_ATTRIBUTE(result),
    SVG_ATTRIBUTE(type),
    SVG_ATTRIBUTE(values)
};

DEFINE_SVG_INFO(FeColorMatrix)

void SkSVGFeColorMatrix::translate(SkSVGParser& parser, bool defState) {
    INHERITED::translate(parser, defState);
}
