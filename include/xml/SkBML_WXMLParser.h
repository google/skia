
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkBML_WXMLParser_DEFINED
#define SkBML_WXMLParser_DEFINED

#include "SkString.h"
#include "SkXMLParser.h"

class SkStream;
class SkWStream;

class BML_WXMLParser : public SkXMLParser {
public:
    BML_WXMLParser(SkWStream& writer);
    virtual ~BML_WXMLParser();
    static void Write(SkStream& s, const char filename[]);

  /** @cond UNIT_TEST */
  SkDEBUGCODE(static void UnitTest();)
  /** @endcond */
private:
    virtual bool onAddAttribute(const char name[], const char value[]);
    virtual bool onEndElement(const char name[]);
    virtual bool onStartElement(const char name[]);
    BML_WXMLParser& operator=(const BML_WXMLParser& src);
#ifdef SK_DEBUG
    int fElemsCount, fElemsReused;
    int fAttrsCount, fNamesReused, fValuesReused;
#endif
    SkWStream&  fWriter;
    char*       fElems[256];
    char*       fAttrNames[256];
    char*       fAttrValues[256];

    // important that these are U8, so we get automatic wrap-around
    uint8_t  fNextElem, fNextAttrName, fNextAttrValue;
};

#endif // SkBML_WXMLParser_DEFINED
