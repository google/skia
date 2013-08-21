/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfSquareOrCircleAnnotation_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfSquareOrCircleAnnotation::Subtype(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Subtype", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfSquareOrCircleAnnotation::has_Subtype() const {
  return get("Subtype", "") != NULL;
}

SkString SkPdfSquareOrCircleAnnotation::Contents(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Contents", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfSquareOrCircleAnnotation::has_Contents() const {
  return get("Contents", "") != NULL;
}

SkPdfDictionary* SkPdfSquareOrCircleAnnotation::BS(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("BS", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfSquareOrCircleAnnotation::has_BS() const {
  return get("BS", "") != NULL;
}

SkPdfArray* SkPdfSquareOrCircleAnnotation::IC(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("IC", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfSquareOrCircleAnnotation::has_IC() const {
  return get("IC", "") != NULL;
}
