
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkDisplayXMLParser_DEFINED
#define SkDisplayXMLParser_DEFINED

#include "SkIntArray.h"
#include "SkTDict.h"
#include "SkDisplayType.h"
#include "SkXMLParser.h"

class SkAnimateMaker;
class SkDisplayable;

class SkDisplayXMLParserError : public SkXMLParserError {
public:
    enum ErrorCode {
        kApplyScopesItself = kUnknownError + 1,
        kDisplayTreeTooDeep,
        kElementMissingParent,
        kElementTypeNotAllowedInParent,
        kErrorAddingDataToPost,
        kErrorAddingToMatrix,
        kErrorAddingToPaint,
        kErrorAddingToPath,
        kErrorInAttributeValue,
        kErrorInScript,
        kExpectedMovie,
        kFieldNotInTarget,
        kGradientOffsetsDontMatchColors,
        kGradientOffsetsMustBeNoMoreThanOne,
        kGradientOffsetsMustEndWithOne,
        kGradientOffsetsMustIncrease,
        kGradientOffsetsMustStartWithZero,
        kGradientPointsLengthMustBeFour,
        kInInclude,
        kInMovie,
        kIncludeNameUnknownOrMissing,
        kIndexOutOfRange,
        kMovieNameUnknownOrMissing,
        kNoParentAvailable,
        kParentElementCantContain,
        kSaveLayerNeedsBounds,
        kTargetIDNotFound,
        kUnexpectedType
    };
    virtual ~SkDisplayXMLParserError();
    virtual void getErrorString(SkString* str) const;
    void setCode(ErrorCode code) { INHERITED::setCode((INHERITED::ErrorCode) code); }
    void setInnerError(SkAnimateMaker* maker, const SkString& str);
    typedef SkXMLParserError INHERITED;
    friend class SkDisplayXMLParser;
};

class SkDisplayXMLParser : public SkXMLParser {
public:
    SkDisplayXMLParser(SkAnimateMaker& maker);
    virtual ~SkDisplayXMLParser();
protected:
    virtual bool onAddAttribute(const char name[], const char value[]);
    bool onAddAttributeLen(const char name[], const char value[], size_t len);
    virtual bool onEndElement(const char elem[]);
    virtual bool onStartElement(const char elem[]);
    bool onStartElementLen(const char elem[], size_t len);
private:
    struct Parent {
        SkDisplayable* fDisplayable;
        SkDisplayTypes fType;
    };
    SkTDArray<Parent> fParents;
    SkDisplayXMLParser& operator= (const SkDisplayXMLParser& );
    SkDisplayXMLParserError* getError() { return (SkDisplayXMLParserError*) fError; }
    const SkMemberInfo* searchContainer(const SkMemberInfo* ,
        int infoCount);
    SkAnimateMaker& fMaker;
    SkBool fInInclude;
    SkBool fInSkia;
    // local state between onStartElement and onAddAttribute
    SkDisplayable*  fCurrDisplayable;
    SkDisplayTypes  fCurrType;
    friend class SkXMLAnimatorWriter;
    typedef SkXMLParser INHERITED;
};

#endif // SkDisplayXMLParser_DEFINED
