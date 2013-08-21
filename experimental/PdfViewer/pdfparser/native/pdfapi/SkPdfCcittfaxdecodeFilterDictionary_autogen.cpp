/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfCcittfaxdecodeFilterDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

int64_t SkPdfCcittfaxdecodeFilterDictionary::K(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("K", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfCcittfaxdecodeFilterDictionary::has_K() const {
  return get("K", "") != NULL;
}

bool SkPdfCcittfaxdecodeFilterDictionary::EndOfLine(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("EndOfLine", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isBoolean()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->boolValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return false;
}

bool SkPdfCcittfaxdecodeFilterDictionary::has_EndOfLine() const {
  return get("EndOfLine", "") != NULL;
}

bool SkPdfCcittfaxdecodeFilterDictionary::EncodedByteAlign(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("EncodedByteAlign", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isBoolean()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->boolValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return false;
}

bool SkPdfCcittfaxdecodeFilterDictionary::has_EncodedByteAlign() const {
  return get("EncodedByteAlign", "") != NULL;
}

int64_t SkPdfCcittfaxdecodeFilterDictionary::Columns(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Columns", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfCcittfaxdecodeFilterDictionary::has_Columns() const {
  return get("Columns", "") != NULL;
}

int64_t SkPdfCcittfaxdecodeFilterDictionary::Rows(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Rows", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfCcittfaxdecodeFilterDictionary::has_Rows() const {
  return get("Rows", "") != NULL;
}

bool SkPdfCcittfaxdecodeFilterDictionary::EndOfBlock(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("EndOfBlock", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isBoolean()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->boolValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return false;
}

bool SkPdfCcittfaxdecodeFilterDictionary::has_EndOfBlock() const {
  return get("EndOfBlock", "") != NULL;
}

bool SkPdfCcittfaxdecodeFilterDictionary::BlackIs1(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("BlackIs1", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isBoolean()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->boolValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return false;
}

bool SkPdfCcittfaxdecodeFilterDictionary::has_BlackIs1() const {
  return get("BlackIs1", "") != NULL;
}

int64_t SkPdfCcittfaxdecodeFilterDictionary::DamagedRowsBeforeError(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("DamagedRowsBeforeError", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfCcittfaxdecodeFilterDictionary::has_DamagedRowsBeforeError() const {
  return get("DamagedRowsBeforeError", "") != NULL;
}
