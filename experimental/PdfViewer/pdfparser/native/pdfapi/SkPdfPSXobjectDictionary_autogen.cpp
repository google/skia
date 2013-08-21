/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfPSXobjectDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfPSXobjectDictionary::Type(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Type", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfPSXobjectDictionary::has_Type() const {
  return get("Type", "") != NULL;
}

SkString SkPdfPSXobjectDictionary::Subtype(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Subtype", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfPSXobjectDictionary::has_Subtype() const {
  return get("Subtype", "") != NULL;
}

SkPdfStream* SkPdfPSXobjectDictionary::Level1(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Level1", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfPSXobjectDictionary::has_Level1() const {
  return get("Level1", "") != NULL;
}
