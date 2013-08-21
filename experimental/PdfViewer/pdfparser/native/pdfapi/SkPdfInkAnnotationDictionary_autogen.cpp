/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfInkAnnotationDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfInkAnnotationDictionary::Subtype(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Subtype", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfInkAnnotationDictionary::has_Subtype() const {
  return get("Subtype", "") != NULL;
}

SkString SkPdfInkAnnotationDictionary::Contents(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Contents", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfInkAnnotationDictionary::has_Contents() const {
  return get("Contents", "") != NULL;
}

SkPdfArray* SkPdfInkAnnotationDictionary::InkList(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("InkList", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfInkAnnotationDictionary::has_InkList() const {
  return get("InkList", "") != NULL;
}

SkPdfDictionary* SkPdfInkAnnotationDictionary::BS(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("BS", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfInkAnnotationDictionary::has_BS() const {
  return get("BS", "") != NULL;
}
