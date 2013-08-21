#include "SkPdfCheckboxFieldDictionary_autogen.h"


#include "SkPdfNativeDoc.h"
SkString SkPdfCheckboxFieldDictionary::Opt(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Opt", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfCheckboxFieldDictionary::has_Opt() const {
  return get("Opt", "") != NULL;
}
