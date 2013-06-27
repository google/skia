#include "SkPdfType16HalftoneDictionary_autogen.h"

std::string SkPdfType16HalftoneDictionary::Type() const {
  std::string ret;
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Type", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

long SkPdfType16HalftoneDictionary::HalftoneType() const {
  long ret;
  if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "HalftoneType", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

std::string SkPdfType16HalftoneDictionary::HalftoneName() const {
  std::string ret;
  if (StringFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "HalftoneName", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

long SkPdfType16HalftoneDictionary::Width() const {
  long ret;
  if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Width", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

long SkPdfType16HalftoneDictionary::Height() const {
  long ret;
  if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Height", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

long SkPdfType16HalftoneDictionary::Width2() const {
  long ret;
  if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Width2", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

long SkPdfType16HalftoneDictionary::Height2() const {
  long ret;
  if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Height2", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

SkPdfFunction SkPdfType16HalftoneDictionary::getTransferFunctionAsFunction() const {
  SkPdfFunction ret = SkPdfFunction();
  if (FunctionFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "TransferFunction", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkPdfFunction();
}

std::string SkPdfType16HalftoneDictionary::getTransferFunctionAsName() const {
  std::string ret = "";
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "TransferFunction", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

