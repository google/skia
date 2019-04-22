
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkXMLParser_DEFINED
#define SkXMLParser_DEFINED

#include "include/core/SkString.h"

class SkStream;

class SkDOM;
struct SkDOMNode;

class SkXMLParserError {
public:
    enum ErrorCode {
        kNoError,
        kEmptyFile,
        kUnknownElement,
        kUnknownAttributeName,
        kErrorInAttributeValue,
        kDuplicateIDs,
        kUnknownError
    };

    SkXMLParserError();
    virtual ~SkXMLParserError();
    ErrorCode getErrorCode() const { return fCode; }
    virtual void getErrorString(SkString* str) const;
    int getLineNumber() const { return fLineNumber; }
    int getNativeCode() const { return fNativeCode; }
    bool hasError() const { return fCode != kNoError || fNativeCode != -1; }
    bool hasNoun() const { return fNoun.size() > 0; }
    void reset();
    void setCode(ErrorCode code) { fCode = code; }
    void setNoun(const SkString& str) { fNoun.set(str); }
    void setNoun(const char* ch)  { fNoun.set(ch); }
    void setNoun(const char* ch, size_t len) { fNoun.set(ch, len); }
protected:
    ErrorCode fCode;
private:
    int fLineNumber;
    int fNativeCode;
    SkString fNoun;
    friend class SkXMLParser;
};

class SkXMLParser {
public:
    SkXMLParser(SkXMLParserError* parserError = nullptr);
    virtual ~SkXMLParser();

    /** Returns true for success
    */
    bool parse(const char doc[], size_t len);
    bool parse(SkStream& docStream);
    bool parse(const SkDOM&, const SkDOMNode*);

    static void GetNativeErrorString(int nativeErrorCode, SkString* str);

protected:
    // override in subclasses; return true to stop parsing
    virtual bool onStartElement(const char elem[]);
    virtual bool onAddAttribute(const char name[], const char value[]);
    virtual bool onEndElement(const char elem[]);
    virtual bool onText(const char text[], int len);

public:
    // public for ported implementation, not meant for clients to call
    bool startElement(const char elem[]);
    bool addAttribute(const char name[], const char value[]);
    bool endElement(const char elem[]);
    bool text(const char text[], int len);
    void* fParser;
protected:
    SkXMLParserError* fError;
private:
    void reportError(void* parser);
};

#endif
