/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAnnotation.h"
#include "SkDataSet.h"
#include "SkFlattenableBuffers.h"
#include "SkPoint.h"
#include "SkStream.h"

SkAnnotation::SkAnnotation(SkDataSet* data, uint32_t flags) {
    if (NULL == data) {
        data = SkDataSet::NewEmpty();
    } else {
        data->ref();
    }
    fDataSet = data;
    fFlags = flags;
}

SkAnnotation::~SkAnnotation() {
    fDataSet->unref();
}

SkData* SkAnnotation::find(const char name[]) const {
    return fDataSet->find(name);
}

SkAnnotation::SkAnnotation(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {
    fFlags = buffer.readUInt();
    fDataSet = buffer.readFlattenableT<SkDataSet>();
}

void SkAnnotation::flatten(SkFlattenableWriteBuffer& buffer) const {
    buffer.writeUInt(fFlags);
    buffer.writeFlattenable(fDataSet);
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
    SkAutoTUnref<SkDataSet> dataset(SkNEW_ARGS(SkDataSet, (key, value)));
    SkAnnotation* ann = SkNEW_ARGS(SkAnnotation, (dataset,
                                                  SkAnnotation::kNoDraw_Flag));

    paint.setAnnotation(ann)->unref();
    SkASSERT(paint.isNoDrawAnnotation());
}

void SkAnnotateRectWithURL(SkCanvas* canvas, const SkRect& rect, SkData* value) {
    if (NULL == value) {
        return;
    }
    SkPaint paint;
    annotate_paint(paint, SkAnnotationKeys::URL_Key(), value);
    canvas->drawRect(rect, paint);
}

void SkAnnotateNamedDestination(SkCanvas* canvas, const SkPoint& point, SkData* name) {
    if (NULL == name) {
        return;
    }
    SkPaint paint;
    annotate_paint(paint, SkAnnotationKeys::Define_Named_Dest_Key(), name);
    canvas->drawPoint(point.x(), point.y(), paint);
}

void SkAnnotateLinkToDestination(SkCanvas* canvas, const SkRect& rect, SkData* name) {
    if (NULL == name) {
        return;
    }
    SkPaint paint;
    annotate_paint(paint, SkAnnotationKeys::Link_Named_Dest_Key(), name);
    canvas->drawRect(rect, paint);
}
