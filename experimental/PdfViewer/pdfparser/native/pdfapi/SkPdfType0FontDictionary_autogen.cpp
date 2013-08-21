/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfType0FontDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfType0FontDictionary::Type(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Type", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfType0FontDictionary::has_Type() const {
  return get("Type", "") != NULL;
}

SkString SkPdfType0FontDictionary::Subtype(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Subtype", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfType0FontDictionary::has_Subtype() const {
  return get("Subtype", "") != NULL;
}

SkString SkPdfType0FontDictionary::BaseFont(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("BaseFont", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfType0FontDictionary::has_BaseFont() const {
  return get("BaseFont", "") != NULL;
}

bool SkPdfType0FontDictionary::isEncodingAName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Encoding", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isName();
}

SkString SkPdfType0FontDictionary::getEncodingAsName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Encoding", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfType0FontDictionary::isEncodingAStream(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Encoding", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->hasStream();
}

SkPdfStream* SkPdfType0FontDictionary::getEncodingAsStream(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Encoding", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfType0FontDictionary::has_Encoding() const {
  return get("Encoding", "") != NULL;
}

SkPdfArray* SkPdfType0FontDictionary::DescendantFonts(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("DescendantFonts", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfType0FontDictionary::has_DescendantFonts() const {
  return get("DescendantFonts", "") != NULL;
}

SkPdfStream* SkPdfType0FontDictionary::ToUnicode(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ToUnicode", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfType0FontDictionary::has_ToUnicode() const {
  return get("ToUnicode", "") != NULL;
}
