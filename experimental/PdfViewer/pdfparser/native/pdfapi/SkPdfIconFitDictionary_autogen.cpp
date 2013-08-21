/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfIconFitDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfIconFitDictionary::SW(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("SW", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfIconFitDictionary::has_SW() const {
  return get("SW", "") != NULL;
}

SkString SkPdfIconFitDictionary::S(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("S", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfIconFitDictionary::has_S() const {
  return get("S", "") != NULL;
}

SkPdfArray* SkPdfIconFitDictionary::A(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("A", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfIconFitDictionary::has_A() const {
  return get("A", "") != NULL;
}
