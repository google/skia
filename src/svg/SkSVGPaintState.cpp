
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkSVGPaintState.h"
#include "SkSVGElements.h"
#include "SkSVGParser.h"
#include "SkParse.h"

SkSVGAttribute SkSVGPaint::gAttributes[] = {
    SVG_LITERAL_ATTRIBUTE(clip-path, f_clipPath),
    SVG_LITERAL_ATTRIBUTE(clip-rule, f_clipRule),
    SVG_LITERAL_ATTRIBUTE(enable-background, f_enableBackground),
    SVG_ATTRIBUTE(fill),
    SVG_LITERAL_ATTRIBUTE(fill-rule, f_fillRule),
    SVG_ATTRIBUTE(filter),
    SVG_LITERAL_ATTRIBUTE(font-family, f_fontFamily),
    SVG_LITERAL_ATTRIBUTE(font-size, f_fontSize),
    SVG_LITERAL_ATTRIBUTE(letter-spacing, f_letterSpacing),
    SVG_ATTRIBUTE(mask),
    SVG_ATTRIBUTE(opacity),
    SVG_LITERAL_ATTRIBUTE(stop-color, f_stopColor),
    SVG_LITERAL_ATTRIBUTE(stop-opacity, f_stopOpacity),
    SVG_ATTRIBUTE(stroke),
    SVG_LITERAL_ATTRIBUTE(stroke-dasharray, f_strokeDasharray),
    SVG_LITERAL_ATTRIBUTE(stroke-linecap, f_strokeLinecap),
    SVG_LITERAL_ATTRIBUTE(stroke-linejoin, f_strokeLinejoin),
    SVG_LITERAL_ATTRIBUTE(stroke-miterlimit, f_strokeMiterlimit),
    SVG_LITERAL_ATTRIBUTE(stroke-width, f_strokeWidth),
    SVG_ATTRIBUTE(style),
    SVG_ATTRIBUTE(transform)
};

const int SkSVGPaint::kAttributesSize = SK_ARRAY_COUNT(SkSVGPaint::gAttributes);

SkSVGPaint::SkSVGPaint() : fNext(NULL) {
}

SkString* SkSVGPaint::operator[](int index) {
    SkASSERT(index >= 0);
    SkASSERT(index < &fTerminal - &fInitial);
    SkASSERT(&fTerminal - &fInitial == kTerminal - kInitial);
    SkString* result = &fInitial + index + 1;
    return result;
}

void SkSVGPaint::addAttribute(SkSVGParser& parser, int attrIndex,
        const char* attrValue, size_t attrLength) {
    SkString* attr = (*this)[attrIndex];
    switch(attrIndex) {
        case kClipPath:
        case kClipRule:
        case kEnableBackground:
        case kFill:
        case kFillRule:
        case kFilter:
        case kFontFamily:
        case kFontSize:
        case kLetterSpacing:
        case kMask:
        case kOpacity:
        case kStopColor:
        case kStopOpacity:
        case kStroke:
        case kStroke_Dasharray:
        case kStroke_Linecap:
        case kStroke_Linejoin:
        case kStroke_Miterlimit:
        case kStroke_Width:
        case kTransform:
            attr->set(attrValue, attrLength);
            return;
        case kStyle: {
            // iterate through colon / semi-colon delimited pairs
            int pairs = SkParse::Count(attrValue, ';');
            const char* attrEnd = attrValue + attrLength;
            do {
                const char* end = strchr(attrValue, ';');
                if (end == NULL)
                    end = attrEnd;
                const char* delimiter = strchr(attrValue, ':');
                SkASSERT(delimiter != 0 && delimiter < end);
                int index = parser.findAttribute(this, attrValue, (int) (delimiter - attrValue), true);
                SkASSERT(index >= 0);
                delimiter++;
                addAttribute(parser, index, delimiter, (int) (end - delimiter));
                attrValue = end + 1;
            } while (--pairs);
            return;
            }
        default:
            SkASSERT(0);
    }
}

