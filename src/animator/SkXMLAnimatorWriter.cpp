/* libs/graphics/animator/SkXMLAnimatorWriter.cpp
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

#include "SkXMLAnimatorWriter.h"
#include "SkAnimator.h"
#include "SkAnimateMaker.h"
#include "SkDisplayXMLParser.h"

SkXMLAnimatorWriter::SkXMLAnimatorWriter(SkAnimator* animator) : fAnimator(animator)
{
    fParser = new SkDisplayXMLParser(*fAnimator->fMaker);
}

SkXMLAnimatorWriter::~SkXMLAnimatorWriter() {
    delete fParser;
}

void SkXMLAnimatorWriter::onAddAttributeLen(const char name[], const char value[], size_t length)
{
    fParser->onAddAttributeLen(name, value, length);
}

void SkXMLAnimatorWriter::onEndElement()
{
    Elem* elem = getEnd();
    fParser->onEndElement(elem->fName.c_str());
    doEnd(elem);
}

void SkXMLAnimatorWriter::onStartElementLen(const char name[], size_t length)
{
    doStart(name, length);
    fParser->onStartElementLen(name, length);
}

void SkXMLAnimatorWriter::writeHeader()
{
}

#ifdef SK_DEBUG
#include "SkCanvas.h"
#include "SkPaint.h"

void SkXMLAnimatorWriter::UnitTest(SkCanvas* canvas)
{
    SkAnimator  s;
    SkXMLAnimatorWriter     w(&s);
    w.startElement("screenplay");
        w.startElement("animateField");
            w.addAttribute("field", "x1");
            w.addAttribute("id", "to100");
            w.addAttribute("from", "0");
            w.addAttribute("to", "100");
            w.addAttribute("dur", "1");
        w.endElement();
        w.startElement("event");
            w.addAttribute("kind", "onLoad");
            w.startElement("line");
                w.addAttribute("id", "line");
                w.addAttribute("x1", "-1");
                w.addAttribute("y1", "20");
                w.addAttribute("x2", "150");
                w.addAttribute("y2", "40");
            w.endElement();
            w.startElement("apply");
                w.addAttribute("animator", "to100");
                w.addAttribute("scope", "line");
            w.endElement();
        w.endElement();
    w.endElement();
    SkPaint paint;
    canvas->drawColor(SK_ColorWHITE);
    s.draw(canvas, &paint, 0);
}

#endif

