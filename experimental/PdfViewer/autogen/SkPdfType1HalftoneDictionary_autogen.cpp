#include "SkPdfType1HalftoneDictionary_autogen.h"

std::string SkPdfType1HalftoneDictionary::Type() const {
  std::string ret;
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Type", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

long SkPdfType1HalftoneDictionary::HalftoneType() const {
  long ret;
  if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "HalftoneType", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

std::string SkPdfType1HalftoneDictionary::HalftoneName() const {
  std::string ret;
  if (StringFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "HalftoneName", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

double SkPdfType1HalftoneDictionary::Frequency() const {
  double ret;
  if (DoubleFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Frequency", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

double SkPdfType1HalftoneDictionary::Angle() const {
  double ret;
  if (DoubleFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Angle", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

SkPdfFunction SkPdfType1HalftoneDictionary::getSpotFunctionAsFunction() const {
  SkPdfFunction ret = SkPdfFunction();
  if (FunctionFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "SpotFunction", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkPdfFunction();
}

std::string SkPdfType1HalftoneDictionary::getSpotFunctionAsName() const {
  std::string ret = "";
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "SpotFunction", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

bool SkPdfType1HalftoneDictionary::AccurateScreens() const {
  bool ret;
  if (BoolFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "AccurateScreens", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return false;
}

SkPdfFunction SkPdfType1HalftoneDictionary::getTransferFunctionAsFunction() const {
  SkPdfFunction ret = SkPdfFunction();
  if (FunctionFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "TransferFunction", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkPdfFunction();
}

std::string SkPdfType1HalftoneDictionary::getTransferFunctionAsName() const {
  std::string ret = "";
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "TransferFunction", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

