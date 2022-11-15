/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAnnotation.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "src/core/SkAnnotationKeys.h"

const char* SkAnnotationKeys::URL_Key() {
    return "SkAnnotationKey_URL";
}

const char* SkAnnotationKeys::Define_Named_Dest_Key() {
    return "SkAnnotationKey_Define_Named_Dest";
}

const char* SkAnnotationKeys::Link_Named_Dest_Key() {
    return "SkAnnotationKey_Link_Named_Dest";
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void SkAnnotateRectWithURL(SkCanvas* canvas, const SkRect& rect, SkData* value) {
    if (nullptr == value) {
        return;
    }
    canvas->drawAnnotation(rect, SkAnnotationKeys::URL_Key(), value);
}

void SkAnnotateNamedDestination(SkCanvas* canvas, const SkPoint& point, SkData* name) {
    if (nullptr == name) {
        return;
    }
    const SkRect rect = SkRect::MakeXYWH(point.x(), point.y(), 0, 0);
    canvas->drawAnnotation(rect, SkAnnotationKeys::Define_Named_Dest_Key(), name);
}

void SkAnnotateLinkToDestination(SkCanvas* canvas, const SkRect& rect, SkData* name) {
    if (nullptr == name) {
        return;
    }
    canvas->drawAnnotation(rect, SkAnnotationKeys::Link_Named_Dest_Key(), name);
}
