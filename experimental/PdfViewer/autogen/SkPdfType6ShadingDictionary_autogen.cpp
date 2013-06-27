#include "SkPdfType6ShadingDictionary_autogen.h"

long SkPdfType6ShadingDictionary::BitsPerCoordinate() const {
  long ret;
  if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "BitsPerCoordinate", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

long SkPdfType6ShadingDictionary::BitsPerComponent() const {
  long ret;
  if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "BitsPerComponent", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

long SkPdfType6ShadingDictionary::BitsPerFlag() const {
  long ret;
  if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "BitsPerFlag", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

SkPdfArray* SkPdfType6ShadingDictionary::Decode() const {
  SkPdfArray* ret;
  if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Decode", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

SkPdfFunction SkPdfType6ShadingDictionary::Function() const {
  SkPdfFunction ret;
  if (FunctionFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Function", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkPdfFunction();
}

