#ifndef __DEFINED__SkPdfType3FontDictionary
#define __DEFINED__SkPdfType3FontDictionary

#include "SkPdfUtils.h"
#include "SkPdfEnums_autogen.h"
#include "SkPdfArray_autogen.h"
#include "SkPdfType1FontDictionary_autogen.h"

// Entries in a Type 3 font dictionary
class SkPdfType3FontDictionary : public SkPdfType1FontDictionary {
public:
  virtual SkPdfObjectType getType() const { return kType3FontDictionary_SkPdfObjectType;}
  virtual SkPdfObjectType getTypeEnd() const { return (SkPdfObjectType)(kType3FontDictionary_SkPdfObjectType + 1);}
public:
  virtual SkPdfType3FontDictionary* asType3FontDictionary() {return this;}
  virtual const SkPdfType3FontDictionary* asType3FontDictionary() const {return this;}

private:
  virtual SkPdfMultiMasterFontDictionary* asMultiMasterFontDictionary() {return NULL;}
  virtual const SkPdfMultiMasterFontDictionary* asMultiMasterFontDictionary() const {return NULL;}

  virtual SkPdfTrueTypeFontDictionary* asTrueTypeFontDictionary() {return NULL;}
  virtual const SkPdfTrueTypeFontDictionary* asTrueTypeFontDictionary() const {return NULL;}

public:
private:
public:
  SkPdfType3FontDictionary(const PdfMemDocument* podofoDoc = NULL, const PdfObject* podofoObj = NULL) : SkPdfType1FontDictionary(podofoDoc, podofoObj) {}

  SkPdfType3FontDictionary(const SkPdfType3FontDictionary& from) : SkPdfType1FontDictionary(from.fPodofoDoc, from.fPodofoObj) {}

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

  SkRect* FontBBox() const {
    SkRect* ret;
    if (SkRectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "FontBBox", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return NULL;
  }

/** (Required) An array of six numbers specifying the font matrix, mapping
 *  glyph space to text space (see Section 5.1.3, "Glyph Positioning and
 *  Metrics"). A common practice is to define glyphs in terms of a 1000-unit
 *  glyph     coordinate      system,     in    which    case the font    matrix    is
 *  [0.001 0 0 0.001 0 0].
**/
  bool has_FontMatrix() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "FontMatrix", "", NULL));
  }

  SkMatrix* FontMatrix() const {
    SkMatrix* ret;
    if (SkMatrixFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "FontMatrix", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return NULL;
  }

/** (Required) A dictionary in which each key is a character name and the value
 *  associated with that key is a content stream that constructs and paints the
 *  glyph for that character. The stream must include as its first operator either
 *  d0 or d1. This is followed by operators describing one or more graphics
 *  objects, which may include path, text, or image objects. See below for more
 *  details about Type 3 glyph descriptions.
**/
  bool has_CharProcs() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "CharProcs", "", NULL));
  }

  SkPdfDictionary* CharProcs() const {
    SkPdfDictionary* ret;
    if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "CharProcs", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return NULL;
  }

/** (Required) An encoding dictionary whose Differences array specifies the
 *  complete character encoding for this font (see Section 5.5.5, "Character
 *  Encoding"; also see implementation note 46 in Appendix H).
**/
  bool has_Encoding() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Encoding", "", NULL));
  }

  bool isEncodingAName() const {
    SkPdfObject* ret = NULL;
    if (!ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Encoding", "", &ret)) return false;
    return ret->podofo()->GetDataType() == ePdfDataType_Name;
  }

  std::string getEncodingAsName() const {
    std::string ret = "";
    if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Encoding", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return "";
  }

  bool isEncodingAEncodingdictionary() const {
    SkPdfObject* ret = NULL;
    if (!ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Encoding", "", &ret)) return false;
    return ret->podofo()->GetDataType() == ePdfDataType_Dictionary;
  }

  SkPdfEncodingDictionary* getEncodingAsEncodingdictionary() const {
    SkPdfEncodingDictionary* ret = NULL;
    if (DictionaryFromDictionary2(fPodofoDoc, fPodofoObj->GetDictionary(), "Encoding", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return NULL;
  }

/** (Required) The first character code defined in the font's Widths array.
**/
  bool has_FirstChar() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "FirstChar", "", NULL));
  }

  long FirstChar() const {
    long ret;
    if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "FirstChar", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return 0;
  }

/** (Required) The last character code defined in the font's Widths array.
**/
  bool has_LastChar() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "LastChar", "", NULL));
  }

  long LastChar() const {
    long ret;
    if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "LastChar", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return 0;
  }

/** (Required; indirect reference preferred) An array of (LastChar - FirstChar + 1)
 *  widths, each element being the glyph width for the character whose code is
 *  FirstChar plus the array index. For character codes outside the range FirstChar
 *  to LastChar, the width is 0. These widths are interpreted in glyph space as
 *  specified by FontMatrix (unlike the widths of a Type 1 font, which are in
 *  thousandths of a unit of text space).
 *  Note: If FontMatrix specifies a rotation, only the horizontal component of the
 *  transformed width is used. That is, the resulting displacement is always horizon-
 *  tal in text space, as is the case for all simple fonts.
**/
  bool has_Widths() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Widths", "", NULL));
  }

  SkPdfArray* Widths() const {
    SkPdfArray* ret;
    if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Widths", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return NULL;
  }

/** (Optional but strongly recommended; PDF 1.2) A list of the named resources,
 *  such as fonts and images, required by the glyph descriptions in this font (see
 *  Section 3.7.2, "Resource Dictionaries"). If any glyph descriptions refer to
 *  named resources but this dictionary is absent, the names are looked up in the
 *  resource dictionary of the page on which the font is used. (See implementa-
 *  tion note 47 in Appendix H.)
**/
  bool has_Resources() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Resources", "", NULL));
  }

  SkPdfDictionary* Resources() const {
    SkPdfDictionary* ret;
    if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Resources", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return NULL;
  }

/** (Optional; PDF 1.2) A stream containing a CMap file that maps character
 *  codes to Unicode values (see Section 5.9, "ToUnicode CMaps").
**/
  bool has_ToUnicode() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "ToUnicode", "", NULL));
  }

  SkPdfStream* ToUnicode() const {
    SkPdfStream* ret;
    if (StreamFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "ToUnicode", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return NULL;
  }

};

#endif  // __DEFINED__SkPdfType3FontDictionary
