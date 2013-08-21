/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfCIDFontDescriptorDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkPdfDictionary* SkPdfCIDFontDescriptorDictionary::Style(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Style", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfCIDFontDescriptorDictionary::has_Style() const {
  return get("Style", "") != NULL;
}

SkString SkPdfCIDFontDescriptorDictionary::Lang(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Lang", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfCIDFontDescriptorDictionary::has_Lang() const {
  return get("Lang", "") != NULL;
}

SkPdfDictionary* SkPdfCIDFontDescriptorDictionary::FD(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("FD", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfCIDFontDescriptorDictionary::has_FD() const {
  return get("FD", "") != NULL;
}

SkPdfStream* SkPdfCIDFontDescriptorDictionary::CIDSet(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("CIDSet", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfCIDFontDescriptorDictionary::has_CIDSet() const {
  return get("CIDSet", "") != NULL;
}
