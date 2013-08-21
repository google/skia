/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfType1FontDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfType1FontDictionary::Type(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Type", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfType1FontDictionary::has_Type() const {
  return get("Type", "") != NULL;
}

SkString SkPdfType1FontDictionary::Subtype(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Subtype", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfType1FontDictionary::has_Subtype() const {
  return get("Subtype", "") != NULL;
}

SkString SkPdfType1FontDictionary::Name(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Name", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfType1FontDictionary::has_Name() const {
  return get("Name", "") != NULL;
}

SkString SkPdfType1FontDictionary::BaseFont(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("BaseFont", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfType1FontDictionary::has_BaseFont() const {
  return get("BaseFont", "") != NULL;
}

int64_t SkPdfType1FontDictionary::FirstChar(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("FirstChar", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfType1FontDictionary::has_FirstChar() const {
  return get("FirstChar", "") != NULL;
}

int64_t SkPdfType1FontDictionary::LastChar(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("LastChar", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfType1FontDictionary::has_LastChar() const {
  return get("LastChar", "") != NULL;
}

SkPdfArray* SkPdfType1FontDictionary::Widths(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Widths", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfType1FontDictionary::has_Widths() const {
  return get("Widths", "") != NULL;
}

SkPdfFontDescriptorDictionary* SkPdfType1FontDictionary::FontDescriptor(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("FontDescriptor", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary() && ((SkPdfFontDescriptorDictionary*)ret)->valid()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfFontDescriptorDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfType1FontDictionary::has_FontDescriptor() const {
  return get("FontDescriptor", "") != NULL;
}

bool SkPdfType1FontDictionary::isEncodingAName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Encoding", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isName();
}

SkString SkPdfType1FontDictionary::getEncodingAsName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Encoding", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfType1FontDictionary::isEncodingADictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Encoding", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isDictionary();
}

SkPdfDictionary* SkPdfType1FontDictionary::getEncodingAsDictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Encoding", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfType1FontDictionary::has_Encoding() const {
  return get("Encoding", "") != NULL;
}

SkPdfStream* SkPdfType1FontDictionary::ToUnicode(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ToUnicode", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfType1FontDictionary::has_ToUnicode() const {
  return get("ToUnicode", "") != NULL;
}
