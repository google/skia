/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfOutlineDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfOutlineDictionary::Type(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Type", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfOutlineDictionary::has_Type() const {
  return get("Type", "") != NULL;
}

SkPdfDictionary* SkPdfOutlineDictionary::First(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("First", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfOutlineDictionary::has_First() const {
  return get("First", "") != NULL;
}

SkPdfDictionary* SkPdfOutlineDictionary::Last(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Last", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfOutlineDictionary::has_Last() const {
  return get("Last", "") != NULL;
}

int64_t SkPdfOutlineDictionary::Count(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Count", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfOutlineDictionary::has_Count() const {
  return get("Count", "") != NULL;
}
