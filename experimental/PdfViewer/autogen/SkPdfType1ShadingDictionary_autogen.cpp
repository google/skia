#include "SkPdfType1ShadingDictionary_autogen.h"

SkPdfArray* SkPdfType1ShadingDictionary::Domain() const {
  SkPdfArray* ret;
  if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Domain", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

SkPdfArray* SkPdfType1ShadingDictionary::Matrix() const {
  SkPdfArray* ret;
  if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Matrix", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

SkPdfFunction SkPdfType1ShadingDictionary::Function() const {
  SkPdfFunction ret;
  if (FunctionFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Function", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkPdfFunction();
}