bool SkSVGPaint::flush(SkSVGParser& parser, bool isFlushable, bool isDef) {
    SkSVGPaint current;
    SkSVGPaint* walking = parser.fHead;
    int index;
    while (walking != NULL) {
        for (index = kInitial + 1; index < kTerminal; index++) {
            SkString* lastAttr = (*walking)[index];
            if (lastAttr->size() == 0)
                continue;
            if (current[index]->size() > 0)
                continue;
            current[index]->set(*lastAttr);
        }
        walking = walking->fNext;
    }
    bool paintChanged = false;
    SkSVGPaint& lastState = parser.fLastFlush;
    if (isFlushable == false) {
        if (isDef == true) {
            if (current.f_mask.size() > 0 && current.f_mask.equals(lastState.f_mask) == false) {
                SkSVGElement* found;
                const char* idStart = strchr(current.f_mask.c_str(), '#');
                SkASSERT(idStart);
                SkString id(idStart + 1, strlen(idStart) - 2);
                bool itsFound = parser.fIDs.find(id.c_str(), &found);
                SkASSERT(itsFound);
                SkSVGElement* gradient = found->getGradient();
                if (gradient) {
                    gradient->write(parser, current.f_fill);
                    gradient->write(parser, current.f_stroke);
                }
            }
        }
        goto setLast;
    }
    {
        bool changed[kTerminal];
        memset(changed, 0, sizeof(changed));
        for (index = kInitial + 1; index < kTerminal; index++) {
            if (index == kTransform || index == kClipPath || index == kStopColor || index == kStopOpacity ||
                    index == kClipRule || index == kFillRule)
                continue;
            SkString* lastAttr = lastState[index];
            SkString* currentAttr = current[index];
            paintChanged |= changed[index] = lastAttr->equals(*currentAttr) == false;
        }
        if (paintChanged) {
            if (current.f_mask.size() > 0) {
                if (current.f_fill.equals("none") == false && strncmp(current.f_fill.c_str(), "url(#", 5) != 0) {
                    SkASSERT(current.f_fill.c_str()[0] == '#');
                    SkString replacement("url(#mask");
                    replacement.append(current.f_fill.c_str() + 1);
                    replacement.appendUnichar(')');
                    current.f_fill.set(replacement);
                }
                if (current.f_stroke.equals("none") == false && strncmp(current.f_stroke.c_str(), "url(#", 5) != 0) {
                    SkASSERT(current.f_stroke.c_str()[0] == '#');
                    SkString replacement("url(#mask");
                    replacement.append(current.f_stroke.c_str() + 1);
                    replacement.appendUnichar(')');
                    current.f_stroke.set(replacement);
                }
            }
            if (current.f_fill.equals("none") && current.f_stroke.equals("none"))
                current.f_opacity.set("0");
            if (parser.fSuppressPaint == false) {
                parser._startElement("paint");
                bool success = writeChangedAttributes(parser, current, changed);
                if (success == false)
                    return paintChanged;
                success = writeChangedElements(parser, current, changed);
                if (success == false)
                    return paintChanged;
                parser._endElement(); // paint
            }
        }
    }
setLast:
    for (index = kInitial + 1; index < kTerminal; index++) {
        SkString* lastAttr = lastState[index];
        SkString* currentAttr = current[index];
        lastAttr->set(*currentAttr);
    }
    return paintChanged;
}

int SkSVGPaint::getAttributes(const SkSVGAttribute** attrPtr) {
    *attrPtr = gAttributes;
    return kAttributesSize;
}

