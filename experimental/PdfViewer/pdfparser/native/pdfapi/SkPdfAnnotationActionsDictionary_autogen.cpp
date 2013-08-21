/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfAnnotationActionsDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkPdfDictionary* SkPdfAnnotationActionsDictionary::E(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("E", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfAnnotationActionsDictionary::has_E() const {
  return get("E", "") != NULL;
}

SkPdfDictionary* SkPdfAnnotationActionsDictionary::X(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("X", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfAnnotationActionsDictionary::has_X() const {
  return get("X", "") != NULL;
}

SkPdfDictionary* SkPdfAnnotationActionsDictionary::D(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("D", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfAnnotationActionsDictionary::has_D() const {
  return get("D", "") != NULL;
}

SkPdfDictionary* SkPdfAnnotationActionsDictionary::U(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("U", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfAnnotationActionsDictionary::has_U() const {
  return get("U", "") != NULL;
}

SkPdfDictionary* SkPdfAnnotationActionsDictionary::Fo(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Fo", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfAnnotationActionsDictionary::has_Fo() const {
  return get("Fo", "") != NULL;
}

SkPdfDictionary* SkPdfAnnotationActionsDictionary::Bl(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Bl", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfAnnotationActionsDictionary::has_Bl() const {
  return get("Bl", "") != NULL;
}
