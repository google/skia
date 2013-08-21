/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfGraphicsStateDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfGraphicsStateDictionary::Type(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Type", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfGraphicsStateDictionary::has_Type() const {
  return get("Type", "") != NULL;
}

double SkPdfGraphicsStateDictionary::LW(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("LW", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfGraphicsStateDictionary::has_LW() const {
  return get("LW", "") != NULL;
}

int64_t SkPdfGraphicsStateDictionary::LC(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("LC", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfGraphicsStateDictionary::has_LC() const {
  return get("LC", "") != NULL;
}

int64_t SkPdfGraphicsStateDictionary::LJ(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("LJ", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfGraphicsStateDictionary::has_LJ() const {
  return get("LJ", "") != NULL;
}

double SkPdfGraphicsStateDictionary::ML(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ML", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfGraphicsStateDictionary::has_ML() const {
  return get("ML", "") != NULL;
}

SkPdfArray* SkPdfGraphicsStateDictionary::D(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("D", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfGraphicsStateDictionary::has_D() const {
  return get("D", "") != NULL;
}

SkString SkPdfGraphicsStateDictionary::RI(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("RI", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfGraphicsStateDictionary::has_RI() const {
  return get("RI", "") != NULL;
}

bool SkPdfGraphicsStateDictionary::OP(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("OP", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isBoolean()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->boolValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return false;
}

bool SkPdfGraphicsStateDictionary::has_OP() const {
  return get("OP", "") != NULL;
}

bool SkPdfGraphicsStateDictionary::op(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("op", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isBoolean()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->boolValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return false;
}

bool SkPdfGraphicsStateDictionary::has_op() const {
  return get("op", "") != NULL;
}

int64_t SkPdfGraphicsStateDictionary::OPM(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("OPM", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfGraphicsStateDictionary::has_OPM() const {
  return get("OPM", "") != NULL;
}

SkPdfArray* SkPdfGraphicsStateDictionary::Font(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Font", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfGraphicsStateDictionary::has_Font() const {
  return get("Font", "") != NULL;
}

SkPdfFunction SkPdfGraphicsStateDictionary::BG(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("BG", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isFunction()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->functionValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkPdfFunction();
}

bool SkPdfGraphicsStateDictionary::has_BG() const {
  return get("BG", "") != NULL;
}

bool SkPdfGraphicsStateDictionary::isBG2AFunction(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("BG2", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isFunction();
}

SkPdfFunction SkPdfGraphicsStateDictionary::getBG2AsFunction(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("BG2", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isFunction()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->functionValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkPdfFunction();
}

bool SkPdfGraphicsStateDictionary::isBG2AName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("BG2", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isName();
}

SkString SkPdfGraphicsStateDictionary::getBG2AsName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("BG2", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfGraphicsStateDictionary::has_BG2() const {
  return get("BG2", "") != NULL;
}

SkPdfFunction SkPdfGraphicsStateDictionary::UCR(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("UCR", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isFunction()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->functionValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkPdfFunction();
}

bool SkPdfGraphicsStateDictionary::has_UCR() const {
  return get("UCR", "") != NULL;
}

bool SkPdfGraphicsStateDictionary::isUCR2AFunction(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("UCR2", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isFunction();
}

SkPdfFunction SkPdfGraphicsStateDictionary::getUCR2AsFunction(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("UCR2", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isFunction()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->functionValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkPdfFunction();
}

bool SkPdfGraphicsStateDictionary::isUCR2AName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("UCR2", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isName();
}

SkString SkPdfGraphicsStateDictionary::getUCR2AsName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("UCR2", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfGraphicsStateDictionary::has_UCR2() const {
  return get("UCR2", "") != NULL;
}

bool SkPdfGraphicsStateDictionary::isTRAFunction(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TR", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isFunction();
}

SkPdfFunction SkPdfGraphicsStateDictionary::getTRAsFunction(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TR", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isFunction()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->functionValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkPdfFunction();
}

bool SkPdfGraphicsStateDictionary::isTRAArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TR", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isArray();
}

SkPdfArray* SkPdfGraphicsStateDictionary::getTRAsArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TR", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfGraphicsStateDictionary::isTRAName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TR", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isName();
}

SkString SkPdfGraphicsStateDictionary::getTRAsName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TR", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfGraphicsStateDictionary::has_TR() const {
  return get("TR", "") != NULL;
}

bool SkPdfGraphicsStateDictionary::isTR2AFunction(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TR2", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isFunction();
}

SkPdfFunction SkPdfGraphicsStateDictionary::getTR2AsFunction(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TR2", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isFunction()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->functionValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkPdfFunction();
}

bool SkPdfGraphicsStateDictionary::isTR2AArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TR2", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isArray();
}

SkPdfArray* SkPdfGraphicsStateDictionary::getTR2AsArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TR2", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfGraphicsStateDictionary::isTR2AName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TR2", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isName();
}

SkString SkPdfGraphicsStateDictionary::getTR2AsName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TR2", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfGraphicsStateDictionary::has_TR2() const {
  return get("TR2", "") != NULL;
}

bool SkPdfGraphicsStateDictionary::isHTADictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("HT", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isDictionary();
}

SkPdfDictionary* SkPdfGraphicsStateDictionary::getHTAsDictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("HT", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfGraphicsStateDictionary::isHTAStream(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("HT", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->hasStream();
}

SkPdfStream* SkPdfGraphicsStateDictionary::getHTAsStream(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("HT", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfGraphicsStateDictionary::isHTAName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("HT", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isName();
}

SkString SkPdfGraphicsStateDictionary::getHTAsName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("HT", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfGraphicsStateDictionary::has_HT() const {
  return get("HT", "") != NULL;
}

double SkPdfGraphicsStateDictionary::FL(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("FL", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfGraphicsStateDictionary::has_FL() const {
  return get("FL", "") != NULL;
}

double SkPdfGraphicsStateDictionary::SM(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("SM", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfGraphicsStateDictionary::has_SM() const {
  return get("SM", "") != NULL;
}

bool SkPdfGraphicsStateDictionary::SA(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("SA", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isBoolean()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->boolValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return false;
}

bool SkPdfGraphicsStateDictionary::has_SA() const {
  return get("SA", "") != NULL;
}

bool SkPdfGraphicsStateDictionary::isBMAName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("BM", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isName();
}

SkString SkPdfGraphicsStateDictionary::getBMAsName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("BM", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfGraphicsStateDictionary::isBMAArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("BM", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isArray();
}

SkPdfArray* SkPdfGraphicsStateDictionary::getBMAsArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("BM", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfGraphicsStateDictionary::has_BM() const {
  return get("BM", "") != NULL;
}

bool SkPdfGraphicsStateDictionary::isSMaskADictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("SMask", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isDictionary();
}

SkPdfDictionary* SkPdfGraphicsStateDictionary::getSMaskAsDictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("SMask", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfGraphicsStateDictionary::isSMaskAName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("SMask", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isName();
}

SkString SkPdfGraphicsStateDictionary::getSMaskAsName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("SMask", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfGraphicsStateDictionary::has_SMask() const {
  return get("SMask", "") != NULL;
}

double SkPdfGraphicsStateDictionary::CA(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("CA", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfGraphicsStateDictionary::has_CA() const {
  return get("CA", "") != NULL;
}

double SkPdfGraphicsStateDictionary::ca(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ca", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfGraphicsStateDictionary::has_ca() const {
  return get("ca", "") != NULL;
}

bool SkPdfGraphicsStateDictionary::AIS(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("AIS", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isBoolean()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->boolValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return false;
}

bool SkPdfGraphicsStateDictionary::has_AIS() const {
  return get("AIS", "") != NULL;
}

bool SkPdfGraphicsStateDictionary::TK(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TK", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isBoolean()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->boolValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return false;
}

bool SkPdfGraphicsStateDictionary::has_TK() const {
  return get("TK", "") != NULL;
}
