/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfImageDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfImageDictionary::Type(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Type", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfImageDictionary::has_Type() const {
  return get("Type", "") != NULL;
}

SkString SkPdfImageDictionary::Subtype(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Subtype", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfImageDictionary::has_Subtype() const {
  return get("Subtype", "") != NULL;
}

int64_t SkPdfImageDictionary::Width(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Width", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfImageDictionary::has_Width() const {
  return get("Width", "") != NULL;
}

int64_t SkPdfImageDictionary::Height(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Height", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfImageDictionary::has_Height() const {
  return get("Height", "") != NULL;
}

bool SkPdfImageDictionary::isColorSpaceAName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ColorSpace", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isName();
}

SkString SkPdfImageDictionary::getColorSpaceAsName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ColorSpace", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfImageDictionary::isColorSpaceAArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ColorSpace", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isArray();
}

SkPdfArray* SkPdfImageDictionary::getColorSpaceAsArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ColorSpace", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfImageDictionary::has_ColorSpace() const {
  return get("ColorSpace", "") != NULL;
}

int64_t SkPdfImageDictionary::BitsPerComponent(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("BitsPerComponent", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfImageDictionary::has_BitsPerComponent() const {
  return get("BitsPerComponent", "") != NULL;
}

SkString SkPdfImageDictionary::Intent(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Intent", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfImageDictionary::has_Intent() const {
  return get("Intent", "") != NULL;
}

bool SkPdfImageDictionary::ImageMask(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ImageMask", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isBoolean()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->boolValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return false;
}

bool SkPdfImageDictionary::has_ImageMask() const {
  return get("ImageMask", "") != NULL;
}

bool SkPdfImageDictionary::isMaskAStream(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Mask", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->hasStream();
}

SkPdfStream* SkPdfImageDictionary::getMaskAsStream(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Mask", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfImageDictionary::isMaskAArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Mask", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isArray();
}

SkPdfArray* SkPdfImageDictionary::getMaskAsArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Mask", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfImageDictionary::has_Mask() const {
  return get("Mask", "") != NULL;
}

SkPdfImageDictionary* SkPdfImageDictionary::SMask(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("SMask", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary() && ((SkPdfImageDictionary*)ret)->valid()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfImageDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfImageDictionary::has_SMask() const {
  return get("SMask", "") != NULL;
}

SkPdfArray* SkPdfImageDictionary::Decode(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Decode", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfImageDictionary::has_Decode() const {
  return get("Decode", "") != NULL;
}

bool SkPdfImageDictionary::Interpolate(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Interpolate", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isBoolean()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->boolValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return false;
}

bool SkPdfImageDictionary::has_Interpolate() const {
  return get("Interpolate", "") != NULL;
}

SkPdfArray* SkPdfImageDictionary::Alternates(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Alternates", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfImageDictionary::has_Alternates() const {
  return get("Alternates", "") != NULL;
}

SkString SkPdfImageDictionary::Name(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Name", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfImageDictionary::has_Name() const {
  return get("Name", "") != NULL;
}

int64_t SkPdfImageDictionary::StructParent(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("StructParent", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfImageDictionary::has_StructParent() const {
  return get("StructParent", "") != NULL;
}

SkString SkPdfImageDictionary::ID(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ID", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfImageDictionary::has_ID() const {
  return get("ID", "") != NULL;
}

SkPdfDictionary* SkPdfImageDictionary::OPI(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("OPI", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfImageDictionary::has_OPI() const {
  return get("OPI", "") != NULL;
}

SkPdfStream* SkPdfImageDictionary::Metadata(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Metadata", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfImageDictionary::has_Metadata() const {
  return get("Metadata", "") != NULL;
}
