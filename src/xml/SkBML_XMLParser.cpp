
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkBML_XMLParser.h"
#include "SkBML_Verbs.h"
#include "SkStream.h"
#include "SkXMLWriter.h"

static uint8_t rbyte(SkStream& s)
{
    uint8_t b;
    size_t size = s.read(&b, 1);
    SkASSERT(size == 1);
    return b;
}

static int rdata(SkStream& s, int data)
{
    SkASSERT((data & ~31) == 0);
    if (data == 31)
    {
        data = rbyte(s);
        if (data == 0xFF)
        {
            data = rbyte(s);
            data = (data << 8) | rbyte(s);
        }
    }
    return data;
}

static void set(char* array[256], int index, SkStream& s, int data)
{
    SkASSERT((unsigned)index <= 255);

    size_t size = rdata(s, data);

    if (array[index] == NULL)
        array[index] = (char*)sk_malloc_throw(size + 1);
    else
    {
        if (strlen(array[index]) < size)
            array[index] = (char*)sk_realloc_throw(array[index], size + 1);
    }

    s.read(array[index], size);
    array[index][size] = 0;
}

static void freeAll(char* array[256])
{
    for (int i = 0; i < 256; i++)
        sk_free(array[i]);
}

struct BMLW {
    char*   fElems[256];
    char*   fNames[256];
    char*   fValues[256];

    // important that these are uint8_t, so we get automatic wrap-around
    uint8_t  fNextElem, fNextName, fNextValue;

    BMLW()
    {
        memset(fElems, 0, sizeof(fElems));
        memset(fNames, 0, sizeof(fNames));
        memset(fValues, 0, sizeof(fValues));

        fNextElem = fNextName = fNextValue = 0;
    }
    ~BMLW()
    {
        freeAll(fElems);
        freeAll(fNames);
        freeAll(fValues);
    }
};

static void rattr(unsigned verb, SkStream& s, BMLW& rec, SkXMLWriter& writer)
{
    int data = verb & 31;
    verb >>= 5;

    int nameIndex, valueIndex;

    switch (verb) {
    case kAttr_Value_Value_Verb:
        nameIndex = rec.fNextName;      // record before the ++
        set(rec.fNames, rec.fNextName++, s, data);
        valueIndex = rec.fNextValue;    // record before the ++
        set(rec.fValues, rec.fNextValue++, s, 31);
        break;
    case kAttr_Value_Index_Verb:
        nameIndex = rec.fNextName;      // record before the ++
        set(rec.fNames, rec.fNextName++, s, data);
        valueIndex = rbyte(s);
        break;
    case kAttr_Index_Value_Verb:
        nameIndex = rdata(s, data);
        valueIndex = rec.fNextValue;    // record before the ++
        set(rec.fValues, rec.fNextValue++, s, 31);
        break;
    case kAttr_Index_Index_Verb:
        nameIndex = rdata(s, data);
        valueIndex = rbyte(s);
        break;
    default:
        SkASSERT(!"bad verb");
        return;
    }
    writer.addAttribute(rec.fNames[nameIndex], rec.fValues[valueIndex]);
}

static void relem(unsigned verb, SkStream& s, BMLW& rec, SkXMLWriter& writer)
{
    int data = verb & 31;
    verb >>= 5;

    int elemIndex;

    if (verb == kStartElem_Value_Verb)
    {
        elemIndex = rec.fNextElem;      // record before the ++
        set(rec.fElems, rec.fNextElem++, s, data);
    }
    else
    {
        SkASSERT(verb == kStartElem_Index_Verb);
        elemIndex = rdata(s, data);
    }

    writer.startElement(rec.fElems[elemIndex]);

    for (;;)
    {
        verb = rbyte(s);
        switch (verb >> 5) {
        case kAttr_Value_Value_Verb:
        case kAttr_Value_Index_Verb:
        case kAttr_Index_Value_Verb:
        case kAttr_Index_Index_Verb:
            rattr(verb, s, rec, writer);
            break;
        case kStartElem_Value_Verb:
        case kStartElem_Index_Verb:
            relem(verb, s, rec, writer);
            break;
        case kEndElem_Verb:
            writer.endElement();
            return;
        default:
            SkASSERT(!"bad verb");
        }
    }
}

void BML_XMLParser::Read(SkStream& s, SkXMLWriter& writer)
{
    BMLW rec;
    writer.writeHeader();
    relem(rbyte(s), s, rec, writer);
}

void BML_XMLParser::Read(SkStream& s, SkWStream& output)
{
    SkXMLStreamWriter writer(&output);
    Read(s, writer);
}

void BML_XMLParser::Read(SkStream& s, SkXMLParser& output)
{
    SkXMLParserWriter writer(&output);
    Read(s, writer);
}



