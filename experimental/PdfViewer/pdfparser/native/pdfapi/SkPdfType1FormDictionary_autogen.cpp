/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfType1FormDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfType1FormDictionary::Type(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Type", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfType1FormDictionary::has_Type() const {
  return get("Type", "") != NULL;
}

SkString SkPdfType1FormDictionary::Subtype(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Subtype", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfType1FormDictionary::has_Subtype() const {
  return get("Subtype", "") != NULL;
}

int64_t SkPdfType1FormDictionary::FormType(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("FormType", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfType1FormDictionary::has_FormType() const {
  return get("FormType", "") != NULL;
}

SkString SkPdfType1FormDictionary::Name(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Name", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfType1FormDictionary::has_Name() const {
  return get("Name", "") != NULL;
}

SkPdfDate SkPdfType1FormDictionary::LastModified(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("LastModified", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDate()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->dateValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkPdfDate();
}

bool SkPdfType1FormDictionary::has_LastModified() const {
  return get("LastModified", "") != NULL;
}

SkRect SkPdfType1FormDictionary::BBox(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("BBox", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isRectangle()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->rectangleValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkRect::MakeEmpty();
}

bool SkPdfType1FormDictionary::has_BBox() const {
  return get("BBox", "") != NULL;
}

SkMatrix SkPdfType1FormDictionary::Matrix(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Matrix", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isMatrix()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->matrixValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkMatrix::I();
}

bool SkPdfType1FormDictionary::has_Matrix() const {
  return get("Matrix", "") != NULL;
}

SkPdfResourceDictionary* SkPdfType1FormDictionary::Resources(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Resources", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary() && ((SkPdfResourceDictionary*)ret)->valid()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfResourceDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfType1FormDictionary::has_Resources() const {
  return get("Resources", "") != NULL;
}

SkPdfTransparencyGroupDictionary* SkPdfType1FormDictionary::Group(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Group", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary() && ((SkPdfTransparencyGroupDictionary*)ret)->valid()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfTransparencyGroupDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfType1FormDictionary::has_Group() const {
  return get("Group", "") != NULL;
}

SkPdfDictionary* SkPdfType1FormDictionary::Ref(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Ref", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfType1FormDictionary::has_Ref() const {
  return get("Ref", "") != NULL;
}

SkPdfStream* SkPdfType1FormDictionary::Metadata(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Metadata", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfType1FormDictionary::has_Metadata() const {
  return get("Metadata", "") != NULL;
}

SkPdfDictionary* SkPdfType1FormDictionary::PieceInfo(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("PieceInfo", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfType1FormDictionary::has_PieceInfo() const {
  return get("PieceInfo", "") != NULL;
}

int64_t SkPdfType1FormDictionary::StructParent(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("StructParent", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfType1FormDictionary::has_StructParent() const {
  return get("StructParent", "") != NULL;
}

int64_t SkPdfType1FormDictionary::StructParents(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("StructParents", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfType1FormDictionary::has_StructParents() const {
  return get("StructParents", "") != NULL;
}

SkPdfDictionary* SkPdfType1FormDictionary::OPI(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("OPI", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfType1FormDictionary::has_OPI() const {
  return get("OPI", "") != NULL;
}
