/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAnnotation.h"
#include "SkData.h"
#include "SkPaint.h"
#include "SkPoint.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

SkAnnotation::SkAnnotation(const char key[], SkData* value) : fKey(key) {
    if (nullptr == value) {
        value = SkData::NewEmpty();
    } else {
        value->ref();
    }
    fData = value;
}

SkAnnotation::~SkAnnotation() {
    fData->unref();
}

SkData* SkAnnotation::find(const char key[]) const {
    return fKey.equals(key) ? fData : nullptr;
}

SkAnnotation::SkAnnotation(SkReadBuffer& buffer) {
    buffer.readString(&fKey);
    fData = buffer.readByteArrayAsData();
}

void SkAnnotation::writeToBuffer(SkWriteBuffer& buffer) const {
    buffer.writeString(fKey.c_str());
    buffer.writeDataAsByteArray(fData);
}

const char* SkAnnotationKeys::URL_Key() {
    return "SkAnnotationKey_URL";
};

const char* SkAnnotationKeys::Define_Named_Dest_Key() {
    return "SkAnnotationKey_Define_Named_Dest";
};

const char* SkAnnotationKeys::Link_Named_Dest_Key() {
    return "SkAnnotationKey_Link_Named_Dest";
};

///////////////////////////////////////////////////////////////////////////////

#include "SkCanvas.h"

static void annotate_paint(SkPaint& paint, const char* key, SkData* value) {
    paint.setAnnotation(SkAnnotation::Create(key, value))->unref();
}

void SkAnnotateRectWithURL(SkCanvas* canvas, const SkRect& rect, SkData* value) {
    if (nullptr == value) {
        return;
    }
    SkPaint paint;
    annotate_paint(paint, SkAnnotationKeys::URL_Key(), value);
    canvas->drawRect(rect, paint);
}

void SkAnnotateNamedDestination(SkCanvas* canvas, const SkPoint& point, SkData* name) {
    if (nullptr == name) {
        return;
    }
    SkPaint paint;
    annotate_paint(paint, SkAnnotationKeys::Define_Named_Dest_Key(), name);
    canvas->drawPoint(point.x(), point.y(), paint);
}

void SkAnnotateLinkToDestination(SkCanvas* canvas, const SkRect& rect, SkData* name) {
    if (nullptr == name) {
        return;
    }
    SkPaint paint;
    annotate_paint(paint, SkAnnotationKeys::Link_Named_Dest_Key(), name);
    canvas->drawRect(rect, paint);
}
