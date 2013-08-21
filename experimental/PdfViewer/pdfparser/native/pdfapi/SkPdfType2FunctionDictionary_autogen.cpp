/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfType2FunctionDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkPdfArray* SkPdfType2FunctionDictionary::C0(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("C0", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfType2FunctionDictionary::has_C0() const {
  return get("C0", "") != NULL;
}

SkPdfArray* SkPdfType2FunctionDictionary::C1(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("C1", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfType2FunctionDictionary::has_C1() const {
  return get("C1", "") != NULL;
}

double SkPdfType2FunctionDictionary::N(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("N", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfType2FunctionDictionary::has_N() const {
  return get("N", "") != NULL;
}
