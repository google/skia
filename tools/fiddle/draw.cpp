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


    const char * svg = "<svg width='100' height='100'><rect width='100' height='100' fill='green'/> </svg>\n";

    sk_sp<SkData> data(SkData::MakeWithCString(svg));
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
