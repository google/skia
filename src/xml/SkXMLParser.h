
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkXMLParser_DEFINED
#define SkXMLParser_DEFINED

#include "include/core/SkString.h"
#include "src/partition_alloc/raw_ptr.h"

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
    explicit SkXMLParser(SkXMLParserError* parserError = nullptr);
    virtual ~SkXMLParser();

    /** Returns true for success
    */
    bool parse(const char doc[], size_t len);
    bool parse(SkStream& docStream);
    bool parse(const SkDOM&, const SkDOMNode*);

    static void GetNativeErrorString(int nativeErrorCode, SkString* str);

protected:
    // override in subclasses
    virtual void onStartElement(const char elem[]);
    virtual void onAddAttribute(const char name[], const char value[]);
    virtual void onEndElement(const char elem[]);
    virtual void onText(const char text[], int len);

public:
    // public for ported implementation, not meant for clients to call
    void startElement(const char elem[]);
    void addAttribute(const char name[], const char value[]);
    void endElement(const char elem[]);
    void text(const char text[], int len);
    raw_ptr<void> fParser;

protected:
    raw_ptr<SkXMLParserError> fError;

private:
    void reportError(void* parser);
};

#endif
