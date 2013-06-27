#include "SkPdfType4ShadingDictionary_autogen.h"

long SkPdfType4ShadingDictionary::BitsPerCoordinate() const {
  long ret;
  if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "BitsPerCoordinate", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

long SkPdfType4ShadingDictionary::BitsPerComponent() const {
  long ret;
  if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "BitsPerComponent", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

long SkPdfType4ShadingDictionary::BitsPerFlag() const {
  long ret;
  if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "BitsPerFlag", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

SkRect* SkPdfType4ShadingDictionary::Decode() const {
  SkRect* ret;
  if (SkRectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Decode", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

SkPdfFunction SkPdfType4ShadingDictionary::Function() const {
  SkPdfFunction ret;
  if (FunctionFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Function", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkPdfFunction();
}

