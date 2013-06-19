#ifndef __DEFINED__SkPdfType3FontDictionary
#define __DEFINED__SkPdfType3FontDictionary

#include "SkPdfEnums_autogen.h"
#include "SkPdfArray_autogen.h"
#include "SkPdfFontDictionary_autogen.h"

// Entries in a Type 3 font dictionary
class SkPdfType3FontDictionary : public SkPdfFontDictionary {
public:
  virtual SkPdfObjectType getType() const { return kType3FontDictionary_SkPdfObjectType;}
  virtual SkPdfObjectType getTypeEnd() const { return (SkPdfObjectType)(kType3FontDictionary_SkPdfObjectType + 1);}
public:
  virtual SkPdfType3FontDictionary* asType3FontDictionary() {return this;}
  virtual const SkPdfType3FontDictionary* asType3FontDictionary() const {return this;}

private:
  virtual SkPdfCIDFontDictionary* asCIDFontDictionary() {return NULL;}
  virtual const SkPdfCIDFontDictionary* asCIDFontDictionary() const {return NULL;}

  virtual SkPdfTrueTypeFontDictionary* asTrueTypeFontDictionary() {return NULL;}
  virtual const SkPdfTrueTypeFontDictionary* asTrueTypeFontDictionary() const {return NULL;}

  virtual SkPdfType0FontDictionary* asType0FontDictionary() {return NULL;}
  virtual const SkPdfType0FontDictionary* asType0FontDictionary() const {return NULL;}

  virtual SkPdfType1FontDictionary* asType1FontDictionary() {return NULL;}
  virtual const SkPdfType1FontDictionary* asType1FontDictionary() const {return NULL;}

  virtual SkPdfMultiMasterFontDictionary* asMultiMasterFontDictionary() {return NULL;}
  virtual const SkPdfMultiMasterFontDictionary* asMultiMasterFontDictionary() const {return NULL;}

public:
private:
public:
  SkPdfType3FontDictionary(const PdfMemDocument* podofoDoc = NULL, const PdfObject* podofoObj = NULL) : SkPdfFontDictionary(podofoDoc, podofoObj) {}

  virtual bool valid() const {return true;}

  SkPdfType3FontDictionary& operator=(const SkPdfType3FontDictionary& from) {this->fPodofoDoc = from.fPodofoDoc; this->fPodofoObj = from.fPodofoObj; return *this;}

/** (Required) The type of PDF object that this dictionary describes; must be
 *  Font for a font dictionary.
**/
  bool has_Type() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Type", "", NULL));
  }

  std::string Type() const {
    std::string ret;
    if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Type", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return "";
  }

/** (Required) The type of font; must be Type3 for a Type 3 font.
**/
  bool has_Subtype() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Subtype", "", NULL));
  }

  std::string Subtype() const {
    std::string ret;
    if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Subtype", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return "";
  }

/** (Required in PDF 1.0; optional otherwise) See Table 5.8 on page 317.
**/
  bool has_Name() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Name", "", NULL));
  }

  std::string Name() const {
    std::string ret;
    if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Name", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return "";
  }

/** (Required) A rectangle (see Section 3.8.3, "Rectangles"), expressed in the
 *  glyph coordinate system, specifying the font bounding box. This is the small-
 *  est rectangle enclosing the shape that would result if all of the glyphs of the
 *  font were placed with their origins coincident and then filled.
 *  If all four elements of the rectangle are zero, no assumptions are made based
 *  on the font bounding box. If any element is nonzero, it is essential that the
 *  font bounding box be accurate; if any glyph's marks fall outside this bound-
 *  ing box, incorrect behavior may result.
**/
  bool has_FontBBox() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "FontBBox", "", NULL));
  }

  SkRect FontBBox() const {
    SkRect ret;
    if (SkRectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "FontBBox", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return SkRect();
  }

/** (Required) An array of six numbers specifying the font matrix, mapping
 *  glyph space to text space (see Section 5.1.3, "Glyph Positioning and
 *  Metrics"). A common practice is to define glyphs in terms of a 1000-unit
**/
  bool has_FontMatrix() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "FontMatrix", "", NULL));
  }

  SkPdfArray FontMatrix() const {
    SkPdfArray ret;
    if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "FontMatrix", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return SkPdfArray();
  }

};

#endif  // __DEFINED__SkPdfType3FontDictionary
