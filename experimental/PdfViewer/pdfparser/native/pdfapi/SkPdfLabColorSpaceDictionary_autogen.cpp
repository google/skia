/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfLabColorSpaceDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkPdfArray* SkPdfLabColorSpaceDictionary::WhitePoint(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("WhitePoint", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfLabColorSpaceDictionary::has_WhitePoint() const {
  return get("WhitePoint", "") != NULL;
}

SkPdfArray* SkPdfLabColorSpaceDictionary::BlackPoint(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("BlackPoint", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfLabColorSpaceDictionary::has_BlackPoint() const {
  return get("BlackPoint", "") != NULL;
}

SkPdfArray* SkPdfLabColorSpaceDictionary::Range(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Range", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfLabColorSpaceDictionary::has_Range() const {
  return get("Range", "") != NULL;
}
