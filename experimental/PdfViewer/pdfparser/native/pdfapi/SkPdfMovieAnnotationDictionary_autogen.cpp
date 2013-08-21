/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfMovieAnnotationDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfMovieAnnotationDictionary::Subtype(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Subtype", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfMovieAnnotationDictionary::has_Subtype() const {
  return get("Subtype", "") != NULL;
}

SkPdfDictionary* SkPdfMovieAnnotationDictionary::Movie(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Movie", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfMovieAnnotationDictionary::has_Movie() const {
  return get("Movie", "") != NULL;
}

SkString SkPdfMovieAnnotationDictionary::Contents(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Contents", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfMovieAnnotationDictionary::has_Contents() const {
  return get("Contents", "") != NULL;
}

bool SkPdfMovieAnnotationDictionary::isAABoolean(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("A", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isBoolean();
}

bool SkPdfMovieAnnotationDictionary::getAAsBoolean(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("A", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isBoolean()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->boolValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return false;
}

bool SkPdfMovieAnnotationDictionary::isAADictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("A", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isDictionary();
}

SkPdfDictionary* SkPdfMovieAnnotationDictionary::getAAsDictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("A", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfMovieAnnotationDictionary::has_A() const {
  return get("A", "") != NULL;
}
