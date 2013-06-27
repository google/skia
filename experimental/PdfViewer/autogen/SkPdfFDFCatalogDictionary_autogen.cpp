#include "SkPdfFDFCatalogDictionary_autogen.h"

std::string SkPdfFDFCatalogDictionary::Version() const {
  std::string ret;
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Version", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

SkPdfDictionary* SkPdfFDFCatalogDictionary::FDF() const {
  SkPdfDictionary* ret;
  if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "FDF", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

