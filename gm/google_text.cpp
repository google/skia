/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkRRect.h"
#include "SkPaint.h"
#include "SkColor.h"
#include "SkString.h"
#include "SkPath.h"
#include "sk_tool_utils.h"
#include "SkTypeface.h"

class GoogleDraw {
public:

	static void drawGoogle(SkCanvas* canvas) {

        SkAutoCanvasRestore acr(canvas, true);

        SkPaint paint;
        paint.setLCDRenderText(false);
        paint.setColor(0x80FF0000);
        sk_tool_utils::set_portable_typeface(&paint, "serif");

        SkMatrix matrix;
        SkMatrix scale;
        scale.setScale(0.8, 0.8);
        matrix.set(0, 1464.61);
        matrix.set(3, -6.1324);
        matrix.set(6, -0.0592723);
        matrix.set(1, -365.449);
        matrix.set(4, -1769.18);
        matrix.set(7, -0.64453);
        matrix.set(2, 291.048);
        matrix.set(5, 414.224);
        matrix.set(8, 0.452724);
        canvas->concat(scale);
        canvas->concat(matrix);

        canvas->drawCircle(0, 0, 0.1, paint);

        paint.setColor(SK_ColorBLUE);
        paint.setTextSize(0.1);
        paint.setAntiAlias(true);
        const char text[] = "GooGLe small TEXT";
        size_t byteLength = strlen(static_cast<const char*>(text));

        canvas->drawText(text, byteLength, 0, 0, paint);
    }
};

DEF_SIMPLE_GM(google_text, canvas, 0, 0) {
	GoogleDraw g;
	g.drawGoogle(canvas);
}
