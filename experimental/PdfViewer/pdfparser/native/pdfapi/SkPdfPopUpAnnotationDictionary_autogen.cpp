/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfPopUpAnnotationDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfPopUpAnnotationDictionary::Subtype(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Subtype", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfPopUpAnnotationDictionary::has_Subtype() const {
  return get("Subtype", "") != NULL;
}

SkString SkPdfPopUpAnnotationDictionary::Contents(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Contents", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfPopUpAnnotationDictionary::has_Contents() const {
  return get("Contents", "") != NULL;
}

SkPdfDictionary* SkPdfPopUpAnnotationDictionary::Parent(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Parent", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfPopUpAnnotationDictionary::has_Parent() const {
  return get("Parent", "") != NULL;
}

bool SkPdfPopUpAnnotationDictionary::Open(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Open", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isBoolean()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->boolValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return false;
}

bool SkPdfPopUpAnnotationDictionary::has_Open() const {
  return get("Open", "") != NULL;
}
