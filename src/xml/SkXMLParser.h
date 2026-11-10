
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkXMLParser_DEFINED
#define SkXMLParser_DEFINED

#include "include/core/SkSpan.h"
#include "include/core/SkString.h"
#include "include/private/SkOnlyStackAllocatable.h"

#include <string_view>

class SkStream;

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
    SK_ONLY_STACK_ALLOCATABLE;

public:
    struct Attribute {
        std::string_view fName;
        std::string_view fValue;
    };

    explicit SkXMLParser(SkXMLParserError* parserError = nullptr);
    virtual ~SkXMLParser();

    /** Returns true for success
    */
    bool parse(const char doc[], size_t len);
    bool parse(SkStream& docStream);

    static void GetNativeErrorString(int nativeErrorCode, SkString* str);

protected:
    // Override in subclasses. The std::string_view and SkSpan<const Attribute>
    // arguments are only valid for the duration of the callback and are not
    // guaranteed to be null-terminated.
    virtual void onStartElement(std::string_view elem, SkSpan<const Attribute> attributes);
    virtual void onEndElement(std::string_view elem);
    virtual void onText(std::string_view text);

    SkXMLParserError* fError;

private:
    struct ParsingContext;
};

#endif
