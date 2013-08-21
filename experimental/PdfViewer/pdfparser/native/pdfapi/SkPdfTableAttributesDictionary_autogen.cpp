/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfTableAttributesDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

int64_t SkPdfTableAttributesDictionary::RowSpan(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("RowSpan", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfTableAttributesDictionary::has_RowSpan() const {
  return get("RowSpan", "") != NULL;
}

int64_t SkPdfTableAttributesDictionary::ColSpan(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ColSpan", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfTableAttributesDictionary::has_ColSpan() const {
  return get("ColSpan", "") != NULL;
}
