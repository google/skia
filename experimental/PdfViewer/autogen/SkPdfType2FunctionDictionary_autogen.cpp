#include "SkPdfType2FunctionDictionary_autogen.h"

SkPdfArray* SkPdfType2FunctionDictionary::C0() const {
  SkPdfArray* ret;
  if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "C0", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

SkPdfArray* SkPdfType2FunctionDictionary::C1() const {
  SkPdfArray* ret;
  if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "C1", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

double SkPdfType2FunctionDictionary::N() const {
  double ret;
  if (DoubleFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "N", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

