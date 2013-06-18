#ifndef __DEFINED__SkPdfTrueTypeFontDictionary
#define __DEFINED__SkPdfTrueTypeFontDictionary

#include "SkPdfEnums_autogen.h"
#include "SkPdfArray_autogen.h"
#include "SkPdfFontDictionary_autogen.h"

class SkPdfTrueTypeFontDictionary : public SkPdfFontDictionary {
public:
  virtual SkPdfObjectType getType() const { return kTrueTypeFontDictionary_SkPdfObjectType;}
  virtual SkPdfObjectType getTypeEnd() const { return (SkPdfObjectType)(kTrueTypeFontDictionary_SkPdfObjectType + 1);}
public:
  virtual SkPdfTrueTypeFontDictionary* asTrueTypeFontDictionary() {return this;}
  virtual const SkPdfTrueTypeFontDictionary* asTrueTypeFontDictionary() const {return this;}

private:
  virtual SkPdfCIDFontDictionary* asCIDFontDictionary() {return NULL;}
  virtual const SkPdfCIDFontDictionary* asCIDFontDictionary() const {return NULL;}

  virtual SkPdfType0FontDictionary* asType0FontDictionary() {return NULL;}
  virtual const SkPdfType0FontDictionary* asType0FontDictionary() const {return NULL;}

  virtual SkPdfType1FontDictionary* asType1FontDictionary() {return NULL;}
  virtual const SkPdfType1FontDictionary* asType1FontDictionary() const {return NULL;}

  virtual SkPdfMultiMasterFontDictionary* asMultiMasterFontDictionary() {return NULL;}
  virtual const SkPdfMultiMasterFontDictionary* asMultiMasterFontDictionary() const {return NULL;}

  virtual SkPdfType3FontDictionary* asType3FontDictionary() {return NULL;}
  virtual const SkPdfType3FontDictionary* asType3FontDictionary() const {return NULL;}

public:
private:
public:
  SkPdfTrueTypeFontDictionary(const PdfMemDocument* podofoDoc = NULL, const PdfObject* podofoObj = NULL) : SkPdfFontDictionary(podofoDoc, podofoObj) {}

  virtual bool valid() const {return true;}

  SkPdfTrueTypeFontDictionary& operator=(const SkPdfTrueTypeFontDictionary& from) {this->fPodofoDoc = from.fPodofoDoc; this->fPodofoObj = from.fPodofoObj; return *this;}

};

#endif  // __DEFINED__SkPdfTrueTypeFontDictionary
