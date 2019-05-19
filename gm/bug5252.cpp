/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"

DEF_SIMPLE_GM(bug5252, canvas, 500, 500) {
	canvas->translate(SkIntToScalar(10), SkIntToScalar(20));

	SkPath clip1;
	clip1.addOval(SkRect::MakeWH(225, 200));
	canvas->clipPath(clip1); // bug

	SkPath clip2;
	clip2.addRect(SkRect::MakeWH(220, 200));
	//canvas->clipPath(clip2); // ok

	SkPaint pa;
	pa.setStyle(SkPaint::kStroke_Style);
	pa.setAntiAlias(true);
	pa.setStrokeWidth(1.0f);
	for (int i = 0; i < 15; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			SkAutoCanvasRestore acs(canvas, true);

			canvas->translate(i * 15.f, j * 20.f);
			canvas->drawRect(SkRect::MakeXYWH(5, 5, 10, 15),pa);
			SkPath path;
			path.moveTo(6, 6);
			path.cubicTo(14, 10, 13, 12, 10, 12);
			path.cubicTo(7, 15, 8, 17, 14, 18);
			canvas->drawPath(path, pa);
		}
	}
}
