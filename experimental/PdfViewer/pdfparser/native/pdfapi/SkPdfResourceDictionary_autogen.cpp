/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfResourceDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkPdfDictionary* SkPdfResourceDictionary::ExtGState(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ExtGState", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfResourceDictionary::has_ExtGState() const {
  return get("ExtGState", "") != NULL;
}

SkPdfDictionary* SkPdfResourceDictionary::ColorSpace(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ColorSpace", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfResourceDictionary::has_ColorSpace() const {
  return get("ColorSpace", "") != NULL;
}

SkPdfDictionary* SkPdfResourceDictionary::Pattern(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Pattern", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfResourceDictionary::has_Pattern() const {
  return get("Pattern", "") != NULL;
}

SkPdfDictionary* SkPdfResourceDictionary::Shading(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Shading", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfResourceDictionary::has_Shading() const {
  return get("Shading", "") != NULL;
}

SkPdfDictionary* SkPdfResourceDictionary::XObject(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("XObject", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfResourceDictionary::has_XObject() const {
  return get("XObject", "") != NULL;
}

SkPdfDictionary* SkPdfResourceDictionary::Font(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Font", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfResourceDictionary::has_Font() const {
  return get("Font", "") != NULL;
}

SkPdfArray* SkPdfResourceDictionary::ProcSet(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ProcSet", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfResourceDictionary::has_ProcSet() const {
  return get("ProcSet", "") != NULL;
}

SkPdfDictionary* SkPdfResourceDictionary::Properties(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Properties", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfResourceDictionary::has_Properties() const {
  return get("Properties", "") != NULL;
}
