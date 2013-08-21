/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfWebCaptureCommandSettingsDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkPdfDictionary* SkPdfWebCaptureCommandSettingsDictionary::G(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("G", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfWebCaptureCommandSettingsDictionary::has_G() const {
  return get("G", "") != NULL;
}

SkPdfDictionary* SkPdfWebCaptureCommandSettingsDictionary::C(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("C", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfWebCaptureCommandSettingsDictionary::has_C() const {
  return get("C", "") != NULL;
}
