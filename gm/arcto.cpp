/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkParsePath.h"
#include "SkPath.h"

/*
The arcto test below should draw the same as this SVG:
(Note that Skia's arcTo Direction parameter value is opposite SVG's sweep value, e.g. 0 / 1)

<svg width="500" height="600">
<path d="M 50,100 A50,50,   0,0,1, 150,200" style="stroke:#660000; fill:none; stroke-width:2" />
<path d="M100,100 A50,100,  0,0,1, 200,200" style="stroke:#660000; fill:none; stroke-width:2" />
<path d="M150,100 A50,50,  45,0,1, 250,200" style="stroke:#660000; fill:none; stroke-width:2" />
<path d="M200,100 A50,100, 45,0,1, 300,200" style="stroke:#660000; fill:none; stroke-width:2" />

<path d="M150,200 A50,50,   0,1,0, 150,300" style="stroke:#660000; fill:none; stroke-width:2" />
<path d="M200,200 A50,100,  0,1,0, 200,300" style="stroke:#660000; fill:none; stroke-width:2" />
<path d="M250,200 A50,50,  45,1,0, 250,300" style="stroke:#660000; fill:none; stroke-width:2" />
<path d="M300,200 A50,100, 45,1,0, 300,300" style="stroke:#660000; fill:none; stroke-width:2" />

<path d="M250,400  A120,80 0 0,0 250,500"
    fill="none" stroke="red" stroke-width="5" />

<path d="M250,400  A120,80 0 1,1 250,500"
    fill="none" stroke="green" stroke-width="5"/>

<path d="M250,400  A120,80 0 1,0 250,500"
    fill="none" stroke="purple" stroke-width="5"/>

<path d="M250,400  A120,80 0 0,1 250,500"
    fill="none" stroke="blue" stroke-width="5"/>
</svg>
 */

DEF_SIMPLE_GM(arcto, canvas, 500, 600) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(2);
    paint.setColor(0xFF660000);
    canvas->scale(2, 2);
    SkRect oval = SkRect::MakeXYWH(100, 100, 100, 100);
    SkPath svgArc;

    for (int angle = 0; angle <= 45; angle += 45) {
       for (int oHeight = 2; oHeight >= 1; --oHeight) {
            SkScalar ovalHeight = oval.height() / oHeight;
            svgArc.moveTo(oval.fLeft, oval.fTop);
            svgArc.arcTo(oval.width() / 2, ovalHeight, SkIntToScalar(angle), SkPath::kSmall_ArcSize,
                    SkPath::kCW_Direction, oval.right(), oval.bottom());
            canvas->drawPath(svgArc, paint);
            svgArc.reset();

            svgArc.moveTo(oval.fLeft + 100, oval.fTop + 100);
            svgArc.arcTo(oval.width() / 2, ovalHeight, SkIntToScalar(angle), SkPath::kLarge_ArcSize,
                    SkPath::kCCW_Direction, oval.right(), oval.bottom() + 100);
            canvas->drawPath(svgArc, paint);
            oval.offset(50, 0);
            svgArc.reset();

        }
    }

    paint.setStrokeWidth(5);
    const SkColor purple = 0xFF800080;
    const SkColor darkgreen = 0xFF008000;
    const SkColor colors[] = { SK_ColorRED, darkgreen, purple, SK_ColorBLUE };
    const char* arcstrs[] = {
        "M250,400  A120,80 0 0,0 250,500",
        "M250,400  A120,80 0 1,1 250,500",
        "M250,400  A120,80 0 1,0 250,500",
        "M250,400  A120,80 0 0,1 250,500"
    };
    int cIndex = 0;
    for (const char* arcstr : arcstrs) {
        SkParsePath::FromSVGString(arcstr, &svgArc);
        paint.setColor(colors[cIndex++]);
        canvas->drawPath(svgArc, paint);
    }
}
