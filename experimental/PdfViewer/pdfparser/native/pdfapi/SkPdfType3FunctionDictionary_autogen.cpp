/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfType3FunctionDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkPdfArray* SkPdfType3FunctionDictionary::Functions(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Functions", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfType3FunctionDictionary::has_Functions() const {
  return get("Functions", "") != NULL;
}

SkPdfArray* SkPdfType3FunctionDictionary::Bounds(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Bounds", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfType3FunctionDictionary::has_Bounds() const {
  return get("Bounds", "") != NULL;
}

SkPdfArray* SkPdfType3FunctionDictionary::Encode(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Encode", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfType3FunctionDictionary::has_Encode() const {
  return get("Encode", "") != NULL;
}
