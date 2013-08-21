/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfType3FontDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfType3FontDictionary::Type(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Type", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfType3FontDictionary::has_Type() const {
  return get("Type", "") != NULL;
}

SkString SkPdfType3FontDictionary::Subtype(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Subtype", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfType3FontDictionary::has_Subtype() const {
  return get("Subtype", "") != NULL;
}

SkString SkPdfType3FontDictionary::Name(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Name", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfType3FontDictionary::has_Name() const {
  return get("Name", "") != NULL;
}

SkRect SkPdfType3FontDictionary::FontBBox(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("FontBBox", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isRectangle()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->rectangleValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkRect::MakeEmpty();
}

bool SkPdfType3FontDictionary::has_FontBBox() const {
  return get("FontBBox", "") != NULL;
}

SkMatrix SkPdfType3FontDictionary::FontMatrix(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("FontMatrix", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isMatrix()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->matrixValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkMatrix::I();
}

bool SkPdfType3FontDictionary::has_FontMatrix() const {
  return get("FontMatrix", "") != NULL;
}

SkPdfDictionary* SkPdfType3FontDictionary::CharProcs(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("CharProcs", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfType3FontDictionary::has_CharProcs() const {
  return get("CharProcs", "") != NULL;
}

bool SkPdfType3FontDictionary::isEncodingAName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Encoding", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isName();
}

SkString SkPdfType3FontDictionary::getEncodingAsName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Encoding", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfType3FontDictionary::isEncodingAEncodingdictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Encoding", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isDictionary() && ((SkPdfEncodingDictionary*)ret)->valid();
}

SkPdfEncodingDictionary* SkPdfType3FontDictionary::getEncodingAsEncodingdictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Encoding", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary() && ((SkPdfEncodingDictionary*)ret)->valid()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfEncodingDictionary*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfType3FontDictionary::has_Encoding() const {
  return get("Encoding", "") != NULL;
}

int64_t SkPdfType3FontDictionary::FirstChar(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("FirstChar", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfType3FontDictionary::has_FirstChar() const {
  return get("FirstChar", "") != NULL;
}

int64_t SkPdfType3FontDictionary::LastChar(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("LastChar", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfType3FontDictionary::has_LastChar() const {
  return get("LastChar", "") != NULL;
}

SkPdfArray* SkPdfType3FontDictionary::Widths(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Widths", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfType3FontDictionary::has_Widths() const {
  return get("Widths", "") != NULL;
}

SkPdfResourceDictionary* SkPdfType3FontDictionary::Resources(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Resources", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary() && ((SkPdfResourceDictionary*)ret)->valid()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfResourceDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfType3FontDictionary::has_Resources() const {
  return get("Resources", "") != NULL;
}

SkPdfStream* SkPdfType3FontDictionary::ToUnicode(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ToUnicode", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfType3FontDictionary::has_ToUnicode() const {
  return get("ToUnicode", "") != NULL;
}
