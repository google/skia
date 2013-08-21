/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfPageLabelDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfPageLabelDictionary::Type(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Type", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfPageLabelDictionary::has_Type() const {
  return get("Type", "") != NULL;
}

SkString SkPdfPageLabelDictionary::S(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("S", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfPageLabelDictionary::has_S() const {
  return get("S", "") != NULL;
}

SkString SkPdfPageLabelDictionary::P(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("P", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfPageLabelDictionary::has_P() const {
  return get("P", "") != NULL;
}

int64_t SkPdfPageLabelDictionary::St(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("St", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfPageLabelDictionary::has_St() const {
  return get("St", "") != NULL;
}
