/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <string>

#include "include/core/SkStream.h"
#include "include/utils/SkNoDrawCanvas.h"
#include "modules/svg/include/SkSVGDOM.h"
#include "modules/svg/include/SkSVGNode.h"
#include "tests/Test.h"

DEF_TEST(Svg_Filters_NonePaintInputs, r) {
    const std::string svgText = R"EOF(
    <svg width="500" height="500" xmlns="http://www.w3.org/2000/svg"
         xmlns:xlink="http://www.w3.org/1999/xlink">
        <defs>
            <filter id="f" x="0" y="0" width="1" height="1">
                <feComposite operator="arithmetic" in="FillPaint" in2="StrokePaint"
                             k1="0" k2="10" k3="20" k4="0"/>
            </filter>
        </defs>
        <rect fill="none" stroke="none" filter="url(#f)" x="10" y="10" width="100" height="1,0"/>
    </svg>
    )EOF";

    auto str = SkMemoryStream::MakeDirect(svgText.c_str(), svgText.size());
    auto svg_dom = SkSVGDOM::Builder().make(*str);
    SkNoDrawCanvas canvas(500, 500);
    svg_dom->render(&canvas);
}
