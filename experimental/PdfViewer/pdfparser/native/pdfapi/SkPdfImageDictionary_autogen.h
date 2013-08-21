/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPdfImageDictionary_DEFINED
#define SkPdfImageDictionary_DEFINED

#include "SkPdfXObjectDictionary_autogen.h"

// Additional entries specific to an image dictionary
class SkPdfImageDictionary : public SkPdfXObjectDictionary {
public:
public:
   SkPdfImageDictionary* asImageDictionary() {return this;}
   const SkPdfImageDictionary* asImageDictionary() const {return this;}

private:
   SkPdfType1FormDictionary* asType1FormDictionary() {return (SkPdfType1FormDictionary*)this;}
   const SkPdfType1FormDictionary* asType1FormDictionary() const {return (const SkPdfType1FormDictionary*)this;}

public:
   bool valid() const {return true;}
  SkString Type(SkPdfNativeDoc* doc);
  bool has_Type() const;
  SkString Subtype(SkPdfNativeDoc* doc);
  bool has_Subtype() const;
  int64_t Width(SkPdfNativeDoc* doc);
  bool has_Width() const;
  int64_t Height(SkPdfNativeDoc* doc);
  bool has_Height() const;

  bool isColorSpaceAName(SkPdfNativeDoc* doc);
  SkString getColorSpaceAsName(SkPdfNativeDoc* doc);

  bool isColorSpaceAArray(SkPdfNativeDoc* doc);
  SkPdfArray* getColorSpaceAsArray(SkPdfNativeDoc* doc);
  bool has_ColorSpace() const;
  int64_t BitsPerComponent(SkPdfNativeDoc* doc);
  bool has_BitsPerComponent() const;
  SkString Intent(SkPdfNativeDoc* doc);
  bool has_Intent() const;
  bool ImageMask(SkPdfNativeDoc* doc);
  bool has_ImageMask() const;

  bool isMaskAStream(SkPdfNativeDoc* doc);
  SkPdfStream* getMaskAsStream(SkPdfNativeDoc* doc);

  bool isMaskAArray(SkPdfNativeDoc* doc);
  SkPdfArray* getMaskAsArray(SkPdfNativeDoc* doc);
  bool has_Mask() const;
  SkPdfImageDictionary* SMask(SkPdfNativeDoc* doc);
  bool has_SMask() const;
  SkPdfArray* Decode(SkPdfNativeDoc* doc);
  bool has_Decode() const;
  bool Interpolate(SkPdfNativeDoc* doc);
  bool has_Interpolate() const;
  SkPdfArray* Alternates(SkPdfNativeDoc* doc);
  bool has_Alternates() const;
  SkString Name(SkPdfNativeDoc* doc);
  bool has_Name() const;
  int64_t StructParent(SkPdfNativeDoc* doc);
  bool has_StructParent() const;
  SkString ID(SkPdfNativeDoc* doc);
  bool has_ID() const;
  SkPdfDictionary* OPI(SkPdfNativeDoc* doc);
  bool has_OPI() const;
  SkPdfStream* Metadata(SkPdfNativeDoc* doc);
  bool has_Metadata() const;
};

#endif  // SkPdfImageDictionary_DEFINED