void SkSVGPaint::setSave(SkSVGParser& parser) {
    SkTDArray<SkString*> clips;
    SkSVGPaint* walking = parser.fHead;
    int index;
    SkMatrix sum;
    sum.reset();
    while (walking != NULL) {
        for (index = kInitial + 1; index < kTerminal; index++) {
            SkString* lastAttr = (*walking)[index];
            if (lastAttr->size() == 0)
                continue;
            if (index == kTransform) {
                const char* str = lastAttr->c_str();
                SkASSERT(strncmp(str, "matrix(", 7) == 0);
                str += 6;
                const char* strEnd = strrchr(str, ')');
                SkASSERT(strEnd != NULL);
                SkString mat(str, strEnd - str);
                SkSVGParser::ConvertToArray(mat);
                SkScalar values[6];
                SkParse::FindScalars(mat.c_str() + 1, values, 6);
                SkMatrix matrix;
                matrix.reset();
                matrix.setScaleX(values[0]);
                matrix.setSkewY(values[1]);
                matrix.setSkewX(values[2]);
                matrix.setScaleY(values[3]);
                matrix.setTranslateX(values[4]);
                matrix.setTranslateY(values[5]);
                sum.setConcat(matrix, sum);
                continue;
            }
            if ( index == kClipPath)
                *clips.insert(0) = lastAttr;
        }
        walking = walking->fNext;
    }
    if ((sum == parser.fLastTransform) == false) {
        SkMatrix inverse;
        bool success = parser.fLastTransform.invert(&inverse);
        SkASSERT(success == true);
        SkMatrix output;
        output.setConcat(inverse, sum);
        parser.fLastTransform = sum;
        SkString outputStr;
        outputStr.appendUnichar('[');
        outputStr.appendScalar(output.getScaleX());
        outputStr.appendUnichar(',');
        outputStr.appendScalar(output.getSkewX());
        outputStr.appendUnichar(',');
        outputStr.appendScalar(output.getTranslateX());
        outputStr.appendUnichar(',');
        outputStr.appendScalar(output.getSkewY());
        outputStr.appendUnichar(',');
        outputStr.appendScalar(output.getScaleY());
        outputStr.appendUnichar(',');
        outputStr.appendScalar(output.getTranslateY());
        outputStr.appendUnichar(',');
        outputStr.appendScalar(output.getPerspX());
        outputStr.appendUnichar(',');
        outputStr.appendScalar(output.getPerspY());
        outputStr.append(",1]");
        parser._startElement("matrix");
        parser._addAttributeLen("matrix", outputStr.c_str(), outputStr.size());
        parser._endElement();
    }
#if 0   // incomplete
    if (parser.fTransformClips.size() > 0) {
        // need to reset the clip when the 'g' scope is ended
        parser._startElement("add");
        const char* start = strchr(current->f_clipPath.c_str(), '#') + 1;
        SkASSERT(start);
        parser._addAttributeLen("use", start, strlen(start) - 1);
        parser._endElement();   // clip
    }
#endif
}

bool SkSVGPaint::writeChangedAttributes(SkSVGParser& parser,
        SkSVGPaint& current, bool* changed) {
    SkSVGPaint& lastState = parser.fLastFlush;
    for (int index = kInitial + 1; index < kTerminal; index++) {
        if (changed[index] == false)
                continue;
        SkString* topAttr = current[index];
        size_t attrLength = topAttr->size();
        if (attrLength == 0)
            continue;
        const char* attrValue = topAttr->c_str();
        SkString* lastAttr = lastState[index];
        switch(index) {
            case kClipPath:
            case kClipRule:
            case kEnableBackground:
                break;
            case kFill:
                if (topAttr->equals("none") == false && lastAttr->equals("none") == true)
                    parser._addAttribute("stroke", "false");
                goto fillStrokeAttrCommon;
            case kFillRule:
            case kFilter:
            case kFontFamily:
                break;
            case kFontSize:
                parser._addAttributeLen("textSize", attrValue, attrLength);
                break;
            case kLetterSpacing:
                parser._addAttributeLen("textTracking", attrValue, attrLength);
                break;
            case kMask:
                break;
            case kOpacity:
                break;
            case kStopColor:
                break;
            case kStopOpacity:
                break;
            case kStroke:
                if (topAttr->equals("none") == false && lastAttr->equals("none") == true)
                    parser._addAttribute("stroke", "true");
fillStrokeAttrCommon:
                if (strncmp(attrValue, "url(", 4) == 0) {
                    SkASSERT(attrValue[4] == '#');
                    const char* idStart = attrValue + 5;
                    const char* idEnd = strrchr(attrValue, ')');
                    SkASSERT(idStart < idEnd);
                    SkString id(idStart, idEnd - idStart);
                    SkSVGElement* found;
                    if (strncmp(id.c_str(), "mask", 4) != 0) {
                        bool itsFound = parser.fIDs.find(id.c_str(), &found);
                        SkASSERT(itsFound);
                        SkASSERT(found->getType() == SkSVGType_LinearGradient ||
                            found->getType() == SkSVGType_RadialGradient);
                    }
                    parser._addAttribute("shader", id.c_str());
                }
                break;
            case kStroke_Dasharray:
                break;
            case kStroke_Linecap:
                parser._addAttributeLen("strokeCap", attrValue, attrLength);
                break;
            case kStroke_Linejoin:
                parser._addAttributeLen("strokeJoin", attrValue, attrLength);
                break;
            case kStroke_Miterlimit:
                parser._addAttributeLen("strokeMiter", attrValue, attrLength);
                break;
            case kStroke_Width:
                parser._addAttributeLen("strokeWidth", attrValue, attrLength);
            case kStyle:
            case kTransform:
                break;
        default:
            SkASSERT(0);
            return false;
        }
    }
    return true;
}

