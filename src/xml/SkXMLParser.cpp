/* libs/graphics/xml/SkXMLParser.cpp
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#include "SkXMLParser.h"

static char const* const gErrorStrings[] = {
    "empty or missing file ",
    "unknown element ",
    "unknown attribute name ",
    "error in attribute value ",
    "duplicate ID ",
    "unknown error "
};

SkXMLParserError::SkXMLParserError() : fCode(kNoError), fLineNumber(-1),
    fNativeCode(-1)
{
    reset();
}

SkXMLParserError::~SkXMLParserError()
{
    // need a virtual destructor for our subclasses
}

void SkXMLParserError::getErrorString(SkString* str) const
{
    SkASSERT(str);
    SkString temp;
    if (fCode != kNoError) {
        if ((unsigned)fCode < SK_ARRAY_COUNT(gErrorStrings))
            temp.set(gErrorStrings[fCode - 1]);
        temp.append(fNoun);
    } else
        SkXMLParser::GetNativeErrorString(fNativeCode, &temp);
    str->append(temp);
}

void SkXMLParserError::reset() {
    fCode = kNoError;
    fLineNumber = -1;
    fNativeCode = -1;
}


////////////////

SkXMLParser::SkXMLParser(SkXMLParserError* parserError) : fParser(NULL), fError(parserError)
{
}

SkXMLParser::~SkXMLParser()
{
}

bool SkXMLParser::startElement(const char elem[])
{
    return this->onStartElement(elem);
}

bool SkXMLParser::addAttribute(const char name[], const char value[])
{
    return this->onAddAttribute(name, value);
}

bool SkXMLParser::endElement(const char elem[])
{
    return this->onEndElement(elem);
}

bool SkXMLParser::text(const char text[], int len) 
{
    return this->onText(text, len);
}

////////////////////////////////////////////////////////////////////////////////

bool SkXMLParser::onStartElement(const char elem[]) {return false; }
bool SkXMLParser::onAddAttribute(const char name[], const char value[]) {return false; }
bool SkXMLParser::onEndElement(const char elem[]) { return false; }
bool SkXMLParser::onText(const char text[], int len) {return false; }
