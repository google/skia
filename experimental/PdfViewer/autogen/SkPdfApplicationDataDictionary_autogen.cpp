#include "SkPdfApplicationDataDictionary_autogen.h"

SkPdfDate SkPdfApplicationDataDictionary::LastModified() const {
  SkPdfDate ret;
  if (DateFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "LastModified", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkPdfDate();
}

SkPdfObject* SkPdfApplicationDataDictionary::Private() const {
  SkPdfObject* ret;
  if (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Private", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

