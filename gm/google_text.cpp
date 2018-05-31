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

class GoogleDraw {
public:
	
	static SkMatrix getPerspective() {
		SkPoint fPerspectivePoints[4];
		fPerspectivePoints[0].set(0, 0);
		fPerspectivePoints[1].set(1, 0);
		fPerspectivePoints[2].set(0, 1);
		fPerspectivePoints[3].set(1, 1);
		
		SkScalar w = 1000, h = 1000;
		SkPoint orthoPts[4] = { { 0, 0 }, { w, 0 }, { 0, h }, { w, h } };
		SkPoint perspPts[4] = {
				{ 500, fPerspectivePoints[0].fY * h },
				{ fPerspectivePoints[1].fX * w, fPerspectivePoints[1].fY * h },
				{ fPerspectivePoints[2].fX * w, fPerspectivePoints[2].fY * h },
				{ fPerspectivePoints[3].fX * w, fPerspectivePoints[3].fY * h }
		};
		SkMatrix m;
		m.setPolyToPoly(orthoPts, perspPts, 4);
		return m;
	}
	
	static void drawGoogle(SkCanvas* canvas) {
		canvas->drawColor(SK_ColorBLACK);
		canvas->concat(getPerspective());
		SkPaint paint;
		paint.setAntiAlias(true);
		paint.setStyle(SkPaint::kStroke_Style);
		paint.setStrokeWidth(4);
		
		//G
		SkPaint paintText;
		paintText.setColor(SK_ColorBLUE);
		paintText.setTextSize(80);
		const char text[] = "G";
		canvas->drawText(text, strlen(text), 0, 80, paintText);
		
		//O
		SkRect rect = SkRect::MakeXYWH(80, 80, 40, 40);
		paint.setColor(SK_ColorRED);
		canvas->drawRect(rect, paint);
		
		//O
		SkRRect oval;
		oval.setOval(rect);
		oval.offset(40, 40);
		paint.setColor(SK_ColorYELLOW);
		canvas->drawRRect(oval, paint);
		
		//G
		paintText.setColor(SK_ColorBLUE);
		canvas->drawText(text, strlen(text), 200, 250, paintText);
		
		//L
		SkPath path;
		path.moveTo(250, 300);
		path.lineTo(250, 350);
		path.moveTo(250, 350);
		path.lineTo(300, 350);
		paint.setColor(SK_ColorGREEN);
		canvas->drawPath(path, paint);
		
		//E
		path.reset();
		int originX = 400;
		int originY = 400;
		path.moveTo(originX, originY);
		path.lineTo(originX + 50, originY);
		path.moveTo(originX, originY);
		path.lineTo(originX, originY + 100);
		path.moveTo(originX, originY + 50);
		path.lineTo(originX + 50, originY + 50);
		path.moveTo(originX, originY + 100);
		path.lineTo(originX + 50, originY + 100);
		paint.setColor(SK_ColorRED);
		canvas->drawPath(path, paint);
	}
};

DEF_SIMPLE_GM(google_text, canvas, 50, 50) {
	GoogleDraw g;
	g.drawGoogle(canvas);
}