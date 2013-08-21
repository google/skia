/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfType2PatternDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

int64_t SkPdfType2PatternDictionary::Type(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Type", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfType2PatternDictionary::has_Type() const {
  return get("Type", "") != NULL;
}

int64_t SkPdfType2PatternDictionary::PatternType(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("PatternType", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfType2PatternDictionary::has_PatternType() const {
  return get("PatternType", "") != NULL;
}

bool SkPdfType2PatternDictionary::isShadingADictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Shading", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isDictionary();
}

SkPdfDictionary* SkPdfType2PatternDictionary::getShadingAsDictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Shading", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfType2PatternDictionary::isShadingAStream(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Shading", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->hasStream();
}

SkPdfStream* SkPdfType2PatternDictionary::getShadingAsStream(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Shading", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfType2PatternDictionary::has_Shading() const {
  return get("Shading", "") != NULL;
}

SkPdfArray* SkPdfType2PatternDictionary::Matrix(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Matrix", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfType2PatternDictionary::has_Matrix() const {
  return get("Matrix", "") != NULL;
}

SkPdfDictionary* SkPdfType2PatternDictionary::ExtGState(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ExtGState", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfType2PatternDictionary::has_ExtGState() const {
  return get("ExtGState", "") != NULL;
}
