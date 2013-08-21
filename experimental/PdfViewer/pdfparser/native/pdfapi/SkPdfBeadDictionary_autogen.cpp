/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfBeadDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfBeadDictionary::Type(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Type", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfBeadDictionary::has_Type() const {
  return get("Type", "") != NULL;
}

SkPdfDictionary* SkPdfBeadDictionary::T(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("T", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfBeadDictionary::has_T() const {
  return get("T", "") != NULL;
}

SkPdfDictionary* SkPdfBeadDictionary::N(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("N", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfBeadDictionary::has_N() const {
  return get("N", "") != NULL;
}

SkPdfDictionary* SkPdfBeadDictionary::V(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("V", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfBeadDictionary::has_V() const {
  return get("V", "") != NULL;
}

SkPdfDictionary* SkPdfBeadDictionary::P(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("P", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfBeadDictionary::has_P() const {
  return get("P", "") != NULL;
}

SkRect SkPdfBeadDictionary::R(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("R", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isRectangle()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->rectangleValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkRect::MakeEmpty();
}

bool SkPdfBeadDictionary::has_R() const {
  return get("R", "") != NULL;
}
