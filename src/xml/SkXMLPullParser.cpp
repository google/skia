/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkXMLParser.h"
#include "SkStream.h"

static void reset(SkXMLPullParser::Curr* curr)
{
    curr->fEventType = SkXMLPullParser::ERROR;
    curr->fName = "";
    curr->fAttrInfoCount = 0;
    curr->fIsWhitespace = false;
}

SkXMLPullParser::SkXMLPullParser() : fStream(nullptr)
{
    fCurr.fEventType = ERROR;
    fDepth = -1;
}

SkXMLPullParser::SkXMLPullParser(SkStream* stream) : fStream(nullptr)
{
    fCurr.fEventType = ERROR;
    fDepth = 0;

    this->setStream(stream);
}

SkXMLPullParser::~SkXMLPullParser()
{
    this->setStream(nullptr);
}

SkStream* SkXMLPullParser::setStream(SkStream* stream)
{
    if (fStream && !stream)
        this->onExit();

    SkRefCnt_SafeAssign(fStream, stream);

    if (fStream)
    {
        fCurr.fEventType = START_DOCUMENT;
        this->onInit();
    }
    else
    {
        fCurr.fEventType = ERROR;
    }
    fDepth = 0;

    return fStream;
}

SkXMLPullParser::EventType SkXMLPullParser::nextToken()
{
    switch (fCurr.fEventType) {
    case ERROR:
    case END_DOCUMENT:
        break;
    case END_TAG:
        fDepth -= 1;
        // fall through
    default:
        reset(&fCurr);
        fCurr.fEventType = this->onNextToken();
        break;
    }

    switch (fCurr.fEventType) {
    case START_TAG:
        fDepth += 1;
        break;
    default:
        break;
    }

    return fCurr.fEventType;
}

const char* SkXMLPullParser::getName()
{
    switch (fCurr.fEventType) {
    case START_TAG:
    case END_TAG:
        return fCurr.fName;
    default:
        return nullptr;
    }
}

const char* SkXMLPullParser::getText()
{
    switch (fCurr.fEventType) {
    case TEXT:
    case IGNORABLE_WHITESPACE:
        return fCurr.fName;
    default:
        return nullptr;
    }
}

bool SkXMLPullParser::isWhitespace()
{
    switch (fCurr.fEventType) {
    case IGNORABLE_WHITESPACE:
        return true;
    case TEXT:
    case CDSECT:
        return fCurr.fIsWhitespace;
    default:
        return false;   // unknown/illegal
    }
}

int SkXMLPullParser::getAttributeCount()
{
    return fCurr.fAttrInfoCount;
}

void SkXMLPullParser::getAttributeInfo(int index, AttrInfo* info)
{
    SkASSERT((unsigned)index < (unsigned)fCurr.fAttrInfoCount);

    if (info)
        *info = fCurr.fAttrInfos[index];
}

bool SkXMLPullParser::onEntityReplacement(const char name[],
                                          SkString* replacement)
{
    // TODO: std 5 entities here
    return false;
}
