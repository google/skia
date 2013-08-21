/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfOutlineItemDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfOutlineItemDictionary::Title(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Title", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfOutlineItemDictionary::has_Title() const {
  return get("Title", "") != NULL;
}

SkPdfDictionary* SkPdfOutlineItemDictionary::Parent(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Parent", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfOutlineItemDictionary::has_Parent() const {
  return get("Parent", "") != NULL;
}

SkPdfDictionary* SkPdfOutlineItemDictionary::Prev(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Prev", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfOutlineItemDictionary::has_Prev() const {
  return get("Prev", "") != NULL;
}

SkPdfDictionary* SkPdfOutlineItemDictionary::Next(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Next", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfOutlineItemDictionary::has_Next() const {
  return get("Next", "") != NULL;
}

SkPdfDictionary* SkPdfOutlineItemDictionary::First(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("First", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfOutlineItemDictionary::has_First() const {
  return get("First", "") != NULL;
}

SkPdfDictionary* SkPdfOutlineItemDictionary::Last(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Last", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfOutlineItemDictionary::has_Last() const {
  return get("Last", "") != NULL;
}

int64_t SkPdfOutlineItemDictionary::Count(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Count", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfOutlineItemDictionary::has_Count() const {
  return get("Count", "") != NULL;
}

bool SkPdfOutlineItemDictionary::isDestAName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Dest", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isName();
}

SkString SkPdfOutlineItemDictionary::getDestAsName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Dest", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfOutlineItemDictionary::isDestAString(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Dest", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isAnyString();
}

SkString SkPdfOutlineItemDictionary::getDestAsString(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Dest", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfOutlineItemDictionary::isDestAArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Dest", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isArray();
}

SkPdfArray* SkPdfOutlineItemDictionary::getDestAsArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Dest", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfOutlineItemDictionary::has_Dest() const {
  return get("Dest", "") != NULL;
}

SkPdfDictionary* SkPdfOutlineItemDictionary::A(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("A", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfOutlineItemDictionary::has_A() const {
  return get("A", "") != NULL;
}

SkPdfDictionary* SkPdfOutlineItemDictionary::SE(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("SE", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfOutlineItemDictionary::has_SE() const {
  return get("SE", "") != NULL;
}

SkPdfArray* SkPdfOutlineItemDictionary::C(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("C", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfOutlineItemDictionary::has_C() const {
  return get("C", "") != NULL;
}

int64_t SkPdfOutlineItemDictionary::F(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("F", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfOutlineItemDictionary::has_F() const {
  return get("F", "") != NULL;
}
