#include "SkPdfDocumentCatalogActionsDictionary_autogen.h"

SkPdfDictionary* SkPdfDocumentCatalogActionsDictionary::DC() const {
  SkPdfDictionary* ret;
  if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "DC", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

SkPdfDictionary* SkPdfDocumentCatalogActionsDictionary::WS() const {
  SkPdfDictionary* ret;
  if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "WS", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

SkPdfDictionary* SkPdfDocumentCatalogActionsDictionary::DS() const {
  SkPdfDictionary* ret;
  if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "DS", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

SkPdfDictionary* SkPdfDocumentCatalogActionsDictionary::WP() const {
  SkPdfDictionary* ret;
  if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "WP", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

SkPdfDictionary* SkPdfDocumentCatalogActionsDictionary::DP() const {
  SkPdfDictionary* ret;
  if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "DP", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

