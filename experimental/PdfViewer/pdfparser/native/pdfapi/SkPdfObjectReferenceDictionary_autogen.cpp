/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfObjectReferenceDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfObjectReferenceDictionary::Type(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Type", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfObjectReferenceDictionary::has_Type() const {
  return get("Type", "") != NULL;
}

SkPdfDictionary* SkPdfObjectReferenceDictionary::Pg(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Pg", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfObjectReferenceDictionary::has_Pg() const {
  return get("Pg", "") != NULL;
}

SkPdfNativeObject* SkPdfObjectReferenceDictionary::Obj(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Obj", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && true) || (doc == NULL && ret != NULL && ret->isReference())) return ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfObjectReferenceDictionary::has_Obj() const {
  return get("Obj", "") != NULL;
}
