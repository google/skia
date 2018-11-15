/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bmhParser.h"

    // replace #Method description, #Param, #Return with #Populate
    // if description, params, return are free of phrase refs
bool HackParser::hackFiles() {
    string filename(fFileName);
    size_t len = filename.length() - 1;
    while (len > 0 && (isalnum(filename[len]) || '_' == filename[len] || '.' == filename[len])) {
        --len;
    }
    filename = filename.substr(len + 1);
    if (filename.substr(0, 2) != "Sk") {
        return true;
    }
    size_t under = filename.find('_');
    SkASSERT(under);
    string className = filename.substr(0, under);
    fOut = fopen(filename.c_str(), "wb");
    if (!fOut) {
        SkDebugf("could not open output file %s\n", filename.c_str());
        return false;
    }
    auto mapEntry = fBmhParser.fClassMap.find(className);
    if (fBmhParser.fClassMap.end() == mapEntry) {
        remove(filename.c_str());
        return true;
    }
    const Definition* classMarkup = &mapEntry->second;
    const Definition* root = classMarkup->fParent;
    SkASSERT(root);
    SkASSERT(root->fTerminator);
    SkASSERT('\n' == root->fTerminator[0]);
    SkASSERT(!root->fParent);
    fStart = root->fStart;
    fChar = fStart;
    fEnd = root->fTerminator;
    this->replaceWithPop(root);
    FPRINTF("%.*s", (int) (fEnd - fChar), fChar);
    if ('\n' != fEnd[-1]) {
        FPRINTF("\n");
    }
    fclose(fOut);
    if (ParserCommon::WrittenFileDiffers(filename, root->fFileName)) {
        SkDebugf("wrote %s\n", filename.c_str());
    } else {
        remove(filename.c_str());
    }
    return true;
}

// returns true if topic has method
void HackParser::replaceWithPop(const Definition* root) {
    for (auto child : root->fChildren) {
        if (MarkType::kClass == child->fMarkType || MarkType::kStruct == child->fMarkType
                || MarkType::kSubtopic == child->fMarkType) {
            this->replaceWithPop(child);
        }
        if (MarkType::kMethod != child->fMarkType) {
            continue;
        }
        auto& grans = child->fChildren;
        if (grans.end() != std::find_if(grans.begin(), grans.end(),
                [](const Definition* def) {
                    return MarkType::kPopulate == def->fMarkType
                        || MarkType::kPhraseRef == def->fMarkType
                        || MarkType::kFormula == def->fMarkType
                        || MarkType::kAnchor == def->fMarkType
                        || MarkType::kList == def->fMarkType
                        || MarkType::kTable == def->fMarkType;
                } )) {
            continue;
        }
        // write #Populate in place of description, #Param(s), #Return (if present)
        const char* keep = child->fContentStart;
        const char* next = nullptr;
        for (auto gran : grans) {
            if (MarkType::kIn == gran->fMarkType || MarkType::kLine == gran->fMarkType) {
                keep = gran->fTerminator;
                continue;
            }
            if (MarkType::kExample == gran->fMarkType
                    || MarkType::kNoExample == gran->fMarkType) {
                next = gran->fStart;
                break;
            }
            if (MarkType::kParam == gran->fMarkType
                    || MarkType::kReturn == gran->fMarkType
                    || MarkType::kToDo == gran->fMarkType
                    || MarkType::kComment == gran->fMarkType) {
                continue;
            }
            SkDebugf("");  // convenient place to set a breakpoint
        }
        SkASSERT(next);
        FPRINTF("%.*s", (int) (keep - fChar), fChar);
        if ('\n' != keep[-1]) {
            FPRINTF("\n");
        }
        FPRINTF("#Populate\n\n");
        fChar = next;
    }
}
