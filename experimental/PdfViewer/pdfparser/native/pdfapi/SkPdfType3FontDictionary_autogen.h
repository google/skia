/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPdfType3FontDictionary_DEFINED
#define SkPdfType3FontDictionary_DEFINED

#include "SkPdfType1FontDictionary_autogen.h"

// Entries in a Type 3 font dictionary
class SkPdfType3FontDictionary : public SkPdfType1FontDictionary {
public:
public:
   SkPdfType3FontDictionary* asType3FontDictionary() {return this;}
   const SkPdfType3FontDictionary* asType3FontDictionary() const {return this;}

private:
   SkPdfMultiMasterFontDictionary* asMultiMasterFontDictionary() {return (SkPdfMultiMasterFontDictionary*)this;}
   const SkPdfMultiMasterFontDictionary* asMultiMasterFontDictionary() const {return (const SkPdfMultiMasterFontDictionary*)this;}

   SkPdfTrueTypeFontDictionary* asTrueTypeFontDictionary() {return (SkPdfTrueTypeFontDictionary*)this;}
   const SkPdfTrueTypeFontDictionary* asTrueTypeFontDictionary() const {return (const SkPdfTrueTypeFontDictionary*)this;}

public:
   bool valid() const {return true;}
  SkString Type(SkPdfNativeDoc* doc);
  bool has_Type() const;
  SkString Subtype(SkPdfNativeDoc* doc);
  bool has_Subtype() const;
  SkString Name(SkPdfNativeDoc* doc);
  bool has_Name() const;
  SkRect FontBBox(SkPdfNativeDoc* doc);
  bool has_FontBBox() const;
  SkMatrix FontMatrix(SkPdfNativeDoc* doc);
  bool has_FontMatrix() const;
  SkPdfDictionary* CharProcs(SkPdfNativeDoc* doc);
  bool has_CharProcs() const;

  bool isEncodingAName(SkPdfNativeDoc* doc);
  SkString getEncodingAsName(SkPdfNativeDoc* doc);

  bool isEncodingAEncodingdictionary(SkPdfNativeDoc* doc);
  SkPdfEncodingDictionary* getEncodingAsEncodingdictionary(SkPdfNativeDoc* doc);
  bool has_Encoding() const;
  int64_t FirstChar(SkPdfNativeDoc* doc);
  bool has_FirstChar() const;
  int64_t LastChar(SkPdfNativeDoc* doc);
  bool has_LastChar() const;
  SkPdfArray* Widths(SkPdfNativeDoc* doc);
  bool has_Widths() const;
  SkPdfResourceDictionary* Resources(SkPdfNativeDoc* doc);
  bool has_Resources() const;
  SkPdfStream* ToUnicode(SkPdfNativeDoc* doc);
  bool has_ToUnicode() const;
};

#endif  // SkPdfType3FontDictionary_DEFINED
