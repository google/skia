#include "SkPdfReferenceDictionary_autogen.h"

SkPdfFileSpec SkPdfReferenceDictionary::F() const {
  SkPdfFileSpec ret;
  if (FileSpecFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "F", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkPdfFileSpec();
}

long SkPdfReferenceDictionary::getPageAsInteger() const {
  long ret = 0;
  if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Page", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

std::string SkPdfReferenceDictionary::getPageAsString() const {
  std::string ret = "";
  if (StringFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Page", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

SkPdfArray* SkPdfReferenceDictionary::ID() const {
  SkPdfArray* ret;
  if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "ID", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

