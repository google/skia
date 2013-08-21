/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfFontDescriptorDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfFontDescriptorDictionary::Type(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Type", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfFontDescriptorDictionary::has_Type() const {
  return get("Type", "") != NULL;
}

SkString SkPdfFontDescriptorDictionary::FontName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("FontName", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfFontDescriptorDictionary::has_FontName() const {
  return get("FontName", "") != NULL;
}

int64_t SkPdfFontDescriptorDictionary::Flags(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Flags", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfFontDescriptorDictionary::has_Flags() const {
  return get("Flags", "") != NULL;
}

SkRect SkPdfFontDescriptorDictionary::FontBBox(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("FontBBox", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isRectangle()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->rectangleValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkRect::MakeEmpty();
}

bool SkPdfFontDescriptorDictionary::has_FontBBox() const {
  return get("FontBBox", "") != NULL;
}

double SkPdfFontDescriptorDictionary::ItalicAngle(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ItalicAngle", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfFontDescriptorDictionary::has_ItalicAngle() const {
  return get("ItalicAngle", "") != NULL;
}

double SkPdfFontDescriptorDictionary::Ascent(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Ascent", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfFontDescriptorDictionary::has_Ascent() const {
  return get("Ascent", "") != NULL;
}

double SkPdfFontDescriptorDictionary::Descent(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Descent", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfFontDescriptorDictionary::has_Descent() const {
  return get("Descent", "") != NULL;
}

double SkPdfFontDescriptorDictionary::Leading(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Leading", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfFontDescriptorDictionary::has_Leading() const {
  return get("Leading", "") != NULL;
}

double SkPdfFontDescriptorDictionary::CapHeight(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("CapHeight", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfFontDescriptorDictionary::has_CapHeight() const {
  return get("CapHeight", "") != NULL;
}

double SkPdfFontDescriptorDictionary::XHeight(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("XHeight", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfFontDescriptorDictionary::has_XHeight() const {
  return get("XHeight", "") != NULL;
}

double SkPdfFontDescriptorDictionary::StemV(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("StemV", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfFontDescriptorDictionary::has_StemV() const {
  return get("StemV", "") != NULL;
}

double SkPdfFontDescriptorDictionary::StemH(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("StemH", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfFontDescriptorDictionary::has_StemH() const {
  return get("StemH", "") != NULL;
}

double SkPdfFontDescriptorDictionary::AvgWidth(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("AvgWidth", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfFontDescriptorDictionary::has_AvgWidth() const {
  return get("AvgWidth", "") != NULL;
}

double SkPdfFontDescriptorDictionary::MaxWidth(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("MaxWidth", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfFontDescriptorDictionary::has_MaxWidth() const {
  return get("MaxWidth", "") != NULL;
}

double SkPdfFontDescriptorDictionary::MissingWidth(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("MissingWidth", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfFontDescriptorDictionary::has_MissingWidth() const {
  return get("MissingWidth", "") != NULL;
}

SkPdfStream* SkPdfFontDescriptorDictionary::FontFile(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("FontFile", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfFontDescriptorDictionary::has_FontFile() const {
  return get("FontFile", "") != NULL;
}

SkPdfStream* SkPdfFontDescriptorDictionary::FontFile2(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("FontFile2", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfFontDescriptorDictionary::has_FontFile2() const {
  return get("FontFile2", "") != NULL;
}

SkPdfStream* SkPdfFontDescriptorDictionary::FontFile3(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("FontFile3", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfFontDescriptorDictionary::has_FontFile3() const {
  return get("FontFile3", "") != NULL;
}

SkString SkPdfFontDescriptorDictionary::CharSet(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("CharSet", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfFontDescriptorDictionary::has_CharSet() const {
  return get("CharSet", "") != NULL;
}
