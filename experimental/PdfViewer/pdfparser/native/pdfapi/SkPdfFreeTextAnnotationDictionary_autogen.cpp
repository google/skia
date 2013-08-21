/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfFreeTextAnnotationDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfFreeTextAnnotationDictionary::Subtype(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Subtype", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfFreeTextAnnotationDictionary::has_Subtype() const {
  return get("Subtype", "") != NULL;
}

SkString SkPdfFreeTextAnnotationDictionary::Contents(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Contents", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfFreeTextAnnotationDictionary::has_Contents() const {
  return get("Contents", "") != NULL;
}

SkString SkPdfFreeTextAnnotationDictionary::DA(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("DA", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfFreeTextAnnotationDictionary::has_DA() const {
  return get("DA", "") != NULL;
}

int64_t SkPdfFreeTextAnnotationDictionary::Q(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Q", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfFreeTextAnnotationDictionary::has_Q() const {
  return get("Q", "") != NULL;
}
