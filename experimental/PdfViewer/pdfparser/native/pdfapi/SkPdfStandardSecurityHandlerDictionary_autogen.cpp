/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfStandardSecurityHandlerDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

double SkPdfStandardSecurityHandlerDictionary::R(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("R", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfStandardSecurityHandlerDictionary::has_R() const {
  return get("R", "") != NULL;
}

SkString SkPdfStandardSecurityHandlerDictionary::O(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("O", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfStandardSecurityHandlerDictionary::has_O() const {
  return get("O", "") != NULL;
}

SkString SkPdfStandardSecurityHandlerDictionary::U(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("U", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfStandardSecurityHandlerDictionary::has_U() const {
  return get("U", "") != NULL;
}

int64_t SkPdfStandardSecurityHandlerDictionary::P(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("P", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfStandardSecurityHandlerDictionary::has_P() const {
  return get("P", "") != NULL;
}
