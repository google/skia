/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAnnotation.h"
#include "SkDataSet.h"
#include "SkFlattenableBuffers.h"
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

SK_DEFINE_FLATTENABLE_REGISTRAR(SkAnnotation)

const char* SkAnnotationKeys::URL_Key() {
    return "SkAnnotationKey_URL";
};

///////////////////////////////////////////////////////////////////////////////

#include "SkCanvas.h"

void SkAnnotateRectWithURL(SkCanvas* canvas, const SkRect& rect, SkData* value) {
    if (NULL == value) {
        return;
    }

    const char* key = SkAnnotationKeys::URL_Key();
    SkAutoTUnref<SkDataSet> dataset(SkNEW_ARGS(SkDataSet, (key, value)));
    SkAnnotation* ann = SkNEW_ARGS(SkAnnotation, (dataset,
                                                  SkAnnotation::kNoDraw_Flag));

    SkPaint paint;
    paint.setAnnotation(ann)->unref();
    SkASSERT(paint.isNoDrawAnnotation());

    canvas->drawRect(rect, paint);
}

