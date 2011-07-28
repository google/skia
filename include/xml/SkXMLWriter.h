
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkXMLWriter_DEFINED
#define SkXMLWriter_DEFINED

#include "SkTDArray.h"
#include "SkString.h"
#include "SkDOM.h"

class SkWStream;
class SkXMLParser;

class SkXMLWriter {
public:
            SkXMLWriter(bool doEscapeMarkup = true);
    virtual ~SkXMLWriter();

    void    addS32Attribute(const char name[], int32_t value);
    void    addAttribute(const char name[], const char value[]);
    void    addAttributeLen(const char name[], const char value[], size_t length);
    void    addHexAttribute(const char name[], uint32_t value, int minDigits = 0);
    void    addScalarAttribute(const char name[], SkScalar value);
    void    endElement() { this->onEndElement(); }
    void    startElement(const char elem[]);
    void    startElementLen(const char elem[], size_t length);
    void    writeDOM(const SkDOM&, const SkDOM::Node*, bool skipRoot);
    void    flush();
    virtual void writeHeader();

protected:
    virtual void onStartElementLen(const char elem[], size_t length) = 0;
    virtual void onAddAttributeLen(const char name[], const char value[], size_t length) = 0;
    virtual void onEndElement() = 0;

    struct Elem {
        SkString    fName;
        bool        fHasChildren;
    };
    void doEnd(Elem* elem);
    bool doStart(const char name[], size_t length);
    Elem* getEnd();
    const char* getHeader();
    SkTDArray<Elem*> fElems;

private:
    bool fDoEscapeMarkup;
    // illegal
    SkXMLWriter& operator=(const SkXMLWriter&);
};

class SkXMLStreamWriter : public SkXMLWriter {
public:
    SkXMLStreamWriter(SkWStream*);
    virtual ~SkXMLStreamWriter();
    virtual void    writeHeader();
    SkDEBUGCODE(static void UnitTest();)
protected:
    virtual void onStartElementLen(const char elem[], size_t length);
    virtual void onEndElement();
    virtual void onAddAttributeLen(const char name[], const char value[], size_t length);
private:
    SkWStream&      fStream;
};

class SkXMLParserWriter : public SkXMLWriter {
public:
    SkXMLParserWriter(SkXMLParser*);
    virtual ~SkXMLParserWriter();
protected:
    virtual void onStartElementLen(const char elem[], size_t length);
    virtual void onEndElement();
    virtual void onAddAttributeLen(const char name[], const char value[], size_t length);
private:
    SkXMLParser&        fParser;
};


#endif

