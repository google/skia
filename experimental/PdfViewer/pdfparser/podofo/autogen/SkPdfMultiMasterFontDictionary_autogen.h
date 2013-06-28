#ifndef __DEFINED__SkPdfMultiMasterFontDictionary
#define __DEFINED__SkPdfMultiMasterFontDictionary

#include "SkPdfUtils.h"
#include "SkPdfEnums_autogen.h"
#include "SkPdfArray_autogen.h"
#include "SkPdfType1FontDictionary_autogen.h"

class SkPdfMultiMasterFontDictionary : public SkPdfType1FontDictionary {
public:
  virtual SkPdfObjectType getType() const { return kMultiMasterFontDictionary_SkPdfObjectType;}
  virtual SkPdfObjectType getTypeEnd() const { return (SkPdfObjectType)(kMultiMasterFontDictionary_SkPdfObjectType + 1);}
public:
  virtual SkPdfMultiMasterFontDictionary* asMultiMasterFontDictionary() {return this;}
  virtual const SkPdfMultiMasterFontDictionary* asMultiMasterFontDictionary() const {return this;}

private:
  virtual SkPdfTrueTypeFontDictionary* asTrueTypeFontDictionary() {return NULL;}
  virtual const SkPdfTrueTypeFontDictionary* asTrueTypeFontDictionary() const {return NULL;}

  virtual SkPdfType3FontDictionary* asType3FontDictionary() {return NULL;}
  virtual const SkPdfType3FontDictionary* asType3FontDictionary() const {return NULL;}

public:
private:
public:
  SkPdfMultiMasterFontDictionary(const PdfMemDocument* podofoDoc = NULL, const PdfObject* podofoObj = NULL) : SkPdfType1FontDictionary(podofoDoc, podofoObj) {}

  SkPdfMultiMasterFontDictionary(const SkPdfMultiMasterFontDictionary& from) : SkPdfType1FontDictionary(from.fPodofoDoc, from.fPodofoObj) {}

  virtual bool valid() const {return true;}

  SkPdfMultiMasterFontDictionary& operator=(const SkPdfMultiMasterFontDictionary& from) {this->fPodofoDoc = from.fPodofoDoc; this->fPodofoObj = from.fPodofoObj; return *this;}

  bool has_Subtype() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Subtype", "", NULL));
  }

  std::string Subtype() const;
};

#endif  // __DEFINED__SkPdfMultiMasterFontDictionary
