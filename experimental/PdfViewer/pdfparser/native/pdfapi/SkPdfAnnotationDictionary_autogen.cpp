/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfAnnotationDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfAnnotationDictionary::Type(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Type", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfAnnotationDictionary::has_Type() const {
  return get("Type", "") != NULL;
}

SkString SkPdfAnnotationDictionary::Subtype(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Subtype", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfAnnotationDictionary::has_Subtype() const {
  return get("Subtype", "") != NULL;
}

SkString SkPdfAnnotationDictionary::Contents(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Contents", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfAnnotationDictionary::has_Contents() const {
  return get("Contents", "") != NULL;
}

SkPdfDictionary* SkPdfAnnotationDictionary::P(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("P", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfAnnotationDictionary::has_P() const {
  return get("P", "") != NULL;
}

SkRect SkPdfAnnotationDictionary::Rect(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Rect", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isRectangle()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->rectangleValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkRect::MakeEmpty();
}

bool SkPdfAnnotationDictionary::has_Rect() const {
  return get("Rect", "") != NULL;
}

bool SkPdfAnnotationDictionary::has_NM() const {
  return get("NM", "") != NULL;
}

bool SkPdfAnnotationDictionary::isMADate(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("M", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isDate();
}

SkPdfDate SkPdfAnnotationDictionary::getMAsDate(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("M", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDate()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->dateValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkPdfDate();
}

bool SkPdfAnnotationDictionary::isMAString(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("M", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isAnyString();
}

SkString SkPdfAnnotationDictionary::getMAsString(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("M", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfAnnotationDictionary::has_M() const {
  return get("M", "") != NULL;
}

int64_t SkPdfAnnotationDictionary::F(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("F", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfAnnotationDictionary::has_F() const {
  return get("F", "") != NULL;
}

SkPdfDictionary* SkPdfAnnotationDictionary::BS(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("BS", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfAnnotationDictionary::has_BS() const {
  return get("BS", "") != NULL;
}

SkPdfArray* SkPdfAnnotationDictionary::Border(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Border", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfAnnotationDictionary::has_Border() const {
  return get("Border", "") != NULL;
}

SkPdfDictionary* SkPdfAnnotationDictionary::AP(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("AP", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfAnnotationDictionary::has_AP() const {
  return get("AP", "") != NULL;
}

SkString SkPdfAnnotationDictionary::AS(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("AS", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfAnnotationDictionary::has_AS() const {
  return get("AS", "") != NULL;
}

SkPdfArray* SkPdfAnnotationDictionary::C(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("C", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfAnnotationDictionary::has_C() const {
  return get("C", "") != NULL;
}

double SkPdfAnnotationDictionary::CA(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("CA", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfAnnotationDictionary::has_CA() const {
  return get("CA", "") != NULL;
}

SkString SkPdfAnnotationDictionary::T(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("T", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfAnnotationDictionary::has_T() const {
  return get("T", "") != NULL;
}

SkPdfDictionary* SkPdfAnnotationDictionary::Popup(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Popup", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfAnnotationDictionary::has_Popup() const {
  return get("Popup", "") != NULL;
}

SkPdfDictionary* SkPdfAnnotationDictionary::A(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("A", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfAnnotationDictionary::has_A() const {
  return get("A", "") != NULL;
}

SkPdfDictionary* SkPdfAnnotationDictionary::AA(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("AA", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfAnnotationDictionary::has_AA() const {
  return get("AA", "") != NULL;
}

int64_t SkPdfAnnotationDictionary::StructParent(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("StructParent", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfAnnotationDictionary::has_StructParent() const {
  return get("StructParent", "") != NULL;
}
