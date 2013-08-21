/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfVariableTextFieldDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkPdfDictionary* SkPdfVariableTextFieldDictionary::DR(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("DR", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfVariableTextFieldDictionary::has_DR() const {
  return get("DR", "") != NULL;
}

SkString SkPdfVariableTextFieldDictionary::DA(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("DA", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfVariableTextFieldDictionary::has_DA() const {
  return get("DA", "") != NULL;
}

int64_t SkPdfVariableTextFieldDictionary::Q(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Q", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfVariableTextFieldDictionary::has_Q() const {
  return get("Q", "") != NULL;
}
