#include "SkPdfPageObjectActionsDictionary_autogen.h"


#include "SkPdfNativeDoc.h"
SkPdfDictionary* SkPdfPageObjectActionsDictionary::O(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("O", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfPageObjectActionsDictionary::has_O() const {
  return get("O", "") != NULL;
}

SkPdfDictionary* SkPdfPageObjectActionsDictionary::C(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("C", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfPageObjectActionsDictionary::has_C() const {
  return get("C", "") != NULL;
}
