/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <string>

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "modules/svg/include/SkSVGDOM.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "tests/Test.h"

namespace {

constexpr SkColor kGreen = 0xFF008000;

// Left half is filled with currentColor directly, right half through a gradient
// whose stops are currentColor. Both must resolve to the same color.
constexpr char kInheritedColor[] = R"EOF(
<svg width="100" height="100" xmlns="http://www.w3.org/2000/svg" color="green">
    <defs>
        <linearGradient id="g" x1="0" y1="0" x2="1" y2="0">
            <stop offset="0" stop-color="currentColor"/>
            <stop offset="1" stop-color="currentColor"/>
        </linearGradient>
    </defs>
    <rect x="0"  y="0" width="50" height="100" fill="currentColor"/>
    <rect x="50" y="0" width="50" height="100" fill="url(#g)"/>
</svg>
)EOF";

constexpr char kSeededColor[] = R"EOF(
<svg width="100" height="100" xmlns="http://www.w3.org/2000/svg">
    <defs>
        <linearGradient id="g" x1="0" y1="0" x2="1" y2="0">
            <stop offset="0" stop-color="currentColor"/>
            <stop offset="1" stop-color="currentColor"/>
        </linearGradient>
    </defs>
    <g id="glyph1">
        <rect x="0"  y="0" width="50" height="100" fill="currentColor"/>
        <rect x="50" y="0" width="50" height="100" fill="url(#g)"/>
    </g>
</svg>
)EOF";

sk_sp<SkSVGDOM> parse(const char* svg) {
    auto stream = SkMemoryStream::MakeDirect(svg, strlen(svg));
    auto dom = SkSVGDOM::Builder().make(*stream);
    if (dom) {
        dom->setContainerSize(SkSize::Make(100, 100));
    }
    return dom;
}

}  // namespace

// 'color' inherits through the tree, so currentColor inside a referenced paint
// server must resolve the same as it does on the referencing element.
DEF_TEST(Svg_PaintServer_InheritedCurrentColor, r) {
    auto dom = parse(kInheritedColor);
    REPORTER_ASSERT(r, dom);

    SkBitmap bm;
    bm.allocN32Pixels(100, 100);
    bm.eraseColor(SK_ColorWHITE);
    SkCanvas canvas(bm);
    dom->render(&canvas);

    REPORTER_ASSERT(r, bm.getColor(25, 50) == kGreen);  // fill="currentColor"
    REPORTER_ASSERT(r, bm.getColor(75, 50) == kGreen);  // fill="url(#g)"
}

// Same requirement when the host seeds the color on the presentation context
// instead of the document supplying it -- the OpenType-SVG path taken by
// SkSVGOpenTypeSVGDecoder::render().
DEF_TEST(Svg_PaintServer_SeededCurrentColor, r) {
    auto dom = parse(kSeededColor);
    REPORTER_ASSERT(r, dom);

    SkBitmap bm;
    bm.allocN32Pixels(100, 100);
    bm.eraseColor(SK_ColorWHITE);
    SkCanvas canvas(bm);

    SkSVGPresentationContext pctx;
    pctx.fInherited.fColor.set(kGreen);
    dom->renderNode(&canvas, pctx, "glyph1");

    REPORTER_ASSERT(r, bm.getColor(25, 50) == kGreen);  // fill="currentColor"
    REPORTER_ASSERT(r, bm.getColor(75, 50) == kGreen);  // fill="url(#g)"
}
