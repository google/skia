/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPdfType1FontDictionary_DEFINED
#define SkPdfType1FontDictionary_DEFINED

#include "SkPdfFontDictionary_autogen.h"

// Entries in a Type 1 font dictionary
class SkPdfType1FontDictionary : public SkPdfFontDictionary {
public:
public:
   SkPdfType1FontDictionary* asType1FontDictionary() {return this;}
   const SkPdfType1FontDictionary* asType1FontDictionary() const {return this;}

private:
   SkPdfType0FontDictionary* asType0FontDictionary() {return (SkPdfType0FontDictionary*)this;}
   const SkPdfType0FontDictionary* asType0FontDictionary() const {return (const SkPdfType0FontDictionary*)this;}

public:
   bool valid() const {return true;}
  SkString Type(SkPdfNativeDoc* doc);
  bool has_Type() const;
  SkString Subtype(SkPdfNativeDoc* doc);
  bool has_Subtype() const;
  SkString Name(SkPdfNativeDoc* doc);
  bool has_Name() const;
  SkString BaseFont(SkPdfNativeDoc* doc);
  bool has_BaseFont() const;
  int64_t FirstChar(SkPdfNativeDoc* doc);
  bool has_FirstChar() const;
  int64_t LastChar(SkPdfNativeDoc* doc);
  bool has_LastChar() const;
  SkPdfArray* Widths(SkPdfNativeDoc* doc);
  bool has_Widths() const;
  SkPdfFontDescriptorDictionary* FontDescriptor(SkPdfNativeDoc* doc);
  bool has_FontDescriptor() const;

  bool isEncodingAName(SkPdfNativeDoc* doc);
  SkString getEncodingAsName(SkPdfNativeDoc* doc);

  bool isEncodingADictionary(SkPdfNativeDoc* doc);
  SkPdfDictionary* getEncodingAsDictionary(SkPdfNativeDoc* doc);
  bool has_Encoding() const;
  SkPdfStream* ToUnicode(SkPdfNativeDoc* doc);
  bool has_ToUnicode() const;
};

#endif  // SkPdfType1FontDictionary_DEFINED
