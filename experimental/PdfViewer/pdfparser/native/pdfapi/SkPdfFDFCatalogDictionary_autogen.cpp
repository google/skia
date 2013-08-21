/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfFDFCatalogDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfFDFCatalogDictionary::Version(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Version", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfFDFCatalogDictionary::has_Version() const {
  return get("Version", "") != NULL;
}

SkPdfDictionary* SkPdfFDFCatalogDictionary::FDF(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("FDF", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfFDFCatalogDictionary::has_FDF() const {
  return get("FDF", "") != NULL;
}
