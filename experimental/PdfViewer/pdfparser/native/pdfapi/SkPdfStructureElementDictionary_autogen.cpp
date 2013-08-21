/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfStructureElementDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfStructureElementDictionary::Type(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Type", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfStructureElementDictionary::has_Type() const {
  return get("Type", "") != NULL;
}

SkString SkPdfStructureElementDictionary::S(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("S", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfStructureElementDictionary::has_S() const {
  return get("S", "") != NULL;
}

SkPdfDictionary* SkPdfStructureElementDictionary::P(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("P", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfStructureElementDictionary::has_P() const {
  return get("P", "") != NULL;
}

SkString SkPdfStructureElementDictionary::ID(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ID", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfStructureElementDictionary::has_ID() const {
  return get("ID", "") != NULL;
}

SkPdfDictionary* SkPdfStructureElementDictionary::Pg(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Pg", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfStructureElementDictionary::has_Pg() const {
  return get("Pg", "") != NULL;
}

SkPdfNativeObject* SkPdfStructureElementDictionary::K(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("K", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && true) || (doc == NULL && ret != NULL && ret->isReference())) return ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfStructureElementDictionary::has_K() const {
  return get("K", "") != NULL;
}

SkPdfNativeObject* SkPdfStructureElementDictionary::A(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("A", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && true) || (doc == NULL && ret != NULL && ret->isReference())) return ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfStructureElementDictionary::has_A() const {
  return get("A", "") != NULL;
}

bool SkPdfStructureElementDictionary::isCAName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("C", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isName();
}

SkString SkPdfStructureElementDictionary::getCAsName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("C", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfStructureElementDictionary::isCAArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("C", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isArray();
}

SkPdfArray* SkPdfStructureElementDictionary::getCAsArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("C", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfStructureElementDictionary::has_C() const {
  return get("C", "") != NULL;
}

int64_t SkPdfStructureElementDictionary::R(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("R", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfStructureElementDictionary::has_R() const {
  return get("R", "") != NULL;
}

SkString SkPdfStructureElementDictionary::T(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("T", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfStructureElementDictionary::has_T() const {
  return get("T", "") != NULL;
}

SkString SkPdfStructureElementDictionary::Lang(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Lang", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfStructureElementDictionary::has_Lang() const {
  return get("Lang", "") != NULL;
}

SkString SkPdfStructureElementDictionary::Alt(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Alt", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfStructureElementDictionary::has_Alt() const {
  return get("Alt", "") != NULL;
}

SkString SkPdfStructureElementDictionary::ActualText(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ActualText", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfStructureElementDictionary::has_ActualText() const {
  return get("ActualText", "") != NULL;
}
