#include "SkPdfEmbeddedFileParameterDictionary_autogen.h"

long SkPdfEmbeddedFileParameterDictionary::Size() const {
  long ret;
  if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Size", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

SkPdfDate SkPdfEmbeddedFileParameterDictionary::CreationDate() const {
  SkPdfDate ret;
  if (DateFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "CreationDate", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkPdfDate();
}

SkPdfDate SkPdfEmbeddedFileParameterDictionary::ModDate() const {
  SkPdfDate ret;
  if (DateFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "ModDate", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkPdfDate();
}

SkPdfDictionary* SkPdfEmbeddedFileParameterDictionary::Mac() const {
  SkPdfDictionary* ret;
  if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Mac", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

std::string SkPdfEmbeddedFileParameterDictionary::CheckSum() const {
  std::string ret;
  if (StringFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "CheckSum", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}
