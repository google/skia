/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfType0FunctionDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkPdfArray* SkPdfType0FunctionDictionary::Size(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Size", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfType0FunctionDictionary::has_Size() const {
  return get("Size", "") != NULL;
}

int64_t SkPdfType0FunctionDictionary::BitsPerSample(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("BitsPerSample", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfType0FunctionDictionary::has_BitsPerSample() const {
  return get("BitsPerSample", "") != NULL;
}

int64_t SkPdfType0FunctionDictionary::Order(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Order", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfType0FunctionDictionary::has_Order() const {
  return get("Order", "") != NULL;
}

SkPdfArray* SkPdfType0FunctionDictionary::Encode(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Encode", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfType0FunctionDictionary::has_Encode() const {
  return get("Encode", "") != NULL;
}

SkPdfArray* SkPdfType0FunctionDictionary::Decode(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Decode", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfType0FunctionDictionary::has_Decode() const {
  return get("Decode", "") != NULL;
}
