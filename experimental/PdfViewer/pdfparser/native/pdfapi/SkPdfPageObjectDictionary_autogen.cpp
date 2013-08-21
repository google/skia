/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfPageObjectDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfPageObjectDictionary::Type(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Type", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfPageObjectDictionary::has_Type() const {
  return get("Type", "") != NULL;
}

SkPdfDictionary* SkPdfPageObjectDictionary::Parent(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Parent", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfPageObjectDictionary::has_Parent() const {
  return get("Parent", "") != NULL;
}

SkPdfDate SkPdfPageObjectDictionary::LastModified(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("LastModified", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDate()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->dateValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkPdfDate();
}

bool SkPdfPageObjectDictionary::has_LastModified() const {
  return get("LastModified", "") != NULL;
}

SkPdfResourceDictionary* SkPdfPageObjectDictionary::Resources(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Resources", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary() && ((SkPdfResourceDictionary*)ret)->valid()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfResourceDictionary*)ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

bool SkPdfPageObjectDictionary::has_Resources() const {
  return get("Resources", "") != NULL;
}

SkRect SkPdfPageObjectDictionary::MediaBox(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("MediaBox", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isRectangle()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->rectangleValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkRect::MakeEmpty();
}

bool SkPdfPageObjectDictionary::has_MediaBox() const {
  return get("MediaBox", "") != NULL;
}

SkRect SkPdfPageObjectDictionary::CropBox(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("CropBox", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isRectangle()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->rectangleValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkRect::MakeEmpty();
}

bool SkPdfPageObjectDictionary::has_CropBox() const {
  return get("CropBox", "") != NULL;
}

SkRect SkPdfPageObjectDictionary::BleedBox(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("BleedBox", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isRectangle()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->rectangleValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkRect::MakeEmpty();
}

bool SkPdfPageObjectDictionary::has_BleedBox() const {
  return get("BleedBox", "") != NULL;
}

SkRect SkPdfPageObjectDictionary::TrimBox(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TrimBox", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isRectangle()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->rectangleValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkRect::MakeEmpty();
}

bool SkPdfPageObjectDictionary::has_TrimBox() const {
  return get("TrimBox", "") != NULL;
}

SkRect SkPdfPageObjectDictionary::ArtBox(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ArtBox", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isRectangle()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->rectangleValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkRect::MakeEmpty();
}

bool SkPdfPageObjectDictionary::has_ArtBox() const {
  return get("ArtBox", "") != NULL;
}

SkPdfDictionary* SkPdfPageObjectDictionary::BoxColorInfo(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("BoxColorInfo", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfPageObjectDictionary::has_BoxColorInfo() const {
  return get("BoxColorInfo", "") != NULL;
}

bool SkPdfPageObjectDictionary::isContentsAStream(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Contents", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->hasStream();
}

SkPdfStream* SkPdfPageObjectDictionary::getContentsAsStream(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Contents", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfPageObjectDictionary::isContentsAArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Contents", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isArray();
}

SkPdfArray* SkPdfPageObjectDictionary::getContentsAsArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Contents", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfPageObjectDictionary::has_Contents() const {
  return get("Contents", "") != NULL;
}

int64_t SkPdfPageObjectDictionary::Rotate(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Rotate", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfPageObjectDictionary::has_Rotate() const {
  return get("Rotate", "") != NULL;
}

SkPdfTransparencyGroupDictionary* SkPdfPageObjectDictionary::Group(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Group", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary() && ((SkPdfTransparencyGroupDictionary*)ret)->valid()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfTransparencyGroupDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfPageObjectDictionary::has_Group() const {
  return get("Group", "") != NULL;
}

SkPdfStream* SkPdfPageObjectDictionary::Thumb(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Thumb", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfPageObjectDictionary::has_Thumb() const {
  return get("Thumb", "") != NULL;
}

SkPdfArray* SkPdfPageObjectDictionary::B(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("B", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfPageObjectDictionary::has_B() const {
  return get("B", "") != NULL;
}

double SkPdfPageObjectDictionary::Dur(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Dur", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfPageObjectDictionary::has_Dur() const {
  return get("Dur", "") != NULL;
}

SkPdfDictionary* SkPdfPageObjectDictionary::Trans(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Trans", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfPageObjectDictionary::has_Trans() const {
  return get("Trans", "") != NULL;
}

SkPdfArray* SkPdfPageObjectDictionary::Annots(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Annots", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfPageObjectDictionary::has_Annots() const {
  return get("Annots", "") != NULL;
}

SkPdfDictionary* SkPdfPageObjectDictionary::AA(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("AA", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfPageObjectDictionary::has_AA() const {
  return get("AA", "") != NULL;
}

SkPdfStream* SkPdfPageObjectDictionary::Metadata(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Metadata", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfPageObjectDictionary::has_Metadata() const {
  return get("Metadata", "") != NULL;
}

SkPdfDictionary* SkPdfPageObjectDictionary::PieceInfo(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("PieceInfo", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfPageObjectDictionary::has_PieceInfo() const {
  return get("PieceInfo", "") != NULL;
}

int64_t SkPdfPageObjectDictionary::StructParents(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("StructParents", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfPageObjectDictionary::has_StructParents() const {
  return get("StructParents", "") != NULL;
}

SkString SkPdfPageObjectDictionary::ID(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ID", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfPageObjectDictionary::has_ID() const {
  return get("ID", "") != NULL;
}

double SkPdfPageObjectDictionary::PZ(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("PZ", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfPageObjectDictionary::has_PZ() const {
  return get("PZ", "") != NULL;
}

SkPdfDictionary* SkPdfPageObjectDictionary::SeparationInfo(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("SeparationInfo", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfPageObjectDictionary::has_SeparationInfo() const {
  return get("SeparationInfo", "") != NULL;
}
