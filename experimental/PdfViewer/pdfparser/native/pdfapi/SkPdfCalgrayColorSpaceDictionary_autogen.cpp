/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfCalgrayColorSpaceDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkPdfArray* SkPdfCalgrayColorSpaceDictionary::WhitePoint(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("WhitePoint", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfCalgrayColorSpaceDictionary::has_WhitePoint() const {
  return get("WhitePoint", "") != NULL;
}

SkPdfArray* SkPdfCalgrayColorSpaceDictionary::BlackPoint(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("BlackPoint", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfCalgrayColorSpaceDictionary::has_BlackPoint() const {
  return get("BlackPoint", "") != NULL;
}

double SkPdfCalgrayColorSpaceDictionary::Gamma(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Gamma", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfCalgrayColorSpaceDictionary::has_Gamma() const {
  return get("Gamma", "") != NULL;
}
