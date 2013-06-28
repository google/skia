#ifndef __DEFINED__SkPdfType0FontDictionary
#define __DEFINED__SkPdfType0FontDictionary

#include "SkPdfUtils.h"
#include "SkPdfEnums_autogen.h"
#include "SkPdfArray_autogen.h"
#include "SkPdfFontDictionary_autogen.h"

// Entries in a Type 0 font dictionary
class SkPdfType0FontDictionary : public SkPdfFontDictionary {
public:
  virtual SkPdfObjectType getType() const { return kType0FontDictionary_SkPdfObjectType;}
  virtual SkPdfObjectType getTypeEnd() const { return (SkPdfObjectType)(kType0FontDictionary_SkPdfObjectType + 1);}
public:
  virtual SkPdfType0FontDictionary* asType0FontDictionary() {return this;}
  virtual const SkPdfType0FontDictionary* asType0FontDictionary() const {return this;}

private:
  virtual SkPdfType1FontDictionary* asType1FontDictionary() {return NULL;}
  virtual const SkPdfType1FontDictionary* asType1FontDictionary() const {return NULL;}

  virtual SkPdfMultiMasterFontDictionary* asMultiMasterFontDictionary() {return NULL;}
  virtual const SkPdfMultiMasterFontDictionary* asMultiMasterFontDictionary() const {return NULL;}

  virtual SkPdfTrueTypeFontDictionary* asTrueTypeFontDictionary() {return NULL;}
  virtual const SkPdfTrueTypeFontDictionary* asTrueTypeFontDictionary() const {return NULL;}

  virtual SkPdfType3FontDictionary* asType3FontDictionary() {return NULL;}
  virtual const SkPdfType3FontDictionary* asType3FontDictionary() const {return NULL;}

public:
private:
public:
  SkPdfType0FontDictionary(const PdfMemDocument* podofoDoc = NULL, const PdfObject* podofoObj = NULL) : SkPdfFontDictionary(podofoDoc, podofoObj) {}

  SkPdfType0FontDictionary(const SkPdfType0FontDictionary& from) : SkPdfFontDictionary(from.fPodofoDoc, from.fPodofoObj) {}

  virtual bool valid() const {return true;}

  SkPdfType0FontDictionary& operator=(const SkPdfType0FontDictionary& from) {this->fPodofoDoc = from.fPodofoDoc; this->fPodofoObj = from.fPodofoObj; return *this;}

/** (Required) The type of PDF object that this dictionary describes; must be
 *  Font for a font dictionary.
**/
  bool has_Type() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Type", "", NULL));
  }

  std::string Type() const;
/** (Required) The type of font; must be Type0 for a Type 0 font.
**/
  bool has_Subtype() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Subtype", "", NULL));
  }

  std::string Subtype() const;
/** (Required) The PostScript name of the font. In principle, this is an arbitrary
 *  name, since there is no font program associated directly with a Type 0 font
 *  dictionary. The conventions described here ensure maximum compatibility
 *  with existing Acrobat products.
 *  If the descendant is a Type 0 CIDFont, this name should be the concatenation
 *  of the CIDFont's BaseFont name, a hyphen, and the CMap name given in the
 *  Encoding entry (or the CMapName entry in the CMap program itself). If the
 *  descendant is a Type 2 CIDFont, this name should be the same as the
 *  CIDFont's BaseFont name.
**/
  bool has_BaseFont() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "BaseFont", "", NULL));
  }

  std::string BaseFont() const;
/** (Required) The name of a predefined CMap, or a stream containing a CMap
 *  program, that maps character codes to font numbers and CIDs. If the descen-
 *  dant is a Type 2 CIDFont whose associated TrueType font program is not em-
 *  bedded in the PDF file, the Encoding entry must be a predefined CMap name
 *  (see "Glyph Selection in CIDFonts" on page 339).
**/
  bool has_Encoding() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Encoding", "", NULL));
  }

  bool isEncodingAName() const {
    SkPdfObject* ret = NULL;
    if (!ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Encoding", "", &ret)) return false;
    return ret->podofo()->GetDataType() == ePdfDataType_Name;
  }

  std::string getEncodingAsName() const;
  bool isEncodingAStream() const {
    SkPdfObject* ret = NULL;
    if (!ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Encoding", "", &ret)) return false;
    return ret->podofo()->HasStream();
  }

  SkPdfStream* getEncodingAsStream() const;
/** (Required) An array specifying one or more fonts or CIDFonts that are
 *  descendants of this composite font. This array is indexed by the font number
 *  that is obtained by mapping a character code through the CMap specified in
 *  the Encoding entry.
 *  Note: In all PDF versions up to and including PDF 1.4, DescendantFonts must
 *  be a one-element array containing a CIDFont dictionary.
**/
  bool has_DescendantFonts() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "DescendantFonts", "", NULL));
  }

  SkPdfArray* DescendantFonts() const;
/** (Optional) A stream containing a CMap file that maps character codes to
 *  Unicode values (see Section 5.9, "ToUnicode CMaps").
**/
  bool has_ToUnicode() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "ToUnicode", "", NULL));
  }

  SkPdfStream* ToUnicode() const;
};

#endif  // __DEFINED__SkPdfType0FontDictionary
