/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfLzwdecodeAndFlatedecodeFiltersDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

int64_t SkPdfLzwdecodeAndFlatedecodeFiltersDictionary::Predictor(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Predictor", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfLzwdecodeAndFlatedecodeFiltersDictionary::has_Predictor() const {
  return get("Predictor", "") != NULL;
}

int64_t SkPdfLzwdecodeAndFlatedecodeFiltersDictionary::Colors(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Colors", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfLzwdecodeAndFlatedecodeFiltersDictionary::has_Colors() const {
  return get("Colors", "") != NULL;
}

int64_t SkPdfLzwdecodeAndFlatedecodeFiltersDictionary::BitsPerComponent(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("BitsPerComponent", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfLzwdecodeAndFlatedecodeFiltersDictionary::has_BitsPerComponent() const {
  return get("BitsPerComponent", "") != NULL;
}

int64_t SkPdfLzwdecodeAndFlatedecodeFiltersDictionary::Columns(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Columns", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfLzwdecodeAndFlatedecodeFiltersDictionary::has_Columns() const {
  return get("Columns", "") != NULL;
}

int64_t SkPdfLzwdecodeAndFlatedecodeFiltersDictionary::EarlyChange(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("EarlyChange", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfLzwdecodeAndFlatedecodeFiltersDictionary::has_EarlyChange() const {
  return get("EarlyChange", "") != NULL;
}
