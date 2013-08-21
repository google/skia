/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfType4ShadingDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

int64_t SkPdfType4ShadingDictionary::BitsPerCoordinate(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("BitsPerCoordinate", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfType4ShadingDictionary::has_BitsPerCoordinate() const {
  return get("BitsPerCoordinate", "") != NULL;
}

int64_t SkPdfType4ShadingDictionary::BitsPerComponent(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("BitsPerComponent", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfType4ShadingDictionary::has_BitsPerComponent() const {
  return get("BitsPerComponent", "") != NULL;
}

int64_t SkPdfType4ShadingDictionary::BitsPerFlag(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("BitsPerFlag", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfType4ShadingDictionary::has_BitsPerFlag() const {
  return get("BitsPerFlag", "") != NULL;
}

SkRect SkPdfType4ShadingDictionary::Decode(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Decode", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isRectangle()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->rectangleValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkRect::MakeEmpty();
}

bool SkPdfType4ShadingDictionary::has_Decode() const {
  return get("Decode", "") != NULL;
}

SkPdfFunction SkPdfType4ShadingDictionary::Function(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Function", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isFunction()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->functionValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkPdfFunction();
}

bool SkPdfType4ShadingDictionary::has_Function() const {
  return get("Function", "") != NULL;
}
