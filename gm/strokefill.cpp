/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkTextFormatParams.h"
#include "tools/ToolUtils.h"

DEF_SIMPLE_GM(bug339297, canvas, 640, 480) {
    SkPath path;
    path.moveTo(-469515, -10354890);
    path.cubicTo(771919.62f, -10411179, 2013360.1f, -10243774, 3195542.8f, -9860664);
    path.lineTo(3195550, -9860655);
    path.lineTo(3195539, -9860652);
    path.lineTo(3195539, -9860652);
    path.lineTo(3195539, -9860652);
    path.cubicTo(2013358.1f, -10243761, 771919.25f, -10411166, -469513.84f, -10354877);
    path.lineTo(-469515, -10354890);
    path.close();

    canvas->translate(258, 10365663);

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorBLACK);
    paint.setStyle(SkPaint::kFill_Style);
    canvas->drawPath(path, paint);

    paint.setColor(SK_ColorRED);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(1);
    canvas->drawPath(path, paint);
}

DEF_SIMPLE_GM(bug339297_as_clip, canvas, 640, 480) {
    SkPath path;
    path.moveTo(-469515, -10354890);
    path.cubicTo(771919.62f, -10411179, 2013360.1f, -10243774, 3195542.8f, -9860664);
    path.lineTo(3195550, -9860655);
    path.lineTo(3195539, -9860652);
    path.lineTo(3195539, -9860652);
    path.lineTo(3195539, -9860652);
    path.cubicTo(2013358.1f, -10243761, 771919.25f, -10411166, -469513.84f, -10354877);
    path.lineTo(-469515, -10354890);
    path.close();

    canvas->translate(258, 10365663);

    canvas->save();
    canvas->clipPath(path, true);
    canvas->clear(SK_ColorBLACK);
    canvas->restore();

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kFill_Style);
    paint.setColor(SK_ColorRED);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(1);
    canvas->drawPath(path, paint);
}

DEF_SIMPLE_GM(bug6987, canvas, 200, 200) {
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(0.0001f);
    paint.setAntiAlias(true);
    SkPath path;
    canvas->save();
    canvas->scale(50000.0f, 50000.0f);
    path.moveTo(0.0005f, 0.0004f);
    path.lineTo(0.0008f, 0.0010f);
    path.lineTo(0.0002f, 0.0010f);
    path.close();
    canvas->drawPath(path, paint);
    canvas->restore();
}
