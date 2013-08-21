/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfType5ShadingDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

int64_t SkPdfType5ShadingDictionary::BitsPerCoordinate(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("BitsPerCoordinate", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfType5ShadingDictionary::has_BitsPerCoordinate() const {
  return get("BitsPerCoordinate", "") != NULL;
}

int64_t SkPdfType5ShadingDictionary::BitsPerComponent(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("BitsPerComponent", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfType5ShadingDictionary::has_BitsPerComponent() const {
  return get("BitsPerComponent", "") != NULL;
}

int64_t SkPdfType5ShadingDictionary::VerticesPerRow(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("VerticesPerRow", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfType5ShadingDictionary::has_VerticesPerRow() const {
  return get("VerticesPerRow", "") != NULL;
}

SkPdfArray* SkPdfType5ShadingDictionary::Decode(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Decode", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfType5ShadingDictionary::has_Decode() const {
  return get("Decode", "") != NULL;
}

SkPdfFunction SkPdfType5ShadingDictionary::Function(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Function", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isFunction()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->functionValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkPdfFunction();
}

bool SkPdfType5ShadingDictionary::has_Function() const {
  return get("Function", "") != NULL;
}
