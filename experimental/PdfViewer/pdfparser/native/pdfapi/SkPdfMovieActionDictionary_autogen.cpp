/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfMovieActionDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfMovieActionDictionary::S(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("S", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfMovieActionDictionary::has_S() const {
  return get("S", "") != NULL;
}

SkPdfDictionary* SkPdfMovieActionDictionary::Annot(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Annot", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfMovieActionDictionary::has_Annot() const {
  return get("Annot", "") != NULL;
}

SkString SkPdfMovieActionDictionary::T(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("T", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfMovieActionDictionary::has_T() const {
  return get("T", "") != NULL;
}

SkString SkPdfMovieActionDictionary::Operation(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Operation", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfMovieActionDictionary::has_Operation() const {
  return get("Operation", "") != NULL;
}
