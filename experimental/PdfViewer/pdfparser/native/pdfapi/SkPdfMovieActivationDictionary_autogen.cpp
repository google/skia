/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfMovieActivationDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkPdfNativeObject* SkPdfMovieActivationDictionary::Start(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Start", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && true) || (doc == NULL && ret != NULL && ret->isReference())) return ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfMovieActivationDictionary::has_Start() const {
  return get("Start", "") != NULL;
}

SkPdfNativeObject* SkPdfMovieActivationDictionary::Duration(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Duration", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && true) || (doc == NULL && ret != NULL && ret->isReference())) return ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfMovieActivationDictionary::has_Duration() const {
  return get("Duration", "") != NULL;
}

double SkPdfMovieActivationDictionary::Rate(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Rate", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfMovieActivationDictionary::has_Rate() const {
  return get("Rate", "") != NULL;
}

double SkPdfMovieActivationDictionary::Volume(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Volume", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfMovieActivationDictionary::has_Volume() const {
  return get("Volume", "") != NULL;
}

bool SkPdfMovieActivationDictionary::ShowControls(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ShowControls", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isBoolean()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->boolValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return false;
}

bool SkPdfMovieActivationDictionary::has_ShowControls() const {
  return get("ShowControls", "") != NULL;
}

SkString SkPdfMovieActivationDictionary::Mode(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Mode", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfMovieActivationDictionary::has_Mode() const {
  return get("Mode", "") != NULL;
}

bool SkPdfMovieActivationDictionary::Synchronous(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Synchronous", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isBoolean()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->boolValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return false;
}

bool SkPdfMovieActivationDictionary::has_Synchronous() const {
  return get("Synchronous", "") != NULL;
}

SkPdfArray* SkPdfMovieActivationDictionary::FWScale(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("FWScale", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfMovieActivationDictionary::has_FWScale() const {
  return get("FWScale", "") != NULL;
}

SkPdfArray* SkPdfMovieActivationDictionary::FWPosition(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("FWPosition", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfMovieActivationDictionary::has_FWPosition() const {
  return get("FWPosition", "") != NULL;
}
