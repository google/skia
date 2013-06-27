#include "SkPdfMovieDictionary_autogen.h"

SkPdfFileSpec SkPdfMovieDictionary::F() const {
  SkPdfFileSpec ret;
  if (FileSpecFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "F", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkPdfFileSpec();
}

SkPdfArray* SkPdfMovieDictionary::Aspect() const {
  SkPdfArray* ret;
  if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Aspect", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

long SkPdfMovieDictionary::Rotate() const {
  long ret;
  if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Rotate", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfMovieDictionary::getPosterAsBoolean() const {
  bool ret = false;
  if (BoolFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Poster", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return false;
}

SkPdfStream* SkPdfMovieDictionary::getPosterAsStream() const {
  SkPdfStream* ret = NULL;
  if (StreamFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Poster", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}