bool SkSVGPaint::writeChangedElements(SkSVGParser& parser,
        SkSVGPaint& current, bool* changed) {
    SkSVGPaint& lastState = parser.fLastFlush;
    for (int index = kInitial + 1; index < kTerminal; index++) {
        SkString* topAttr = current[index];
        size_t attrLength = topAttr->size();
        if (attrLength == 0)
            continue;
        const char* attrValue = topAttr->c_str();
        SkString* lastAttr = lastState[index];
        switch(index) {
            case kClipPath:
            case kClipRule:
                // !!! need to add this outside of paint
                break;
            case kEnableBackground:
                // !!! don't know what to do with this
                break;
            case kFill:
                goto addColor;
            case kFillRule:
            case kFilter:
                break;
            case kFontFamily:
                parser._startElement("typeface");
                parser._addAttributeLen("fontName", attrValue, attrLength);
                parser._endElement();   // typeface
                break;
            case kFontSize:
            case kLetterSpacing:
                break;
            case kMask:
            case kOpacity:
                if (changed[kStroke] == false && changed[kFill] == false) {
                    parser._startElement("color");
                    SkString& opacity = current.f_opacity;
                    parser._addAttributeLen("color", parser.fLastColor.c_str(), parser.fLastColor.size());
                    parser._addAttributeLen("alpha", opacity.c_str(), opacity.size());
                    parser._endElement();   // color
                }
                break;
            case kStopColor:
                break;
            case kStopOpacity:
                break;
            case kStroke:
addColor:
                if (strncmp(lastAttr->c_str(), "url(", 4) == 0 && strncmp(attrValue, "url(", 4) != 0) {
                    parser._startElement("shader");
                    parser._endElement();
                }
                if (topAttr->equals(*lastAttr))
                    continue;
                {
                    bool urlRef = strncmp(attrValue, "url(", 4) == 0;
                    bool colorNone = strcmp(attrValue, "none") == 0;
                    bool lastEqual = parser.fLastColor.equals(attrValue, attrLength);
                    bool newColor = urlRef == false && colorNone == false && lastEqual == false;
                    if (newColor || changed[kOpacity]) {
                        parser._startElement("color");
                        if (newColor || changed[kOpacity]) {
                            parser._addAttributeLen("color", attrValue, attrLength);
                            parser.fLastColor.set(attrValue, attrLength);
                        }
                        if (changed[kOpacity]) {
                            SkString& opacity = current.f_opacity;
                            parser._addAttributeLen("alpha", opacity.c_str(), opacity.size());
                        }
                        parser._endElement();   // color
                    }
                }
                break;
            case kStroke_Dasharray:
                parser._startElement("dash");
                SkSVGParser::ConvertToArray(*topAttr);
                parser._addAttribute("intervals", topAttr->c_str());
                parser._endElement();   // dash
            break;
            case kStroke_Linecap:
            case kStroke_Linejoin:
            case kStroke_Miterlimit:
            case kStroke_Width:
            case kStyle:
            case kTransform:
                break;
        default:
            SkASSERT(0);
            return false;
        }
    }
    return true;
}

void SkSVGPaint::Push(SkSVGPaint** head, SkSVGPaint* newRecord) {
    newRecord->fNext = *head;
    *head = newRecord;
}

void SkSVGPaint::Pop(SkSVGPaint** head) {
    SkSVGPaint* next = (*head)->fNext;
    *head = next;
}
