/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkSVGGradient.h"
#include "SkSVGParser.h"
#include "SkSVGStop.h"

SkSVGGradient::SkSVGGradient() {
}

SkSVGElement* SkSVGGradient::getGradient() {
    return this;
}

bool SkSVGGradient::isDef() {
    return true;
}

bool SkSVGGradient::isNotDef() {
    return false;
}

void SkSVGGradient::translate(SkSVGParser& parser, bool defState) {
    INHERITED::translate(parser, defState);
    // !!! no support for 'objectBoundingBox' yet
    bool first = true;
    bool addedFirst = false;
    bool addedLast = false;
    SkString offsets("[");
    SkString* lastOffset = nullptr;
    for (SkSVGElement** ptr = fChildren.begin(); ptr < fChildren.end(); ptr++) {
        SkASSERT((*ptr)->getType() == SkSVGType_Stop);
        SkSVGStop* stop = (SkSVGStop*) *ptr;
        if (first && stop->f_offset.equals("0") == false) {
            addedFirst = true;
            offsets.append("0,");
        }
        SkString* thisOffset = &stop->f_offset;
        if (lastOffset && thisOffset->equals(*lastOffset)) {
            if (thisOffset->equals("1")) {
                offsets.remove(offsets.size() - 2, 2);
                offsets.append(".999,");
            } else {
                SkASSERT(0); // !!! need to write this case
            }
        }
        offsets.append(*thisOffset);
        if (ptr == fChildren.end() - 1) { // last
            if (stop->f_offset.equals("1") == false) {
                offsets.append(",1");
                addedLast = true;
            }
        } else
            offsets.appendUnichar(',');
        first = false;
        lastOffset = thisOffset;
    }
    offsets.appendUnichar(']');
    parser._addAttribute("offsets", offsets);
    if (addedFirst)
        parser.translate(*fChildren.begin(), defState);
    for (SkSVGElement** ptr = fChildren.begin(); ptr < fChildren.end(); ptr++)
        parser.translate(*ptr, defState);
    if (addedLast)
        parser.translate(*(fChildren.end() - 1), defState);
}

void SkSVGGradient::translateGradientUnits(SkString& units) {
    // !!! no support for 'objectBoundingBox' yet
    SkASSERT(strcmp(units.c_str(), "userSpaceOnUse") == 0);
}

void SkSVGGradient::write(SkSVGParser& parser, SkString& baseColor) {
    if (baseColor.c_str()[0] != '#')
        return;
    SkSVGPaint* saveHead = parser.fHead;
    parser.fHead = &fPaintState;
    parser.fSuppressPaint = true;
    SkString originalID(f_id);
    f_id.set("mask"); // write out gradient named given name + color (less initial #)
    f_id.append(baseColor.c_str() + 1);
    SkString originalColors;
    for (SkSVGElement** ptr = fChildren.begin(); ptr < fChildren.end(); ptr++) {
        SkSVGStop* colorElement = (SkSVGStop*) *ptr;
        SkString& color = colorElement->fPaintState.f_stopColor;
        originalColors.append(color);
        originalColors.appendUnichar(',');
        SkASSERT(color.c_str()[0] == '#');
        SkString replacement;
        replacement.set("0x");
        replacement.append(color.c_str() + 1, 2); // add stop colors using given color, turning existing stop color into alpha
        SkASSERT(baseColor.c_str()[0] == '#');
        SkASSERT(baseColor.size() == 7);
        replacement.append(baseColor.c_str() + 1);
        color.set(replacement);
    }
    translate(parser, true);
    const char* originalPtr = originalColors.c_str(); // restore original gradient values
    for (SkSVGElement** ptr = fChildren.begin(); ptr < fChildren.end(); ptr++) {
        SkSVGStop* color = (SkSVGStop*) *ptr;
        const char* originalEnd = strchr(originalPtr, ',');
        color->fPaintState.f_stopColor.set(originalPtr, originalEnd - originalPtr);
        originalPtr = originalEnd + 1;
    }
    f_id.set(originalID);
    parser.fSuppressPaint = false;
    parser.fHead = saveHead;
}
