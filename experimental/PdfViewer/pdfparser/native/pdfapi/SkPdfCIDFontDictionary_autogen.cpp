/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfCIDFontDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfCIDFontDictionary::Type(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Type", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfCIDFontDictionary::has_Type() const {
  return get("Type", "") != NULL;
}

SkString SkPdfCIDFontDictionary::Subtype(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Subtype", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfCIDFontDictionary::has_Subtype() const {
  return get("Subtype", "") != NULL;
}

SkString SkPdfCIDFontDictionary::BaseFont(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("BaseFont", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfCIDFontDictionary::has_BaseFont() const {
  return get("BaseFont", "") != NULL;
}

SkPdfDictionary* SkPdfCIDFontDictionary::CIDSystemInfo(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("CIDSystemInfo", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfCIDFontDictionary::has_CIDSystemInfo() const {
  return get("CIDSystemInfo", "") != NULL;
}

SkPdfFontDescriptorDictionary* SkPdfCIDFontDictionary::FontDescriptor(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("FontDescriptor", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary() && ((SkPdfFontDescriptorDictionary*)ret)->valid()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfFontDescriptorDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfCIDFontDictionary::has_FontDescriptor() const {
  return get("FontDescriptor", "") != NULL;
}

int64_t SkPdfCIDFontDictionary::DW(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("DW", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfCIDFontDictionary::has_DW() const {
  return get("DW", "") != NULL;
}

SkPdfArray* SkPdfCIDFontDictionary::W(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("W", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfCIDFontDictionary::has_W() const {
  return get("W", "") != NULL;
}

SkPdfArray* SkPdfCIDFontDictionary::DW2(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("DW2", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfCIDFontDictionary::has_DW2() const {
  return get("DW2", "") != NULL;
}

SkPdfArray* SkPdfCIDFontDictionary::W2(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("W2", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfCIDFontDictionary::has_W2() const {
  return get("W2", "") != NULL;
}

bool SkPdfCIDFontDictionary::isCIDToGIDMapAStream(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("CIDToGIDMap", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->hasStream();
}

SkPdfStream* SkPdfCIDFontDictionary::getCIDToGIDMapAsStream(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("CIDToGIDMap", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfCIDFontDictionary::isCIDToGIDMapAName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("CIDToGIDMap", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isName();
}

SkString SkPdfCIDFontDictionary::getCIDToGIDMapAsName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("CIDToGIDMap", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfCIDFontDictionary::has_CIDToGIDMap() const {
  return get("CIDToGIDMap", "") != NULL;
}
