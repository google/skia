/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfSignatureDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfSignatureDictionary::Type(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Type", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfSignatureDictionary::has_Type() const {
  return get("Type", "") != NULL;
}

SkString SkPdfSignatureDictionary::Filter(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Filter", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfSignatureDictionary::has_Filter() const {
  return get("Filter", "") != NULL;
}

SkString SkPdfSignatureDictionary::SubFilter(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("SubFilter", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfSignatureDictionary::has_SubFilter() const {
  return get("SubFilter", "") != NULL;
}

SkPdfArray* SkPdfSignatureDictionary::ByteRange(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ByteRange", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfSignatureDictionary::has_ByteRange() const {
  return get("ByteRange", "") != NULL;
}

SkString SkPdfSignatureDictionary::Contents(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Contents", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfSignatureDictionary::has_Contents() const {
  return get("Contents", "") != NULL;
}

SkString SkPdfSignatureDictionary::Name(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Name", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfSignatureDictionary::has_Name() const {
  return get("Name", "") != NULL;
}

SkPdfDate SkPdfSignatureDictionary::M(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("M", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDate()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->dateValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkPdfDate();
}

bool SkPdfSignatureDictionary::has_M() const {
  return get("M", "") != NULL;
}

SkString SkPdfSignatureDictionary::Location(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Location", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfSignatureDictionary::has_Location() const {
  return get("Location", "") != NULL;
}

SkString SkPdfSignatureDictionary::Reason(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Reason", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfSignatureDictionary::has_Reason() const {
  return get("Reason", "") != NULL;
}
