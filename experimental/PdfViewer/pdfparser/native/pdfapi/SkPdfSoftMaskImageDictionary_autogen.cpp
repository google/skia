/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfSoftMaskImageDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkPdfArray* SkPdfSoftMaskImageDictionary::Matte(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Matte", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfSoftMaskImageDictionary::has_Matte() const {
  return get("Matte", "") != NULL;
}

SkString SkPdfSoftMaskImageDictionary::Subtype(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Subtype", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfSoftMaskImageDictionary::has_Subtype() const {
  return get("Subtype", "") != NULL;
}

SkString SkPdfSoftMaskImageDictionary::ColorSpace(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ColorSpace", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfSoftMaskImageDictionary::has_ColorSpace() const {
  return get("ColorSpace", "") != NULL;
}
