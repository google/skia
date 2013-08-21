/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfMarkedContentReferenceDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfMarkedContentReferenceDictionary::Type(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Type", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfMarkedContentReferenceDictionary::has_Type() const {
  return get("Type", "") != NULL;
}

SkPdfDictionary* SkPdfMarkedContentReferenceDictionary::Pg(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Pg", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfMarkedContentReferenceDictionary::has_Pg() const {
  return get("Pg", "") != NULL;
}

SkPdfStream* SkPdfMarkedContentReferenceDictionary::Stm(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Stm", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfMarkedContentReferenceDictionary::has_Stm() const {
  return get("Stm", "") != NULL;
}

SkPdfNativeObject* SkPdfMarkedContentReferenceDictionary::StmOwn(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("StmOwn", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && true) || (doc == NULL && ret != NULL && ret->isReference())) return ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfMarkedContentReferenceDictionary::has_StmOwn() const {
  return get("StmOwn", "") != NULL;
}

int64_t SkPdfMarkedContentReferenceDictionary::MCID(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("MCID", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfMarkedContentReferenceDictionary::has_MCID() const {
  return get("MCID", "") != NULL;
}
