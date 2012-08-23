
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkXMLParser.h"
#include "SkChunkAlloc.h"
#include "SkString.h"
#include "SkStream.h"

#include "expat.h"

static inline char* dupstr(SkChunkAlloc& chunk, const char src[], size_t len)
{
    SkASSERT(src);
    char*   dst = (char*)chunk.alloc(len + 1, SkChunkAlloc::kThrow_AllocFailType);

    memcpy(dst, src, len);
    dst[len] = 0;
    return dst;
}

static inline int count_pairs(const char** p)
{
    const char** start = p;
    while (*p)
    {
        SkASSERT(p[1] != NULL);
        p += 2;
    }
    return (p - start) >> 1;
}

struct Data {
    Data() : fAlloc(2048), fState(NORMAL) {}

    XML_Parser              fParser;
    SkXMLPullParser::Curr*  fCurr;
    SkChunkAlloc            fAlloc;

    enum State {
        NORMAL,
        MISSED_START_TAG,
        RETURN_END_TAG
    };
    State fState;
    const char* fEndTag;    // if state is RETURN_END_TAG
};

static void XMLCALL start_proc(void *data, const char *el, const char **attr)
{
    Data*                   p = (Data*)data;
    SkXMLPullParser::Curr*  c = p->fCurr;
    SkChunkAlloc&           alloc = p->fAlloc;

    c->fName = dupstr(alloc, el, strlen(el));

    int n = count_pairs(attr);
    SkXMLPullParser::AttrInfo* info = (SkXMLPullParser::AttrInfo*)alloc.alloc(n * sizeof(SkXMLPullParser::AttrInfo),
                                                                              SkChunkAlloc::kThrow_AllocFailType);
    c->fAttrInfoCount = n;
    c->fAttrInfos = info;

    for (int i = 0; i < n; i++)
    {
        info[i].fName = dupstr(alloc, attr[0], strlen(attr[0]));
        info[i].fValue = dupstr(alloc, attr[1], strlen(attr[1]));
        attr += 2;
    }

    c->fEventType = SkXMLPullParser::START_TAG;
    XML_StopParser(p->fParser, true);
}

static void XMLCALL end_proc(void *data, const char *el)
{
    Data*                   p = (Data*)data;
    SkXMLPullParser::Curr*  c = p->fCurr;

    if (c->fEventType == SkXMLPullParser::START_TAG)
    {
        /*  if we get here, we were called with a start_tag immediately
            followed by this end_tag. The caller will only see the end_tag,
            so we set a flag to notify them of the missed start_tag
        */
        p->fState = Data::MISSED_START_TAG;

        SkASSERT(c->fName != NULL);
        SkASSERT(strcmp(c->fName, el) == 0);
    }
    else
        c->fName = dupstr(p->fAlloc, el, strlen(el));

    c->fEventType = SkXMLPullParser::END_TAG;
    XML_StopParser(p->fParser, true);
}

#include <ctype.h>

static bool isws(const char s[])
{
    for (; *s; s++)
        if (!isspace(*s))
            return false;
    return true;
}

static void XMLCALL text_proc(void* data, const char* text, int len)
{
    Data*                   p = (Data*)data;
    SkXMLPullParser::Curr*  c = p->fCurr;

    c->fName = dupstr(p->fAlloc, text, len);
    c->fIsWhitespace = isws(c->fName);

    c->fEventType = SkXMLPullParser::TEXT;
    XML_StopParser(p->fParser, true);
}

//////////////////////////////////////////////////////////////////////////

struct SkXMLPullParser::Impl {
    Data    fData;
    void*   fBuffer;
    size_t  fBufferLen;
};

static void reportError(XML_Parser parser)
{
    XML_Error code = XML_GetErrorCode(parser);
    int lineNumber = XML_GetCurrentLineNumber(parser);
    const char* msg = XML_ErrorString(code);

    printf("-------- XML error [%d] on line %d, %s\n", code, lineNumber, msg);
}

bool SkXMLPullParser::onInit()
{
    fImpl = new Impl;

    XML_Parser p = XML_ParserCreate(NULL);
    SkASSERT(p);

    fImpl->fData.fParser = p;
    fImpl->fData.fCurr = &fCurr;

    XML_SetElementHandler(p, start_proc, end_proc);
    XML_SetCharacterDataHandler(p, text_proc);
    XML_SetUserData(p, &fImpl->fData);

    size_t len = fStream->read(NULL, 0);
    fImpl->fBufferLen = len;
    fImpl->fBuffer = sk_malloc_throw(len);
    fStream->rewind();
    size_t  len2 = fStream->read(fImpl->fBuffer, len);
    return len2 == len;
}

void SkXMLPullParser::onExit()
{
    sk_free(fImpl->fBuffer);
    XML_ParserFree(fImpl->fData.fParser);
    delete fImpl;
    fImpl = NULL;
}

SkXMLPullParser::EventType SkXMLPullParser::onNextToken()
{
    if (Data::RETURN_END_TAG == fImpl->fData.fState)
    {
        fImpl->fData.fState = Data::NORMAL;
        fCurr.fName = fImpl->fData.fEndTag; // restore name from (below) save
        return SkXMLPullParser::END_TAG;
    }

    fImpl->fData.fAlloc.reset();

    XML_Parser p = fImpl->fData.fParser;
    XML_Status status;

    status = XML_ResumeParser(p);

CHECK_STATUS:
    switch (status) {
    case XML_STATUS_OK:
        return SkXMLPullParser::END_DOCUMENT;

    case XML_STATUS_ERROR:
        if (XML_GetErrorCode(p) != XML_ERROR_NOT_SUSPENDED)
        {
            reportError(p);
            return SkXMLPullParser::ERROR;
        }
        status = XML_Parse(p, (const char*)fImpl->fBuffer, fImpl->fBufferLen, true);
        goto CHECK_STATUS;

    case XML_STATUS_SUSPENDED:
        if (Data::MISSED_START_TAG == fImpl->fData.fState)
        {
            // return a start_tag, and clear the flag so we return end_tag next
            SkASSERT(SkXMLPullParser::END_TAG == fCurr.fEventType);
            fImpl->fData.fState = Data::RETURN_END_TAG;
            fImpl->fData.fEndTag = fCurr.fName;  // save this pointer
            return SkXMLPullParser::START_TAG;
        }
        break;
    }
    return fCurr.fEventType;
}

