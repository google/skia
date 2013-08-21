/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfFDFTemplateDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkPdfDictionary* SkPdfFDFTemplateDictionary::TRef(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TRef", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfFDFTemplateDictionary::has_TRef() const {
  return get("TRef", "") != NULL;
}

SkPdfArray* SkPdfFDFTemplateDictionary::Fields(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Fields", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfFDFTemplateDictionary::has_Fields() const {
  return get("Fields", "") != NULL;
}

bool SkPdfFDFTemplateDictionary::Rename(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Rename", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isBoolean()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->boolValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return false;
}

bool SkPdfFDFTemplateDictionary::has_Rename() const {
  return get("Rename", "") != NULL;
}
