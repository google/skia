/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfType1PatternDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfType1PatternDictionary::Type(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Type", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfType1PatternDictionary::has_Type() const {
  return get("Type", "") != NULL;
}

int64_t SkPdfType1PatternDictionary::PatternType(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("PatternType", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfType1PatternDictionary::has_PatternType() const {
  return get("PatternType", "") != NULL;
}

int64_t SkPdfType1PatternDictionary::PaintType(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("PaintType", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfType1PatternDictionary::has_PaintType() const {
  return get("PaintType", "") != NULL;
}

int64_t SkPdfType1PatternDictionary::TilingType(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TilingType", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfType1PatternDictionary::has_TilingType() const {
  return get("TilingType", "") != NULL;
}

SkRect SkPdfType1PatternDictionary::BBox(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("BBox", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isRectangle()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->rectangleValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkRect::MakeEmpty();
}

bool SkPdfType1PatternDictionary::has_BBox() const {
  return get("BBox", "") != NULL;
}

double SkPdfType1PatternDictionary::XStep(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("XStep", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfType1PatternDictionary::has_XStep() const {
  return get("XStep", "") != NULL;
}

double SkPdfType1PatternDictionary::YStep(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("YStep", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfType1PatternDictionary::has_YStep() const {
  return get("YStep", "") != NULL;
}

SkPdfResourceDictionary* SkPdfType1PatternDictionary::Resources(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Resources", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary() && ((SkPdfResourceDictionary*)ret)->valid()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfResourceDictionary*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfType1PatternDictionary::has_Resources() const {
  return get("Resources", "") != NULL;
}

SkMatrix SkPdfType1PatternDictionary::Matrix(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Matrix", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isMatrix()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->matrixValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkMatrix::I();
}

bool SkPdfType1PatternDictionary::has_Matrix() const {
  return get("Matrix", "") != NULL;
}
