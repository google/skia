/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is an example of the translation unit that needs to be
// assembled by the fiddler program to compile into a fiddle: an
// implementation of the GetDrawOptions() and draw() functions.

#include "tools/fiddle/fiddle_main.h"
#include "tools/gpu/ManagedBackendTexture.h"

DrawOptions GetDrawOptions() {
    // path *should* be absolute.
    static const char path[] = "resources/images/color_wheel.png";
    return DrawOptions(256, 256, true, true, true, true, true, false, false, 0,
                       GrMipmapped::kYes, 64, 64, 0, GrMipmapped::kYes);
}
void draw(SkCanvas* canvas) {
    SkPaint p;
    p.setColor(SK_ColorRED);
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(10);

    canvas->drawLine(20, 20, 100, 100, p);


    const char * svg =
"<?xml version='1.0' encoding='UTF-8' standalone='no'?>\n"
"<!-- In the colr format the eyes and mouth should be filled with the current paint. -->\n"
"<svg\n"
"   xmlns:svg='http://www.w3.org/2000/svg'\n"
"   xmlns='http://www.w3.org/2000/svg'\n"
"   version='1.1'\n"
"   viewBox='0 0 211.66666 211.66667'\n"
"   height='256'\n"
"   width='256'>\n"
"  <g\n"
"     transform='translate(0,-85.333317)'\n"
"     id='layer1'>\n"
"    <circle\n"
"       r='98.273819'\n"
"       cy='192.76852'\n"
"       cx='104.81271'\n"
"       id='path10'\n"
"       style='fill:#000000;stroke-width:0.3084828' />\n"
"    <circle\n"
"       r='90.782074'\n"
"       cy='192.32785'\n"
"       cx='104.37203'\n"
"       id='path12'\n"
"       style='fill:#ffcc00;stroke-width:0.3084828' />\n"
"    <ellipse\n"
"       ry='20.032738'\n"
"       rx='20.410715'\n"
"       cy='159.88458'\n"
"       cx='74.574615'\n"
"       id='path62'\n"
"       style='fill:#2b0000;stroke-width:0.26458332' />\n"
"    <ellipse\n"
"       ry='20.032738'\n"
"       rx='20.410715'\n"
"       cy='158.13309'\n"
"       cx='138.02272'\n"
"       id='path62-7'\n"
"       style='fill:#2b0000;stroke-width:0.26458332' />\n"
"    <path\n"
"       d='m 163.09897,220.67157 a 58.248119,31.234499 0 0 1 -26.87407,29.28648 58.248119,31.234499 0 0 1 -60.863188,0.45572 58.248119,31.234499 0 0 1 -28.388544,-28.87269 l 58.126712,2.01564 z'\n"
"       id='path113'\n"
"       style='fill:#2b0000;stroke-width:0.26458332' />\n"
"  </g>\n"
"</svg>";

    sk_sp<SkData> data(SkData::MakeWithoutCopy(svg, strlen(svg)));
/*
    if (!data) {
        return;
    }
*/
    SkMemoryStream stream(data);
    sk_sp<SkSVGDOM> svgDom = SkSVGDOM::MakeFromStream(stream);

/*
    if (!svgDom) {
        return;
    }
*/
    // Use the intrinsic SVG size if available, otherwise fall back to a default value.
    static const SkSize kDefaultContainerSize = SkSize::Make(128, 128);
    if (svgDom->containerSize().isEmpty()) {
        svgDom->setContainerSize(kDefaultContainerSize);
    }

    svgDom->render(canvas);
}
