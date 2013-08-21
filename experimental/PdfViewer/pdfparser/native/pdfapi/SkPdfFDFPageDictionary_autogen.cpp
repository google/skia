/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfFDFPageDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkPdfArray* SkPdfFDFPageDictionary::Templates(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Templates", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfFDFPageDictionary::has_Templates() const {
  return get("Templates", "") != NULL;
}

SkPdfDictionary* SkPdfFDFPageDictionary::Info(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Info", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfFDFPageDictionary::has_Info() const {
  return get("Info", "") != NULL;
}
