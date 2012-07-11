/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAnnotation.h"
#include "SkDataSet.h"
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

SkAnnotation::SkAnnotation(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {
    fFlags = buffer.readU32();
    fDataSet = SkNEW_ARGS(SkDataSet, (buffer));
}

void SkAnnotation::flatten(SkFlattenableWriteBuffer& buffer) const {
    buffer.write32(fFlags);
    fDataSet->flatten(buffer);
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

