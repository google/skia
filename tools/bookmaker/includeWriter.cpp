/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bookmaker.h"
#include <chrono>
#include <ctime>
#include <string>

bool IncludeWriter::checkChildCommentLength(const Definition* parent, MarkType childType) const {
    bool oneMember = false;
    for (auto& item : parent->fChildren) {
        if (childType != item->fMarkType) {
            continue;
        }
        oneMember = true;
        int lineLen = 0;
        for (auto& itemChild : item->fChildren) {
            if (MarkType::kExperimental == itemChild->fMarkType) {
                lineLen = sizeof("experimental") - 1;
                break;
            }
            if (MarkType::kDeprecated == itemChild->fMarkType) {
                lineLen = sizeof("deprecated") - 1;
                // todo: look for 'soon'
                break;
            }
            if (MarkType::kLine == itemChild->fMarkType) {
                lineLen = itemChild->length();
                break;
            }
        }
        if (!lineLen) {
            item->reportError<void>("missing #Line");
        }
        if (fEnumItemCommentTab + lineLen >= 100) {
// if too long, remove spaces until it fits, or wrap
//            item->reportError<void>("#Line comment too long");
        }
    }
    return oneMember;
}

void IncludeWriter::checkEnumLengths(const Definition& child, string enumName, ItemLength* length) const {
    const Definition* enumItem = this->matchMemberName(enumName, child);
    if (std::any_of(enumItem->fChildren.begin(), enumItem->fChildren.end(),
            [](Definition* child){return MarkType::kNoJustify == child->fMarkType;})) {
        return;
    }
    string comment = this->enumMemberComment(enumItem, child);
    int lineLimit = 100 - fIndent - 7; // 7: , space //!< space
    if (length->fCurValue) {
        lineLimit -= 3; // space = space
    }
    if (length->fCurName + length->fCurValue + (int) comment.length() < lineLimit) {
        length->fLongestName = SkTMax(length->fLongestName, length->fCurName);
        length->fLongestValue = SkTMax(length->fLongestValue, length->fCurValue);
    }
}

void IncludeWriter::constOut(const Definition* memberStart, const Definition* bmhConst) {
    const char* bodyEnd = fDeferComment ? fDeferComment->fContentStart - 1 :
        memberStart->fContentStart;
    this->firstBlockTrim((int) (bodyEnd - fStart), fStart);  // may write nothing
    this->lf(2);
    this->indentDeferred(IndentKind::kConstOut);
    if (fStructEnded) {
        fIndent = fICSStack.size() * 4;
        fStructEnded = false;
    }
    // comment may be legitimately empty; typedef may not have separate comment (for now)
    fReturnOnWrite = true;
    bool commentHasLength = this->descriptionOut(bmhConst, SkipFirstLine::kYes, Phrase::kNo);
    fReturnOnWrite = false;
    if (commentHasLength) {
        this->writeCommentHeader();
        fIndent += 4;
        if (!this->descriptionOut(bmhConst, SkipFirstLine::kYes, Phrase::kNo)) {
            return memberStart->reportError<void>("expected description for const");
        }
        fIndent -= 4;
        this->writeCommentTrailer(OneLine::kNo);
    }
    this->setStart(memberStart->fContentStart, memberStart);
}

bool IncludeWriter::descriptionOut(const Definition* def, SkipFirstLine skipFirstLine,
            Phrase phrase) {
    bool wroteSomething = false;
    const char* commentStart = def->fContentStart;
    if (SkipFirstLine::kYes == skipFirstLine) {
        TextParser parser(def);
        SkAssertResult(parser.skipLine());
        commentStart = parser.fChar;
    }
    int commentLen = (int) (def->fContentEnd - commentStart);
    bool breakOut = false;
    SkDEBUGCODE(bool wroteCode = false);
    if (def->fDeprecated) {
        if (fReturnOnWrite) {
            return true;
        }
        string message = def->incompleteMessage(Definition::DetailsType::kSentence);
        this->writeString(message);
        this->lfcr();
        wroteSomething = true;
    }
    const Definition* lastDescription = def;
    for (auto prop : def->fChildren) {
        fLastDescription = lastDescription;
        lastDescription = prop;
        switch (prop->fMarkType) {
            case MarkType::kCode: {
                bool literal = false;
                bool literalOutdent = false;
                commentLen = (int) (prop->fStart - commentStart);
                if (commentLen > 0) {
                    SkASSERT(commentLen < 1000);
                    if (Wrote::kNone != this->rewriteBlock(commentLen, commentStart, Phrase::kNo)) {
                        if (fReturnOnWrite) {
                            return true;
                        }
                        this->lf(2);
                        wroteSomething = true;
                    }
                }
                size_t childSize = prop->fChildren.size();
                if (childSize) {
                    if (MarkType::kLiteral == prop->fChildren[0]->fMarkType) {
                        SkASSERT(1 == childSize || 2 == childSize);  // incomplete
                        SkASSERT(1 == childSize || MarkType::kOutdent == prop->fChildren[1]->fMarkType);
                        commentStart = prop->fChildren[childSize - 1]->fContentStart;
                        literal = true;
                        literalOutdent = 2 == childSize &&
                                MarkType::kOutdent == prop->fChildren[1]->fMarkType;
                    }
                }
                commentLen = (int) (prop->fContentEnd - commentStart);
                SkASSERT(commentLen > 0);
                if (literal) {
                    if (!fReturnOnWrite && !literalOutdent) {
                        fIndent += 4;
                    }
                    wroteSomething |= this->writeBlockIndent(commentLen, commentStart);
                    if (fReturnOnWrite) {
                        return true;
                    }
                    if (!fReturnOnWrite) {
                        this->lf(2);
                        if (!literalOutdent) {
                            fIndent -= 4;
                        }
                    }
                    SkDEBUGCODE(wroteCode = true);
                }
                commentStart = prop->fTerminator;
                } break;
            case MarkType::kBug: {
                if (fReturnOnWrite) {
                    return true;
                }
                string bugstr("(see skbug.com/" + string(prop->fContentStart,
                    prop->fContentEnd - prop->fContentStart) + ')');
                this->writeString(bugstr);
                this->lfcr();
                wroteSomething = true;
            }
            case MarkType::kDeprecated:
            case MarkType::kPrivate:
                commentLen = (int) (prop->fStart - commentStart);
                if (commentLen > 0) {
                    SkASSERT(commentLen < 1000);
                    if (Wrote::kNone != this->rewriteBlock(commentLen, commentStart, Phrase::kNo)) {
                        if (fReturnOnWrite) {
                            return true;
                        }
                        this->lfcr();
                        wroteSomething = true;
                    }
                }
                commentStart = prop->fContentStart;
                if (MarkType::kPrivate != prop->fMarkType && ' ' < commentStart[0]) {
                    commentStart = strchr(commentStart, '\n');
                }
                if (MarkType::kBug == prop->fMarkType) {
                    commentStart = prop->fContentEnd;
                }
                commentLen = (int) (prop->fContentEnd - commentStart);
                if (commentLen > 0) {
                    wroteSomething |= this->writeBlockIndent(commentLen, commentStart);
                    if (wroteSomething && fReturnOnWrite) {
                        return true;
                    }
                    const char* end = commentStart + commentLen;
                    while (end > commentStart && ' ' == end[-1]) {
                        --end;
                    }
                    if (end > commentStart && '\n' == end[-1]) {
                        this->lfcr();
                    }
                }
                commentStart = prop->fTerminator;
                commentLen = (int) (def->fContentEnd - commentStart);
                break;
            case MarkType::kExperimental:
                commentStart = prop->fContentStart;
                if (' ' < commentStart[0]) {
                    commentStart = strchr(commentStart, '\n');
                }
                commentLen = (int) (prop->fContentEnd - commentStart);
                if (commentLen > 0) {
                    if (Wrote::kNone != this->rewriteBlock(commentLen, commentStart, Phrase::kNo)) {
                        if (fReturnOnWrite) {
                            return true;
                        }
                        this->lfcr();
                        wroteSomething = true;
                    }
                }
                commentStart = prop->fTerminator;
                commentLen = (int) (def->fContentEnd - commentStart);
                break;
            case MarkType::kFormula: {
                commentLen = prop->fStart - commentStart;
                if (commentLen > 0) {
                    if (Wrote::kNone != this->rewriteBlock(commentLen, commentStart, Phrase::kNo)) {
                        if (fReturnOnWrite) {
                            return true;
                        }
                        if (commentLen > 1 && '\n' == prop->fStart[-1] &&
                                '\n' == prop->fStart[-2]) {
                            this->lf(1);
                        } else {
                            this->writeSpace();
                        }
                        wroteSomething = true;
                    }
                }
                int saveIndent = fIndent;
                if (fIndent < fColumn + 1) {
                    fIndent = fColumn + 1;
                }
                wroteSomething |= this->writeBlockIndent(prop->length(), prop->fContentStart);
                fIndent = saveIndent;
                if (wroteSomething && fReturnOnWrite) {
                    return true;
                }
                commentStart = prop->fTerminator;
                commentLen = (int) (def->fContentEnd - commentStart);
                if (!fReturnOnWrite) {
                    if (commentLen > 1 && '\n' == commentStart[0] && '\n' == commentStart[1]) {
                        this->lf(2);
                    } else {
                        SkASSERT('\n' == prop->fTerminator[0]);
                        if ('.' != prop->fTerminator[1] && !fLinefeeds) {
                            this->writeSpace();
                        }
                    }
                }
                } break;
            case MarkType::kDetails:
            case MarkType::kIn:
            case MarkType::kLine:
            case MarkType::kToDo:
                commentLen = (int) (prop->fStart - commentStart);
                if (commentLen > 0) {
                    SkASSERT(commentLen < 1000);
                    if (Wrote::kNone != this->rewriteBlock(commentLen, commentStart, Phrase::kNo)) {
                        if (fReturnOnWrite) {
                            return true;
                        }
                        this->lfcr();
                        wroteSomething = true;
                    }
                }
                commentStart = prop->fTerminator;
                commentLen = (int) (def->fContentEnd - commentStart);
                break;
            case MarkType::kList:
                commentLen = prop->fStart - commentStart;
                if (commentLen > 0) {
                    if (Wrote::kNone != this->rewriteBlock(commentLen, commentStart,
                            Phrase::kNo)) {
                        if (fReturnOnWrite) {
                            return true;
                        }
                        this->lfcr();
                        wroteSomething = true;
                    }
                }
                for (auto row : prop->fChildren) {
                    SkASSERT(MarkType::kRow == row->fMarkType);
                    for (auto column : row->fChildren) {
                        SkASSERT(MarkType::kColumn == column->fMarkType);
                        if (fReturnOnWrite) {
                            return true;
                        }
                        this->writeString("-");
                        this->writeSpace();
                        wroteSomething |= this->descriptionOut(column, SkipFirstLine::kNo, Phrase::kNo);
                        this->lf(1);
                    }
                }
                commentStart = prop->fTerminator;
                commentLen = (int) (def->fContentEnd - commentStart);
                if ('\n' == commentStart[0] && '\n' == commentStart[1]) {
                    this->lf(2);
                }
                break;
            case MarkType::kPhraseRef: {
                commentLen = prop->fStart - commentStart;
                if (commentLen > 0) {
                    if (fReturnOnWrite) {
                        return true;
                    }
                    this->rewriteBlock(commentLen, commentStart, Phrase::kNo);
                    // ince we don't do line wrapping, always insert LF before phrase
                    this->lfcr();   // TODO: remove this once rewriteBlock rewraps paragraphs
                    wroteSomething = true;
                }
                auto iter = fBmhParser->fPhraseMap.find(prop->fName);
                if (fBmhParser->fPhraseMap.end() == iter) {
                    return this->reportError<bool>("missing phrase definition");
                }
                Definition* phraseDef = iter->second;
                // TODO: given TextParser(commentStart, prop->fStart + up to #) return if
                // it ends with two of more linefeeds, ignoring other whitespace
                Phrase defIsPhrase = '\n' == prop->fStart[0] && '\n' == prop->fStart[-1] ?
                        Phrase::kNo : Phrase::kYes;
                if (Phrase::kNo == defIsPhrase) {
                    this->lf(2);
                }
                const char* start = phraseDef->fContentStart;
                int length = phraseDef->length();
                auto propParams = prop->fChildren.begin();
                // can this share code or logic with mdout somehow?
                for (auto child : phraseDef->fChildren) {
                    if (MarkType::kPhraseParam == child->fMarkType) {
                        continue;
                    }
                    int localLength = child->fStart - start;
                    if (fReturnOnWrite) {
                        return true;
                    }
                    this->rewriteBlock(localLength, start, defIsPhrase);
                    start += localLength;
                    length -= localLength;
                    SkASSERT(propParams != prop->fChildren.end());
                    if (fColumn > 0) {
                        this->writeSpace();
                    }
                    this->writeString((*propParams)->fName);
                    localLength = child->fContentEnd - child->fStart;
                    start += localLength;
                    length -= localLength;
                    if (isspace(start[0])) {
                        this->writeSpace();
                    }
                    defIsPhrase = Phrase::kYes;
                    wroteSomething = true;
                }
                if (length > 0) {
                    if (fReturnOnWrite) {
                        return true;
                    }
                    this->rewriteBlock(length, start, defIsPhrase);
                }
                commentStart = prop->fContentStart;
                commentLen = (int) (def->fContentEnd - commentStart);
                if (!fReturnOnWrite) {
                    if ('\n' == commentStart[0] && '\n' == commentStart[1]) {
                        this->lf(2);
                    }
                }
                } break;
            default:
                commentLen = (int) (prop->fStart - commentStart);
                breakOut = true;
        }
        if (breakOut) {
            break;
        }
    }
    if (!breakOut) {
        commentLen = (int) (def->fContentEnd - commentStart);
    }
    SkASSERT(wroteCode || (commentLen > 0 && commentLen < 1500) || def->fDeprecated);
    if (commentLen > 0) {
        if (Wrote::kNone != this->rewriteBlock(commentLen, commentStart, phrase)) {
            if (fReturnOnWrite) {
                return true;
            }
            wroteSomething = true;
        }
    }
    SkASSERT(!fReturnOnWrite || !wroteSomething);
    return wroteSomething;
}

void IncludeWriter::enumHeaderOut(RootDefinition* root, const Definition& child) {
    const Definition* enumDef = nullptr;
    const char* bodyEnd = fDeferComment ? fDeferComment->fContentStart - 1 :
            child.fContentStart;
    this->firstBlockTrim((int) (bodyEnd - fStart), fStart);  // may write nothing
    this->lf(2);
    this->indentDeferred(IndentKind::kEnumHeader);
    fDeferComment = nullptr;
    this->setStart(child.fContentStart, &child);
    const auto& nameDef = child.fTokens.front();
    string fullName;
    if (nullptr != nameDef.fContentEnd) {
        TextParser enumClassCheck(&nameDef);
        const char* start = enumClassCheck.fStart;
        size_t len = (size_t) (enumClassCheck.fEnd - start);
        bool enumClass = enumClassCheck.skipExact("class ");
        if (enumClass) {
            start = enumClassCheck.fChar;
            const char* end = enumClassCheck.anyOf(" \n;{");
            len = (size_t) (end - start);
        }
        string enumName(start, len);
        if (enumClass) {
            child.fChildren[0]->fName = enumName;
        }
        fullName = root->fName + "::" + enumName;
        enumDef = root->find(enumName, RootDefinition::AllowParens::kNo);
        if (!enumDef) {
            enumDef = root->find(fullName, RootDefinition::AllowParens::kNo);
        }
        if (!enumDef) {
            auto mapEntry = fBmhParser->fEnumMap.find(enumName);
            if (fBmhParser->fEnumMap.end() != mapEntry) {
                enumDef = &mapEntry->second;
            }
        }
        if (!enumDef && enumName == root->fName) {
            enumDef = root;
        }
        SkASSERT(enumDef);
        // child[0] should be #Code comment starts at child[0].fTerminator
            // though skip until #Code is found (in case there's a #ToDo, etc)
        // child[1] should be #Const comment ends at child[1].fStart
        // comment becomes enum header (if any)
    } else {
        string enumName(root->fName);
        enumName += "::_anonymous";
        if (fAnonymousEnumCount > 1) {
            enumName += '_' + to_string(fAnonymousEnumCount);
        }
        enumDef = root->find(enumName, RootDefinition::AllowParens::kNo);
        SkASSERT(enumDef);
        ++fAnonymousEnumCount;
    }
    Definition* codeBlock = nullptr;
    const char* commentStart = nullptr;
    bool wroteHeader = false;
    bool lastAnchor = false;
    SkDEBUGCODE(bool foundConst = false);
    for (auto test : enumDef->fChildren) {
        if (MarkType::kCode == test->fMarkType && !codeBlock) {
            codeBlock = test;
            commentStart = codeBlock->fTerminator;
            continue;
        }
        if (!codeBlock) {
            continue;
        }
        const char* commentEnd = test->fStart;
        if (!wroteHeader &&
                !this->contentFree((int) (commentEnd - commentStart), commentStart)) {
            if (fIndentNext) {
                // FIXME: how can I tell where fIdentNext gets cleared?
                this->indentIn(IndentKind::kEnumChild);
            }
            this->writeCommentHeader();
            this->writeString("\\enum");
            if (fullName.length() > 0) {
                this->writeSpace();
                this->writeString(fullName.c_str());
            }
            this->indentIn(IndentKind::kEnumChild2);
            this->lfcr();
            wroteHeader = true;
        }
        if (lastAnchor) {
            if (commentEnd - commentStart > 1) {
                SkASSERT('\n' == commentStart[0]);
                if (' ' == commentStart[1]) {
                    this->writeSpace();
                }
            }
            lastAnchor = false;
        }
        this->rewriteBlock((int) (commentEnd - commentStart), commentStart, Phrase::kNo);
        if (MarkType::kAnchor == test->fMarkType || MarkType::kCode == test->fMarkType) {
            bool newLine = commentEnd - commentStart > 1 &&
                '\n' == commentEnd[-1] && '\n' == commentEnd[-2];
            commentStart = test->fContentStart;
            commentEnd = MarkType::kAnchor == test->fMarkType ? test->fChildren[0]->fStart :
                    test->fContentEnd;
            if (newLine) {
                this->lf(2);
            } else {
                this->writeSpace();
            }
            if (MarkType::kAnchor == test->fMarkType) {
                this->rewriteBlock((int) (commentEnd - commentStart), commentStart, Phrase::kNo);
            } else {
                this->writeBlock((int) (commentEnd - commentStart), commentStart);
                this->lf(2);
            }
            lastAnchor = true;   // this->writeSpace();
        }
        commentStart = test->fTerminator;
        if (MarkType::kConst == test->fMarkType) {
            SkASSERT(codeBlock);  // FIXME: check enum for correct order earlier
            SkDEBUGCODE(foundConst = true);
            break;
        }
    }
    SkASSERT(codeBlock);
    SkASSERT(foundConst);
    if (wroteHeader) {
        this->indentOut();
        this->lfcr();
        this->writeCommentTrailer(OneLine::kNo);
    }
    Definition* braceHolder = child.fChildren[0];
    if (KeyWord::kClass == braceHolder->fKeyWord) {
        braceHolder = braceHolder->fChildren[0];
    }
    bodyEnd = braceHolder->fContentStart;
    SkASSERT('{' == bodyEnd[0]);
    ++bodyEnd;
    this->lfcr();
    this->writeBlock((int) (bodyEnd - fStart), fStart); // write include "enum Name {"
    this->indentIn(IndentKind::kEnumHeader2);
    this->singleLF();
    this->setStart(bodyEnd, braceHolder);
    fEnumDef = enumDef;
}

const Definition* IncludeWriter::enumMemberForComment(const Definition* currentEnumItem) const {
    for (auto constItem : currentEnumItem->fChildren) {
        if (MarkType::kLine == constItem->fMarkType
                || MarkType::kExperimental == constItem->fMarkType
                || MarkType::kDeprecated == constItem->fMarkType) {
            return constItem;
        }
    }
    SkASSERT(0);
    return nullptr;
}

string IncludeWriter::enumMemberComment(const Definition* currentEnumItem,
        const Definition& child) const {
    // #Const should always be followed by #Line, so description follows that
    string shortComment;
    for (auto constItem : currentEnumItem->fChildren) {
        if (MarkType::kLine == constItem->fMarkType) {
            shortComment = string(constItem->fContentStart, constItem->length());
            break;
        }
        if (IncompleteAllowed(constItem->fMarkType)) {
            shortComment = constItem->fParent->incompleteMessage(Definition::DetailsType::kPhrase);
        }
    }
    if (!shortComment.length()) {
        currentEnumItem->reportError<void>("missing #Line or #Deprecated or #Experimental");
    }
    return shortComment;
}

IncludeWriter::ItemState IncludeWriter::enumMemberName(
        const Definition& child, const Definition* token, Item* item, LastItem* last,
        const Definition** currentEnumItem) {
    TextParser parser(fFileName, last->fStart, last->fEnd, fLineCount);
    parser.skipWhiteSpace();
    item->fName = string(parser.fChar, (int) (last->fEnd - parser.fChar));
    *currentEnumItem = this->matchMemberName(item->fName, child);
    if (token) {
        this->setStart(token->fContentEnd, token);
        TextParser enumLine(token->fFileName, last->fEnd, token->fContentStart, token->fLineCount);
        const char* end = enumLine.anyOf(",}=");
        SkASSERT(end);
        if ('=' == *end) {  // write enum value
            last->fEnd = token->fContentEnd;
            item->fValue = string(token->fContentStart, (int) (last->fEnd - token->fContentStart));
            return ItemState::kValue;
        }
    }
    return ItemState::kComment;
}

void IncludeWriter::enumMemberOut(const Definition* currentEnumItem, const Definition& child,
        const Item& item, Preprocessor& preprocessor) {
    SkASSERT(currentEnumItem);
    string shortComment = this->enumMemberComment(currentEnumItem, child);
    int enumItemValueTab =
            SkTMax((int) item.fName.length() + fIndent + 1, fEnumItemValueTab); // 1: ,
    int valueLength = item.fValue.length();
    int assignLength = valueLength ? valueLength + 3 : 0; // 3: space = space
    int enumItemCommentTab = SkTMax(enumItemValueTab + assignLength, fEnumItemCommentTab);
    int trimNeeded = enumItemCommentTab + shortComment.length() - (100 - (sizeof("//!< ") - 1));
    bool crAfterName = false;
    if (trimNeeded > 0) {
        if (item.fValue.length()) {
            int valueSpare = SkTMin(trimNeeded,                  // 3 below: space = space
                    (int) (enumItemCommentTab - enumItemValueTab - item.fValue.length() - 3));
            SkASSERT(valueSpare >= 0);
            trimNeeded -= valueSpare;
            enumItemCommentTab -= valueSpare;
        }
        if (trimNeeded > 0) {
            int nameSpare = SkTMin(trimNeeded, (int) (enumItemValueTab - item.fName.length()
                    - fIndent - 1));  // 1: ,
            SkASSERT(nameSpare >= 0);
            trimNeeded -= nameSpare;
            enumItemValueTab -= nameSpare;
            enumItemCommentTab -= nameSpare;
        }
        if (trimNeeded > 0) {
            crAfterName = true;
            if (!valueLength) {
                this->enumMemberForComment(currentEnumItem)->reportError<void>("comment too long");
            } else if (valueLength + fIndent + 8 + shortComment.length() > // 8: addtional indent
                    100 - (sizeof(", //!< ") - 1)) { // -1: zero-terminated string
                this->enumMemberForComment(currentEnumItem)->reportError<void>("comment 2 long");
            }                                    // 2: = space
            enumItemValueTab = fEnumItemValueTab +  2                 // 2: , space
                    - SkTMax(0, fEnumItemValueTab + 2 + valueLength +    2 - fEnumItemCommentTab);
            enumItemCommentTab = SkTMax(enumItemValueTab + valueLength + 2, fEnumItemCommentTab);
        }
    }
    this->lfcr();
    this->writeString(item.fName);
    int saveIndent = fIndent;
    if (item.fValue.length()) {
        if (!crAfterName) {
            this->indentToColumn(enumItemValueTab);
        } else {
            this->writeSpace();
        }
        this->writeString("=");
        if (crAfterName) {
            this->lfcr();
            fIndent = enumItemValueTab;
        } else {
            this->writeSpace();
        }
        this->writeString(item.fValue);
    }
    this->writeString(",");
    this->indentToColumn(enumItemCommentTab);
    this->writeString("//!<");
    this->writeSpace();
    this->rewriteBlock(shortComment.length(), shortComment.c_str(), Phrase::kYes);
    this->lfcr();
    fIndent = saveIndent;
    if (preprocessor.fStart) {
        SkASSERT(preprocessor.fEnd);
        int saveIndent = fIndent;
        fIndent = SkTMax(0, fIndent - 8);
        this->lf(2);
        this->writeBlock(
                (int) (preprocessor.fEnd - preprocessor.fStart), preprocessor.fStart);
        this->lfcr();
        fIndent = saveIndent;
        preprocessor.reset();
    }
}

// iterate through include tokens and find how much remains for 1 line comments
// put ones that fit on same line, ones that are too big wrap
void IncludeWriter::enumMembersOut(Definition& child) {
    ItemState state = ItemState::kNone;
    const Definition* currentEnumItem;
    LastItem last = { nullptr, nullptr };
    auto brace = child.fChildren[0];
    if (KeyWord::kClass == brace->fKeyWord) {
        brace = brace->fChildren[0];
    }
    SkASSERT(Bracket::kBrace == brace->fBracket);
    vector<IterState> iterStack;
    iterStack.emplace_back(brace->fTokens.begin(), brace->fTokens.end());
    IterState* iterState = &iterStack[0];
    Preprocessor preprocessor;
    Item item;
    while (iterState->fDefIter != iterState->fDefEnd) {
        auto& token = *iterState->fDefIter++;
        if (this->enumPreprocessor(&token, MemberPass::kOut, iterStack, &iterState,
                &preprocessor)) {
            continue;
        }
        if (ItemState::kName == state) {
            state = this->enumMemberName(child, &token, &item, &last, &currentEnumItem);
        }
        if (ItemState::kValue == state) {
            TextParser valueEnd(token.fFileName, last.fEnd, token.fContentStart, token.fLineCount);
            const char* end = valueEnd.anyOf(",}");
            if (!end) {  // write expression continuation
                item.fValue += string(last.fEnd, (int) (token.fContentEnd - last.fEnd));
                continue;
            }
        }
        if (ItemState::kNone != state) {
            this->enumMemberOut(currentEnumItem, child, item, preprocessor);
            item.reset();
            this->setStartBack(token.fContentStart, &token);
            state = ItemState::kNone;
            last.fStart = nullptr;
        }
        SkASSERT(ItemState::kNone == state);
        if (!last.fStart) {
            last.fStart = fStart;
        }
        last.fEnd = token.fContentEnd;
        state = ItemState::kName;
    }
    if (ItemState::kName == state) {
        state = this->enumMemberName(child, nullptr, &item, &last, &currentEnumItem);
    }
    if (ItemState::kValue == state || ItemState::kComment == state) {
        this->enumMemberOut(currentEnumItem, child, item, preprocessor);
    }
    this->indentOut();
}

bool IncludeWriter::enumPreprocessor(Definition* token, MemberPass pass,
        vector<IterState>& iterStack, IterState** iterState, Preprocessor* preprocessor) {
    if (token && Definition::Type::kBracket == token->fType) {
        if (Bracket::kSlashSlash == token->fBracket) {
            if (MemberPass::kOut == pass) {
                this->setStart(token->fContentEnd, token);
            }
            return true;  // ignore old inline comments
        }
        if (Bracket::kSlashStar == token->fBracket) {
            if (MemberPass::kOut == pass) {
                this->setStart(token->fContentEnd + 1, token);
            }
            return true;  // ignore old inline comments
        }
        if (Bracket::kPound == token->fBracket) {  // preprocessor wraps member
            preprocessor->fDefinition = token;
            preprocessor->fStart = token->fContentStart;
            if (KeyWord::kIf == token->fKeyWord || KeyWord::kIfdef == token->fKeyWord) {
                iterStack.emplace_back(token->fTokens.begin(), token->fTokens.end());
                *iterState = &iterStack.back();
                preprocessor->fWord = true;
            } else if (KeyWord::kEndif == token->fKeyWord || KeyWord::kElif == token->fKeyWord
                    || KeyWord::kElse == token->fKeyWord) {
                iterStack.pop_back();
                *iterState = &iterStack.back();
                preprocessor->fEnd = token->fContentEnd;
                if (KeyWord::kElif == token->fKeyWord) {
                    iterStack.emplace_back(token->fTokens.begin(), token->fTokens.end());
                    *iterState = &iterStack.back();
                    preprocessor->fWord = true;
                }
            } else {
                SkASSERT(0); // incomplete
            }
            return true;
        }
        if (preprocessor->fDefinition) {
            if (Bracket::kParen == token->fBracket) {
                preprocessor->fEnd = token->fContentEnd;
                SkASSERT(')' == *preprocessor->fEnd);
                ++preprocessor->fEnd;
                return true;
            }
            SkASSERT(0);  // incomplete
        }
        return true;
    }
    if (token && Definition::Type::kWord != token->fType) {
        SkASSERT(0); // incomplete
    }
    if (preprocessor->fWord) {
        preprocessor->fWord = false;
        preprocessor->fEnd = token->fContentEnd;
        return true;
    }
    return false;
}

void IncludeWriter::enumSizeItems(const Definition& child) {
    ItemState state = ItemState::kNone;
    ItemLength lengths = { 0, 0, 0, 0 };
    const char* lastEnd = nullptr;
    auto brace = child.fChildren[0];
    if (KeyWord::kClass == brace->fKeyWord) {
        brace = brace->fChildren[0];
    }
    SkASSERT(Bracket::kBrace == brace->fBracket);
    vector<IterState> iterStack;
    iterStack.emplace_back(brace->fTokens.begin(), brace->fTokens.end());
    IterState* iterState = &iterStack[0];
    Preprocessor preprocessor;
    string enumName;
    while (iterState->fDefIter != iterState->fDefEnd) {
        auto& token = *iterState->fDefIter++;
        if (this->enumPreprocessor(&token, MemberPass::kCount, iterStack, &iterState,
                &preprocessor)) {
            continue;
        }
        if (ItemState::kName == state) {
            TextParser enumLine(token.fFileName, lastEnd, token.fContentStart, token.fLineCount);
            const char* end = enumLine.anyOf(",}=");
            SkASSERT(end);
            state = '=' == *end ? ItemState::kValue : ItemState::kComment;
            if (ItemState::kValue == state) {
                lastEnd = token.fContentEnd;
                lengths.fCurValue = (int) (lastEnd - token.fContentStart);
                continue;
            }
        }
        if (ItemState::kValue == state) {
            TextParser valueEnd(token.fFileName, lastEnd, token.fContentStart, token.fLineCount);
            const char* end = valueEnd.anyOf(",}");
            if (!end) {  // write expression continuation
                lengths.fCurValue += (int) (token.fContentEnd - lastEnd);
                continue;
            }
        }
        if (ItemState::kNone != state) {
            this->checkEnumLengths(child, enumName, &lengths);
            lengths.fCurValue = 0;
            state = ItemState::kNone;
        }
        SkASSERT(ItemState::kNone == state);
        lastEnd = token.fContentEnd;
        lengths.fCurName = (int) (lastEnd - token.fContentStart);
        enumName = string(token.fContentStart, lengths.fCurName);
        state = ItemState::kName;
    }
    if (ItemState::kNone != state) {
        this->checkEnumLengths(child, enumName, &lengths);
    }
    fEnumItemValueTab = lengths.fLongestName + fIndent + 1 /* 1: , */ ;
    if (lengths.fLongestValue) {
        lengths.fLongestValue += 3; // 3: space = space
    }
    fEnumItemCommentTab = fEnumItemValueTab + lengths.fLongestValue + 1 ; // 1: space before //!<
    // iterate through bmh children and see which comments fit on include lines
    if (!this->checkChildCommentLength(fEnumDef, MarkType::kConst)) {
        fEnumDef->reportError<void>("expected at least one #Const in #Enum");
    }
}

const Definition* IncludeWriter::matchMemberName(string matchName, const Definition& child) const {
    const Definition* parent = &child;
    if (KeyWord::kEnum == child.fKeyWord && child.fChildren.size() > 0
            && KeyWord::kClass == child.fChildren[0]->fKeyWord) {
        matchName = child.fChildren[0]->fName + "::" + matchName;
    }
    do {
        if (KeyWord::kStruct == parent->fKeyWord || KeyWord::kClass == parent->fKeyWord) {
            matchName = parent->fName + "::" + matchName;
        }
    } while ((parent = parent->fParent));
    const Definition* enumItem = nullptr;
    for (auto testItem : fEnumDef->fChildren) {
        if (MarkType::kConst != testItem->fMarkType) {
            continue;
        }
        if (matchName != testItem->fName) {
            continue;
        }
        enumItem = testItem;
        break;
    }
    SkASSERT(enumItem);
    return enumItem;
}

// walk children and output complete method doxygen description
void IncludeWriter::methodOut(Definition* method, const Definition& child) {
    if (fPendingMethod) {
        this->indentOut();
        fPendingMethod = false;
    }
    fBmhMethod = method;
    fMethodDef = &child;
    fContinuation = nullptr;
    fDeferComment = nullptr;
    Definition* csParent = method->csParent();
    if (csParent && (0 == fIndent || fIndentNext)) {
        this->indentIn(IndentKind::kMethodOut);
        fIndentNext = false;
    }
    this->writeCommentHeader();
    fIndent += 4;
    this->descriptionOut(method, SkipFirstLine::kNo, Phrase::kNo);
    // compute indention column
    size_t column = 0;
    bool hasParmReturn = false;
    for (auto methodPart : method->fChildren) {
        if (MarkType::kParam == methodPart->fMarkType) {
            column = SkTMax(column, methodPart->fName.length());
            hasParmReturn = true;
        } else if (MarkType::kReturn == methodPart->fMarkType) {
            hasParmReturn = true;
        }
    }
    if (hasParmReturn) {
        this->lf(2);
        column += fIndent + sizeof("@return ");
        int saveIndent = fIndent;
        for (auto methodPart : method->fChildren) {
            if (MarkType::kParam == methodPart->fMarkType) {
                this->writeString("@param");
                this->writeSpace();
                this->writeString(methodPart->fName.c_str());
            } else if (MarkType::kReturn == methodPart->fMarkType) {
                this->writeString("@return");
            } else {
                continue;
            }
            this->indentToColumn(column);
            fIndent = column;
            this->descriptionOut(methodPart, SkipFirstLine::kNo, Phrase::kYes);
            fIndent = saveIndent;
            this->lfcr();
        }
    } else {
        this->lfcr();
    }
    fIndent -= 4;
    this->lfcr();
    this->writeCommentTrailer(OneLine::kNo);
    fBmhMethod = nullptr;
    fMethodDef = nullptr;
    fEnumDef = nullptr;
    fWroteMethod = true;
}

void IncludeWriter::structOut(const Definition* root, const Definition& child,
        const char* commentStart, const char* commentEnd) {
    this->writeCommentHeader();
    this->writeString("\\");
    SkASSERT(MarkType::kClass == child.fMarkType || MarkType::kStruct == child.fMarkType);
    this->writeString(MarkType::kClass == child.fMarkType ? "class" : "struct");
    this->writeSpace();
    this->writeString(child.fName.c_str());
    fIndent += 4;
    this->lfcr();
    if (child.fDeprecated) {
        this->writeString(child.incompleteMessage(Definition::DetailsType::kSentence));
    } else {
        this->rewriteBlock((int)(commentEnd - commentStart), commentStart, Phrase::kNo);
    }
    fIndent -= 4;
    this->lfcr();
    this->writeCommentTrailer(OneLine::kNo);
}

bool IncludeWriter::findEnumSubtopic(string undername, const Definition** rootDefPtr) const {
    const Definition* subtopic = fEnumDef->fParent;
    string subcheck = subtopic->fFiddle + '_' + undername;
    auto iter = fBmhParser->fTopicMap.find(subcheck);
    if (iter == fBmhParser->fTopicMap.end()) {
        return false;
    }
    *rootDefPtr = iter->second;
    return true;
}

Definition* IncludeWriter::findMemberCommentBlock(const vector<Definition*>& bmhChildren,
        string name) const {
    for (auto memberDef : bmhChildren) {
        if (MarkType::kMember != memberDef->fMarkType) {
            continue;
        }
        string match = memberDef->fName;
        // if match.endsWith(name) ...
        if (match.length() >= name.length() &&
                0 == match.compare(match.length() - name.length(), name.length(), name)) {
            return memberDef;
        }
    }
    for (auto memberDef : bmhChildren) {
        if (MarkType::kSubtopic != memberDef->fMarkType && MarkType::kTopic != memberDef->fMarkType) {
            continue;
        }
        Definition* result = this->findMemberCommentBlock(memberDef->fChildren, name);
        if (result) {
            return result;
        }
    }
    return nullptr;
}

Definition* IncludeWriter::findMethod(string name, RootDefinition* root) const {
    if (root) {
        return root->find(name, RootDefinition::AllowParens::kNo);
    }
    auto methodIter = fBmhParser->fMethodMap.find(name);
    if (fBmhParser->fMethodMap.end() == methodIter) {
        return nullptr;
    }
    return &methodIter->second;
}


void IncludeWriter::firstBlock(int size, const char* data) {
     SkAssertResult(this->firstBlockTrim(size, data));
}

bool IncludeWriter::firstBlockTrim(int size, const char* data) {
    bool result = this->writeBlockTrim(size, data);
    if (fFirstWrite) {
        auto fileInfo = std::find_if(fRootTopic->fChildren.begin(), fRootTopic->fChildren.end(),
                [](const Definition* def){ return MarkType::kFile == def->fMarkType; } );
        if (fRootTopic->fChildren.end() != fileInfo) {
            this->writeCommentHeader();
            this->writeString("\\file");
            this->writeSpace();
            size_t lastSlash = fFileName.rfind('/');
            if (string::npos == lastSlash) {
                lastSlash = fFileName.rfind('\\');
            }
            string fileName = fFileName.substr(lastSlash + 1);
            this->writeString(fileName);
            this->lf(2);
            fIndent += 4;
            this->descriptionOut(*fileInfo, SkipFirstLine::kNo, Phrase::kNo);
            fIndent -= 4;
            this->writeCommentTrailer(OneLine::kNo);
        }
        fFirstWrite = false;
    }
    return result;
}

void IncludeWriter::setStart(const char* start, const Definition* def) {
    SkASSERT(start >= fStart);
    this->setStartBack(start, def);
}

void IncludeWriter::setStartBack(const char* start, const Definition* def) {
    fStartSetter = def;
    fStart = start;
}

Definition* IncludeWriter::structMemberOut(const Definition* memberStart, const Definition& child) {
    const char* blockStart = !fWroteMethod && fDeferComment ? fDeferComment->fContentEnd : fStart;
    const char* blockEnd = fWroteMethod && fDeferComment ? fDeferComment->fStart - 1 :
            memberStart->fStart;
    this->firstBlockTrim((int) (blockEnd - blockStart), blockStart);
    this->indentDeferred(IndentKind::kStructMember);
    fWroteMethod = false;
    string name(child.fContentStart, (int) (child.fContentEnd - child.fContentStart));
    Definition* commentBlock = this->findMemberCommentBlock(fBmhStructDef->fChildren, name);
    if (!commentBlock) {
        return memberStart->reportError<Definition*>("member missing comment block 2");
    }
    auto lineIter = std::find_if(commentBlock->fChildren.begin(), commentBlock->fChildren.end(),
        [](const Definition* def){ return MarkType::kLine == def->fMarkType; } );
    SkASSERT(commentBlock->fChildren.end() != lineIter);
    const Definition* lineDef = *lineIter;
    if (fStructMemberLength > 100) {
        this->writeCommentHeader();
        this->writeSpace();
        this->rewriteBlock(lineDef->length(), lineDef->fContentStart, Phrase::kYes);
        this->writeCommentTrailer(OneLine::kYes);
    }
    this->lfcr();
    this->writeBlock((int) (child.fStart - memberStart->fContentStart),
            memberStart->fContentStart);
    this->indentToColumn(fStructMemberTab);
    this->writeString(name.c_str());
    auto tokenIter = child.fParent->fTokens.begin();
    std::advance(tokenIter, child.fParentIndex + 1);
    Definition* valueStart = &*tokenIter;
    while (Definition::Type::kPunctuation != tokenIter->fType) {
        std::advance(tokenIter, 1);
        SkASSERT(child.fParent->fTokens.end() != tokenIter);
    }
    Definition* valueEnd = &*tokenIter;
    if (valueStart != valueEnd) {
        this->indentToColumn(fStructValueTab);
        this->writeString("=");
        this->writeSpace();
        this->writeBlock((int) (valueEnd->fStart - valueStart->fContentStart),
                valueStart->fContentStart);
    }
    this->writeString(";");
    if (fStructMemberLength <= 100) {
        this->indentToColumn(fStructCommentTab);
        this->writeString("//!<");
        this->writeSpace();
        this->rewriteBlock(lineDef->length(), lineDef->fContentStart, Phrase::kYes);
    }
    this->lf(1);
    return valueEnd;
}

// const and constexpr and #define aren't contained in a braces like struct and enum.
// use a bmh subtopic to group like ones together, then measure them in the include as if
// they were formally linked together
void IncludeWriter::constSizeMembers(const RootDefinition* root) {
    // fBmhConst->fParent is subtopic containing all grouped const expressions
    // fConstDef is token of const include name, hopefully on same line as const start
    string rootPrefix = root ? root->fName + "::" : "";
    const Definition* test = fConstDef;
    int tokenIndex = test->fParentIndex;
    int longestName = 0;
    int longestValue = 0;
    int longestComment = 0;
    const Definition* subtopic = fBmhConst->fParent;
    SkASSERT(subtopic);
    SkASSERT(MarkType::kSubtopic == subtopic->fMarkType);
    // back up to first token on line
    size_t lineCount = test->fLineCount;
    const Definition* last;
    auto tokenIter = test->fParent->fTokens.begin();
    std::advance(tokenIter, tokenIndex);
    do {
        last = test;
        std::advance(tokenIter, -1);
        test = &*tokenIter;
        SkASSERT(test->fParentIndex == --tokenIndex);
    } while (lineCount == test->fLineCount);
    test = last;
    for (auto child : subtopic->fChildren) {
        if (MarkType::kConst != child->fMarkType) {
            continue;
        }
        // expect found name to be on the left of assign
        // expect assign
        // expect semicolon
        // no parens, no braces
        while (rootPrefix + test->fName != child->fName) {
            std::advance(tokenIter, 1);
            test = &*tokenIter;
            SkASSERT(lineCount >= test->fLineCount);
        }
        ++lineCount;
        TextParser constText(test);
        const char* nameEnd = constText.trimmedBracketEnd('=');
        SkAssertResult(constText.skipToEndBracket('='));
        const char* valueEnd = constText.trimmedBracketEnd(';');
        auto lineIter = std::find_if(child->fChildren.begin(), child->fChildren.end(),
                [](const Definition* def){ return MarkType::kLine == def->fMarkType; });
        SkASSERT(child->fChildren.end() != lineIter);
        longestName = SkTMax(longestName, (int) (nameEnd - constText.fStart));
        longestValue = SkTMax(longestValue, (int) (valueEnd - constText.fChar));
        longestComment = SkTMax(longestComment, (*lineIter)->length());
    }
    // write fStructValueTab, fStructCommentTab
    fConstValueTab = longestName + fIndent + 1;
    fConstCommentTab = fConstValueTab + longestValue + 2;
    fConstLength = fConstCommentTab + longestComment + (int) sizeof("//!<");
}

bool IncludeWriter::defineOut(const Definition& def) {
    if (def.fTokens.size() < 1) {
        return false;
    }
    auto& child = def.fTokens.front();
    string name(child.fContentStart, child.length());
    auto defIter = fBmhParser->fDefineMap.find(name);
    if (fBmhParser->fDefineMap.end() == defIter) {
        return false;
    }
    const Definition& bmhDef = defIter->second;
    this->constOut(&def, &bmhDef);
    return true;
}

void IncludeWriter::structSizeMembers(const Definition& child) {
    int longestType = 0;
    Definition* typeStart = nullptr;
    int longestName = 0;
    int longestValue = 0;
    int longestComment = 0;
    SkASSERT(child.fChildren.size() == 1 || child.fChildren.size() == 2);
    bool inEnum = false;
    bool inMethod = false;
    bool inMember = false;
    auto brace = child.fChildren[0];
    SkASSERT(Bracket::kBrace == brace->fBracket);
    for (auto& token : brace->fTokens) {
        if (Definition::Type::kBracket == token.fType) {
            if (Bracket::kSlashSlash == token.fBracket) {
                continue;  // ignore old inline comments
            }
            if (Bracket::kSlashStar == token.fBracket) {
                continue;  // ignore old inline comments
            }
            if (Bracket::kParen == token.fBracket) {
                if (inMethod) {
                    continue;
                }
                break;
            }
            SkASSERT(0); // incomplete
        }
        if (Definition::Type::kKeyWord == token.fType) {
            switch (token.fKeyWord) {
                case KeyWord::kEnum:
                    inEnum = true;
                    break;
                case KeyWord::kConst:
                case KeyWord::kConstExpr:
                case KeyWord::kStatic:
                case KeyWord::kInt:
                case KeyWord::kUint8_t:
                case KeyWord::kUint16_t:
                case KeyWord::kUint32_t:
                case KeyWord::kUint64_t:
                case KeyWord::kUintPtr_t:
                case KeyWord::kSize_t:
                case KeyWord::kFloat:
                case KeyWord::kBool:
                case KeyWord::kVoid:
                    if (!typeStart) {
                        typeStart = &token;
                    }
                    break;
                default:
                    break;
            }
            continue;
        }
        if (Definition::Type::kPunctuation == token.fType) {
            if (inEnum) {
                SkASSERT(Punctuation::kSemicolon == token.fPunctuation);
                inEnum = false;
            }
            if (inMethod) {
                if (Punctuation::kColon == token.fPunctuation) {
                    inMethod = false;
                } else if (Punctuation::kLeftBrace == token.fPunctuation) {
                    inMethod = false;
                } else if (Punctuation::kSemicolon == token.fPunctuation) {
                    inMethod = false;
                } else {
                    SkASSERT(0);  // incomplete
                }
            }
            if (inMember) {
                SkASSERT(Punctuation::kSemicolon == token.fPunctuation);
                typeStart = nullptr;
                inMember = false;
            }
            continue;
        }
        if (Definition::Type::kWord != token.fType) {
            SkASSERT(0); // incomplete
        }
        if (MarkType::kMember == token.fMarkType) {
            TextParser typeStr(token.fFileName, typeStart->fContentStart, token.fContentStart,
                    token.fLineCount);
            typeStr.trimEnd();
            longestType = SkTMax(longestType, (int) (typeStr.fEnd - typeStr.fStart));
            longestName = SkTMax(longestName, (int) (token.fContentEnd - token.fContentStart));
            typeStart->fMemberStart = true;
            inMember = true;
            string tokenName(token.fContentStart, (int) (token.fContentEnd - token.fContentStart));
            Definition* commentBlock = this->findMemberCommentBlock(fBmhStructDef->fChildren,
                    tokenName);
            if (!commentBlock) {
                return token.reportError<void>("member missing comment block 1");
            }
            auto lineIter = std::find_if(commentBlock->fChildren.begin(),
                    commentBlock->fChildren.end(),
                    [](const Definition* def){ return MarkType::kLine == def->fMarkType; } );
            SkASSERT(commentBlock->fChildren.end() != lineIter);
            const Definition* lineDef = *lineIter;
            longestComment = SkTMax(longestComment, lineDef->length());
            continue;
        }
        if (MarkType::kMethod == token.fMarkType) {
            inMethod = true;
            continue;
        }
        SkASSERT(MarkType::kNone == token.fMarkType);
        if (typeStart) {
            if (inMember) {
                longestValue =
                        SkTMax(longestValue, (int) (token.fContentEnd - token.fContentStart));
            }
        } else {
            typeStart = &token;
        }
    }
    fStructMemberTab = longestType + fIndent + 1 /* space before name */ ;
    fStructValueTab = fStructMemberTab + longestName + 2 /* space ; */ ;
    fStructCommentTab = fStructValueTab;
    if (longestValue) {
        fStructCommentTab += longestValue + 3 /* space = space */ ;
        fStructValueTab -= 1 /* ; */ ;
    }
    fStructMemberLength = fStructCommentTab + longestComment;
    // iterate through struct to ensure that members' comments fit on line
    // struct or class may not have any members
    (void) this->checkChildCommentLength(fBmhStructDef, MarkType::kMember);
}

static bool find_start(const Definition* startDef, const char* start) {
    for (const auto& child : startDef->fTokens) {
        if (child.fContentStart == start) {
            return MarkType::kMethod == child.fMarkType;
        }
        if (child.fContentStart >= start) {
            break;
        }
        if (find_start(&child, start)) {
            return true;
        }
    }
    return false;
}

bool IncludeWriter::populate(Definition* def, ParentPair* prevPair, RootDefinition* root) {
    if (!def->fTokens.size()) {
        return true;
    }
    ParentPair pair = { def, prevPair };
    // write bulk of original include up to class, method, enum, etc., excepting preceding comment
    // find associated bmh object
    // write any associated comments in Doxygen form
    // skip include comment
    // if there is a series of same named methods, write one set of comments, then write all methods
    string methodName;
    Definition* method = nullptr;
    Definition* clonedMethod = nullptr;
    const Definition* memberStart = nullptr;
    const Definition* memberEnd = nullptr;
    fContinuation = nullptr;
    bool inStruct = false;
    bool inConstructor = false;
    bool inInline = false;
    bool eatOperator = false;
    bool sawConst = false;
    bool staticOnly = false;
    bool sawTypedef = false;
    Definition* deferredTypedefComment = nullptr;
    const Definition* requireDense = nullptr;
    const Definition* startDef = nullptr;
    for (auto& child : def->fTokens) {
        if (KeyWord::kInline == child.fKeyWord) {
            continue;
        }
        if (KeyWord::kOperator == child.fKeyWord && method &&
                Definition::MethodType::kOperator == method->fMethodType) {
            eatOperator = true;
            continue;
        }
        if (eatOperator) {
            if (Bracket::kSquare == child.fBracket || Bracket::kParen == child.fBracket) {
                continue;
            }
            eatOperator = false;
            fContinuation = nullptr;
            if (KeyWord::kConst == child.fKeyWord) {
                continue;
            }
        }
        if (memberEnd) {
            if (memberEnd != &child) {
                continue;
            }
            startDef = &child;
            this->setStart(child.fContentStart + 1, &child);
            memberEnd = nullptr;
        }
        if (child.fPrivate) {
            if (MarkType::kMethod == child.fMarkType) {
                inInline = true;
            }
            continue;
        }
        if (inInline) {
            if (Definition::Type::kKeyWord == child.fType) {
                SkASSERT(MarkType::kMethod != child.fMarkType);
                continue;
            }
            if (Definition::Type::kPunctuation == child.fType) {
                if (Punctuation::kLeftBrace == child.fPunctuation) {
                    inInline = false;
                } else {
                    SkASSERT(Punctuation::kAsterisk == child.fPunctuation);
                }
                continue;
            }
            if (Definition::Type::kWord == child.fType) {
                string name(child.fContentStart, child.fContentEnd - child.fContentStart);
                SkASSERT(string::npos != name.find("::"));
                continue;
            }
            if (Definition::Type::kBracket == child.fType) {
                SkASSERT(Bracket::kParen == child.fBracket);
                continue;
            }
        }
        if (fContinuation) {
            if (Definition::Type::kKeyWord == child.fType) {
                if (KeyWord::kFriend == child.fKeyWord ||
                        KeyWord::kSK_API == child.fKeyWord) {
                    continue;
                }
                const IncludeKey& includeKey = kKeyWords[(int) child.fKeyWord];
                if (KeyProperty::kNumber == includeKey.fProperty) {
                    continue;
                }
            }
            if (Definition::Type::kBracket == child.fType) {
                if (Bracket::kAngle == child.fBracket) {
                    continue;
                }
                if (Bracket::kParen == child.fBracket) {
                    if (!clonedMethod) {
                        if (inConstructor) {
                            fContinuation = child.fContentStart;
                        }
                        continue;
                    }
                    int alternate = 1;
                    ptrdiff_t childLen = child.fContentEnd - child.fContentStart;
                    SkASSERT(')' == child.fContentStart[childLen]);
                    ++childLen;
                    do {
                        TextParser params(clonedMethod->fFileName, clonedMethod->fStart,
                            clonedMethod->fContentStart, clonedMethod->fLineCount);
                        params.skipToEndBracket('(');
                        if (params.startsWith(child.fContentStart, childLen)) {
                            this->methodOut(clonedMethod, child);
                            sawConst = false;
                            break;
                        }
                        ++alternate;
                        string alternateMethod = methodName + '_' + to_string(alternate);
                       clonedMethod = this->findMethod(alternateMethod, root);
                    } while (clonedMethod);
                    if (!clonedMethod) {
                        return child.reportError<bool>("cloned method not found");
                    }
                    clonedMethod = nullptr;
                    continue;
                }
            }
            if (Definition::Type::kWord == child.fType) {
                if (clonedMethod) {
                    continue;
                }
                size_t len = (size_t) (child.fContentEnd - child.fContentStart);
                const char operatorStr[] = "operator";
                size_t operatorLen = sizeof(operatorStr) - 1;
                if (len >= operatorLen && !strncmp(child.fContentStart, operatorStr, operatorLen)) {
                    fContinuation = child.fContentEnd;
                    continue;
                }
            }
            if (Definition::Type::kPunctuation == child.fType &&
                    (Punctuation::kSemicolon == child.fPunctuation ||
                    Punctuation::kLeftBrace == child.fPunctuation ||
                    (Punctuation::kColon == child.fPunctuation && inConstructor))) {
                SkASSERT(fContinuation[0] == '(');
                const char* continueEnd = child.fContentStart;
                while (continueEnd > fContinuation && isspace(continueEnd[-1])) {
                    --continueEnd;
                }
                const char defaultTag[] = " = default";
                size_t tagSize = sizeof(defaultTag) - 1;
                const char* tokenEnd = continueEnd - tagSize;
                if (tokenEnd <= fContinuation || strncmp(tokenEnd, defaultTag, tagSize)) {
                    tokenEnd = continueEnd;
                }
                methodName += string(fContinuation, tokenEnd - fContinuation);
                if (string::npos != methodName.find('\n')) {
                    methodName.erase(std::remove(methodName.begin(), methodName.end(), '\n'),
                                    methodName.end());
                }
                method = this->findMethod(methodName, root);
                if (!method) {
                    if (fBmhStructDef && fBmhStructDef->fDeprecated) {
                        fContinuation = nullptr;
                        continue;
                    }
                    return child.reportError<bool>("method not found");
                }
                this->methodOut(method, child);
                sawConst = false;
                continue;
            }
            if (Definition::Type::kPunctuation == child.fType &&
                    Punctuation::kAsterisk == child.fPunctuation &&
                    clonedMethod) {
                continue;
            }
            if (inConstructor) {
                continue;
            }
            method = this->findMethod(methodName + "()", root);
            if (method) {
                if (method->fCloned) {
                    clonedMethod = method;
                    continue;
                }
                this->methodOut(method, child);
                sawConst = false;
                continue;
            } else if (fBmhStructDef && fBmhStructDef->fDeprecated) {
                fContinuation = nullptr;
                continue;
            }
            return child.reportError<bool>("method not found");
        }
        if (Bracket::kSlashSlash == child.fBracket || Bracket::kSlashStar == child.fBracket) {
            if (!fDeferComment) {
                fDeferComment = &child;
            }
            continue;
        }
        if (MarkType::kMethod == child.fMarkType) {
            if (this->isInternalName(child)) {
                continue;
            }
            const char* bodyEnd = fDeferComment ? fDeferComment->fContentStart - 1 :
                    fAttrDeprecated ? fAttrDeprecated->fContentStart - 1 :
                    child.fContentStart;
            if (Definition::Type::kBracket == def->fType && Bracket::kDebugCode == def->fBracket) {
                auto tokenIter = def->fParent->fTokens.begin();
                std::advance(tokenIter, def->fParentIndex - 1);
                Definition* prior = &*tokenIter;
                if (Definition::Type::kBracket == def->fType &&
                        Bracket::kSlashStar == prior->fBracket) {
                    bodyEnd = prior->fContentStart - 1;
                }
            }
            // FIXME: roll end-trimming into writeBlockTrim call
            while (fStart < bodyEnd && ' ' >= bodyEnd[-1]) {
                --bodyEnd;
            }
            int blockSize = (int) (bodyEnd - fStart);
            SkASSERT(blockSize >= 0);
            if (blockSize) {
                string debugstr(fStart, blockSize);
                this->writeBlock(blockSize, fStart);
            }
            startDef = &child;
            this->setStart(child.fContentStart, &child);
            auto mapFind = fBmhParser->fMethodMap.find(child.fName);
            if (fBmhParser->fMethodMap.end() != mapFind) {
                inConstructor = false;
                method = &mapFind->second;
                methodName = child.fName;
            } else {
                methodName = root->fName + "::" + child.fName;
                size_t lastName = root->fName.rfind(':');
                lastName = string::npos == lastName ? 0 : lastName + 1;
                inConstructor = root->fName.substr(lastName) == child.fName;
                method = root->find(methodName, RootDefinition::AllowParens::kNo);
            }
            fContinuation = child.fContentEnd;
            if (!method) {
                continue;
            }
            if (method->fCloned) {
                clonedMethod = method;
                continue;
            }
            this->methodOut(method, child);
            sawConst = false;
            if (fAttrDeprecated) {
                startDef = fAttrDeprecated;
                this->setStartBack(fAttrDeprecated->fContentStart, fAttrDeprecated);
                fAttrDeprecated = nullptr;
            }
            continue;
        }
        if (Definition::Type::kKeyWord == child.fType) {
            switch (child.fKeyWord) {
                case KeyWord::kStruct:
                case KeyWord::kClass:
                    fICSStack.push_back(&child);
                    fStructEnded = false;
                    fStructMemberTab = 0;
                    // if struct contains members, compute their name and comment tabs
                    if (child.fChildren.size() > 0) {
                        const ParentPair* testPair = &pair;
                        while ((testPair = testPair->fPrev)) {
                            if (KeyWord::kClass == testPair->fParent->fKeyWord) {
                                inStruct = fInStruct = true;
                                break;
                            }
                        }
                    }
                    if (fInStruct) {
                        // try child; root+child; root->parent+child; etc.
                        int trial = 0;
                        RootDefinition* search = root;
                        Definition* parent = search->fParent;
                        do {
                            string name;
                            if (0 == trial) {
                                name = child.fName;
                            } else if (1 == trial) {
                                name = root->fName + "::" + child.fName;
                            } else if (2 == trial) {
                                name = root->fName;
                            } else {
                                SkASSERT(parent);
                                name = parent->fName + "::" + child.fName;
                                search = parent->asRoot();
                                parent = search->fParent;
                            }
                            fBmhStructDef = search->find(name, RootDefinition::AllowParens::kNo);
                        } while (!fBmhStructDef && ++trial);
                        root = fBmhStructDef->asRoot();
                        SkASSERT(root);
                        fIndent += 4;
                        this->structSizeMembers(child);
                        fIndent -= 4;
                        SkASSERT(!fIndentNext);
                        fIndentNext = true;
                    }
                    if (child.fChildren.size() > 0) {
                        const char* bodyEnd = fDeferComment ? fDeferComment->fContentStart - 1 :
                                child.fContentStart;
                        this->writeBlockTrim((int) (bodyEnd - fStart), fStart);
                        if (fPendingMethod) {
                            if (fIndent >= 4) {
                                this->indentOut();
                            }
                            fPendingMethod = false;
                        }
                        startDef = requireDense ? requireDense : &child;
                        if (requireDense) {
                            startDef = requireDense;
                            this->setStart(requireDense->fContentStart, requireDense);
                        } else {
                            startDef = &child;
                            this->setStart(child.fContentStart, &child);
                        }
                        requireDense = nullptr;
                        if (!fInStruct && (!root || child.fName != root->fName)) {
                            root = &fBmhParser->fClassMap[child.fName];
                            fRootTopic = root->fParent;
                            SkASSERT(!root->fVisited);
                            root->clearVisited();
#if 0
    // this seems better balanced; but real problem is probably fInStruct
                            if (fIndentStack.size() > 0) {
                                this->indentOut();
                            }
                            SkASSERT(!fIndent);
#else
                            fIndent = 0;
#endif
                            fBmhStructDef = root;
                        }
                        if (child.fName == root->fName) {
                            if (Definition* parent = root->fParent) {
                                if (MarkType::kTopic == parent->fMarkType ||
                                        MarkType::kSubtopic == parent->fMarkType) {
                                    const char* commentStart = root->fContentStart;
                                    const char* commentEnd = root->fChildren[0]->fStart;
                                    this->structOut(root, *root, commentStart, commentEnd);
                                } else {
                                    SkASSERT(0); // incomplete
                                }
                            } else {
                                SkASSERT(0); // incomplete
                            }
                        } else {
                            SkASSERT(fInStruct);
                            Definition* priorBlock = fBmhStructDef;
                            Definition* codeBlock = nullptr;
                            Definition* nextBlock = nullptr;
                            for (auto test : fBmhStructDef->fChildren) {
                                if (MarkType::kCode == test->fMarkType) {
                                    SkASSERT(!codeBlock);  // FIXME: check enum earlier
                                    codeBlock = test;
                                    continue;
                                }
                                if (codeBlock) {
                                    nextBlock = test;
                                    break;
                                }
                                priorBlock = test;
                            }
                      // FIXME: trigger error earlier if inner #Struct or #Class is missing #Code
                            if (!fBmhStructDef->fDeprecated) {
                                SkASSERT(codeBlock);
                                SkASSERT(nextBlock);  // FIXME: check enum for correct order earlier
                                const char* commentStart = codeBlock->fTerminator;
                                const char* commentEnd = nextBlock->fStart;
                      // FIXME: trigger error if #Code is present but comment is before it earlier
                                SkASSERT(priorBlock); // code always preceded by #Line (I think)
                                TextParser priorComment(priorBlock->fFileName,
                                        priorBlock->fTerminator, codeBlock->fStart,
                                        priorBlock->fLineCount);
                                priorComment.trimEnd();
                                if (!priorComment.eof()) {
                                    return priorBlock->reportError<bool>(
                                            "expect no comment before #Code");
                                }
                                TextParser nextComment(codeBlock->fFileName, commentStart,
                                        commentEnd, codeBlock->fLineCount);
                                nextComment.trimEnd();
                                if (!priorComment.eof()) {
                                    return priorBlock->reportError<bool>(
                                            "expect comment after #Code");
                                }
                                if (!nextComment.eof()) {

                                }
                                fIndentNext = true;
                                this->structOut(root, *fBmhStructDef, commentStart, commentEnd);
                            }
                        }
                        fDeferComment = nullptr;
                    } else {
                       // empty forward reference
                        bool writeTwo = '\n' == child.fContentStart[-1]
                                && '\n' == child.fContentStart[-2];
                        if (writeTwo) {
                            const char* bodyEnd = fDeferComment ? fDeferComment->fContentStart - 1 :
                                    child.fContentStart;
                            this->writeBlockTrim((int) (bodyEnd - fStart), fStart);
                            this->lf(writeTwo ? 2 : 1);
                            fIndent = 0;
                            this->writeBlockTrim(child.length() + 1, child.fContentStart);
                            writeTwo = '\n' == child.fContentEnd[1]
                                    && '\n' == child.fContentStart[2];
                            this->lf(writeTwo ? 2 : 1);
                            fStart = child.fContentEnd + 1;
                            fDeferComment = nullptr;
                        }
                    }
                    break;
                case KeyWord::kEnum: {
                    fInEnum = true;
                    this->enumHeaderOut(root, child);
                    this->enumSizeItems(child);
                } break;
                case KeyWord::kConst:
                case KeyWord::kConstExpr:
                    sawConst = !memberStart || staticOnly;
                    if (!memberStart) {
                        memberStart = &child;
                        staticOnly = true;
                    }
                    if (MarkType::kConst == child.fMarkType) {
                        auto constIter = fBmhParser->fConstMap.find(child.fName);
                        if (fBmhParser->fConstMap.end() != constIter) {
                            const RootDefinition& bmhConst = constIter->second;
                            this->constOut(&child, &bmhConst);
                            fDeferComment = nullptr;
                        }
                    }
                    break;
                case KeyWord::kStatic:
                    if (!memberStart) {
                        memberStart = &child;
                        staticOnly = true;
                    }
                    break;
                case KeyWord::kInt:
                case KeyWord::kUint8_t:
                case KeyWord::kUint16_t:
                case KeyWord::kUint32_t:
                case KeyWord::kUint64_t:
                case KeyWord::kUintPtr_t:
                case KeyWord::kUnsigned:
                case KeyWord::kSize_t:
                case KeyWord::kFloat:
                case KeyWord::kBool:
                case KeyWord::kChar:
                case KeyWord::kVoid:
                    staticOnly = false;
                    if (!memberStart) {
                        memberStart = &child;
                    }
                    break;
                case KeyWord::kPublic:
                case KeyWord::kPrivate:
                case KeyWord::kProtected:
                case KeyWord::kFriend:
                case KeyWord::kInline:
                case KeyWord::kSK_API:
                case KeyWord::kTemplate:
                    break;
                case KeyWord::kTypedef:
                    SkASSERT(!memberStart);
                    memberStart = &child;
                    deferredTypedefComment = fDeferComment;
                    sawTypedef = true;
                    break;
                case KeyWord::kSK_BEGIN_REQUIRE_DENSE:
                    requireDense = &child;
                    break;
                default:
                    SkASSERT(0);
            }
            if (KeyWord::kUint8_t == child.fKeyWord || KeyWord::kUint32_t == child.fKeyWord) {
                continue;
            } else {
                if (fInEnum && KeyWord::kClass == child.fChildren[0]->fKeyWord) {
                    if (!this->populate(child.fChildren[0], &pair, root)) {
                        return false;
                    }
                } else {
                    if (!this->populate(&child, &pair, root)) {
                        return false;
                    }
                    if (KeyWord::kClass == child.fKeyWord || KeyWord::kStruct == child.fKeyWord) {
                        fICSStack.pop_back();
                        fStructEnded = true;
                        if (fInStruct) {
                            fInStruct = false;
                            do {
                                SkASSERT(root);
                                root = const_cast<RootDefinition*>(root->fParent->asRoot());
                            } while (MarkType::kTopic == root->fMarkType ||
                                    MarkType::kSubtopic == root->fMarkType);
#if 0
                        }
                        if (MarkType::kStruct == root->fMarkType ||
                                MarkType::kClass == root->fMarkType) {
#else
                            SkASSERT(MarkType::kStruct == root->fMarkType ||
                                    MarkType::kClass == root->fMarkType);
#endif
                            fPendingMethod = false;
                            if (startDef) {
                                fPendingMethod = find_start(startDef, fStart);
                            }
                            fOutdentNext = !fPendingMethod;
                        }
                    }
                }
            }
            continue;
        }
        if (Definition::Type::kBracket == child.fType) {
            if (KeyWord::kEnum == child.fParent->fKeyWord ||
                    (KeyWord::kClass == child.fParent->fKeyWord && child.fParent->fParent &&
                    KeyWord::kEnum == child.fParent->fParent->fKeyWord)) {
                SkASSERT(Bracket::kBrace == child.fBracket);
                this->enumMembersOut(*child.fParent);
                this->writeString("};");
                this->lf(2);
                startDef = child.fParent;
                this->setStart(child.fParent->fContentEnd, child.fParent);
                SkASSERT(';' == fStart[0]);
                ++fStart;
                fDeferComment = nullptr;
                fInEnum = false;
                if (fIndentNext) {
//                    fIndent -= 4;
                    fIndentNext = false;
                }
                continue;
            }
            if (fAttrDeprecated) {
                continue;
            }
            if (KeyWord::kDefine == child.fKeyWord && this->defineOut(child)) {
                fDeferComment = nullptr;
                continue;
            }
            fDeferComment = nullptr;
            if (KeyWord::kClass == def->fKeyWord || KeyWord::kStruct == def->fKeyWord) {
                fIndentNext = true;
            }
            if (!this->populate(&child, &pair, root)) {
                return false;
            }
            if (KeyWord::kClass == def->fKeyWord || KeyWord::kStruct == def->fKeyWord) {
                if (def->iRootParent() && (!fStartSetter
                        || MarkType::kMethod != fStartSetter->fMarkType)) {
                    this->setStart(child.fContentEnd, &child);
                    fDeferComment = nullptr;
                }
            }
            continue;
        }
        if (Definition::Type::kWord == child.fType) {
            if (MarkType::kMember == child.fMarkType) {
                if (!memberStart) {
                    auto iter = def->fTokens.begin();
                    std::advance(iter, child.fParentIndex - 1);
                    memberStart = &*iter;
                    staticOnly = false;
                }
                if (!fStructMemberTab) {
                    SkASSERT(KeyWord::kStruct == def->fParent->fKeyWord);
                    fIndent += 4;
                    this->structSizeMembers(*def->fParent);
                    fIndent -= 4;
                    fIndentNext = true;
                }
                SkASSERT(fBmhStructDef);
                if (!fBmhStructDef->fDeprecated) {
                    memberEnd = this->structMemberOut(memberStart, child);
                    startDef = &child;
                    this->setStart(child.fContentEnd + 1, &child);
                    fDeferComment = nullptr;
                }
            } else if (MarkType::kNone == child.fMarkType && sawConst
                    && fEnumDef && !fEnumDef->fDeprecated) {
                const Definition* bmhConst = nullptr;
                string match;
                if (root) {
                    match = root->fName + "::";
                }
                match += string(child.fContentStart, child.fContentEnd - child.fContentStart);
                for (auto enumChild : fEnumDef->fChildren) {
                    if (MarkType::kConst == enumChild->fMarkType && enumChild->fName == match) {
                        bmhConst = enumChild;
                        break;
                    }
                }
                if (bmhConst) {
                    this->constOut(memberStart, bmhConst);
                    fDeferComment = nullptr;
                    sawConst = false;
                }
            } else if (MarkType::kNone == child.fMarkType && sawConst && !fEnumDef) {
                string match;
                if (root) {
                    match = root->fName + "::";
                    match += string(child.fContentStart, child.fContentEnd - child.fContentStart);
                    auto bmhClassIter = fBmhParser->fClassMap.find(root->fName);
                    if (fBmhParser->fClassMap.end() != bmhClassIter) {
                        RootDefinition& bmhClass = bmhClassIter->second;
                        auto constIter = std::find_if(bmhClass.fLeaves.begin(), bmhClass.fLeaves.end(),
                                [match](std::pair<const string, Definition>& leaf){ return match == leaf.second.fName; } );
                        if (bmhClass.fLeaves.end() != constIter) {
                            const Definition& bmhConst = constIter->second;
                            if (MarkType::kConst == bmhConst.fMarkType
                                    && MarkType::kSubtopic == bmhConst.fParent->fMarkType) {
                                fBmhConst = &bmhConst;
                                fConstDef = &child;
                            }
                        }
                    }
                }
            }
            if (child.fMemberStart) {
                memberStart = &child;
                staticOnly = false;
            }
            if (kAttrDeprecatedLen == (size_t) (child.fContentEnd - child.fContentStart) &&
                    !strncmp(gAttrDeprecated, child.fStart, kAttrDeprecatedLen)) {
                fAttrDeprecated = &child;
            }
            continue;
        }
        if (Definition::Type::kPunctuation == child.fType) {
            if (Punctuation::kSemicolon == child.fPunctuation) {
                if (sawConst && fBmhConst) {  // find bmh documentation. Parent must be subtopic.
                    const Definition* subtopic = fBmhConst->fParent;
                    SkASSERT(subtopic);
                    SkASSERT(MarkType::kSubtopic == subtopic->fMarkType);
                    auto firstConst = std::find_if(subtopic->fChildren.begin(),
                            subtopic->fChildren.end(),
                            [](const Definition* def){ return MarkType::kConst == def->fMarkType;});
                    SkASSERT(firstConst != subtopic->fChildren.end());
                    bool constIsFirst = *firstConst == fBmhConst;
                    if (constIsFirst) {  // If first #Const child, output subtopic description.
                        this->constOut(memberStart, subtopic);
                        // find member / value / comment tabs
                        // look for a one-to-one correspondence between bmh and include
                        this->constSizeMembers(root);
                        fDeferComment = nullptr;
                    }
                    // after const code, output #Line description as short comment
                    auto lineIter = std::find_if(fBmhConst->fChildren.begin(),
                            fBmhConst->fChildren.end(),
                            [](const Definition* def){ return MarkType::kLine == def->fMarkType; });
                    SkASSERT(fBmhConst->fChildren.end() != lineIter);
                    const Definition* lineDef = *lineIter;
                    if (fConstLength > 100) {
                        this->writeCommentHeader();
                        this->writeSpace();
                        this->rewriteBlock(lineDef->length(), lineDef->fContentStart, Phrase::kYes);
                        this->writeCommentTrailer(OneLine::kYes);
                    }
                    this->lfcr();
                    TextParser constText(memberStart);
                    const char* nameEnd = constText.trimmedBracketEnd('=');
                    SkAssertResult(constText.skipToEndBracket('='));
                    const char* valueEnd = constText.trimmedBracketEnd(';');
                    this->writeBlock((int) (nameEnd - memberStart->fContentStart),
                            memberStart->fContentStart);
                    this->indentToColumn(fConstValueTab);
                    this->writeBlock((int) (valueEnd - constText.fChar), constText.fChar);
                    this->writeString(";");
                    if (fConstLength <= 100) {
                        this->indentToColumn(fConstCommentTab);
                        this->writeString("//!<");
                        this->writeSpace();
                        this->rewriteBlock(lineDef->length(), lineDef->fContentStart, Phrase::kYes);
                    }
                    this->setStart(child.fContentStart + 1, &child);
                    fDeferComment = nullptr;
                    fBmhConst = nullptr;
                    sawConst = false;
                } else if (sawTypedef) {
                    const Definition* bmhTypedef = nullptr;
                    if (root) {
                        SkDEBUGCODE(auto classIter = fBmhParser->fClassMap.find(root->fName));
                        SkASSERT(fBmhParser->fClassMap.end() != classIter);
                        RootDefinition& classDef = fBmhParser->fClassMap[root->fName];
                        auto leafIter = classDef.fLeaves.find(memberStart->fName);
                        if (classDef.fLeaves.end() != leafIter) {
                            bmhTypedef = &leafIter->second;
                        }
                    }
                    if (!bmhTypedef) {
                        auto typedefIter = fBmhParser->fTypedefMap.find(memberStart->fName);
                        SkASSERT(fBmhParser->fTypedefMap.end() != typedefIter);
                        bmhTypedef = &typedefIter->second;
                    }
                    fDeferComment = deferredTypedefComment;
                    this->constOut(memberStart, bmhTypedef);
                    fDeferComment = nullptr;
                    sawTypedef = false;
                }
                memberStart = nullptr;
                staticOnly = false;
                if (inStruct) {
                    fInStruct = false;
                }
                continue;
            }
            if (Punctuation::kLeftBrace == child.fPunctuation ||
                    Punctuation::kColon == child.fPunctuation ||
                    Punctuation::kAsterisk == child.fPunctuation
                ) {
                continue;
            }
        }
    }
    return true;
}

bool IncludeWriter::populate(BmhParser& bmhParser) {
    bool allPassed = true;
    for (auto& includeMapper : fIncludeMap) {
        size_t lastSlash = includeMapper.first.rfind('/');
        if (string::npos == lastSlash) {
            lastSlash = includeMapper.first.rfind('\\');
        }
        if (string::npos == lastSlash || lastSlash >= includeMapper.first.length() - 1) {
            return this->reportError<bool>("malformed include name");
        }
        string fileName = includeMapper.first.substr(lastSlash + 1);
        if (".h" != fileName.substr(fileName.length() - 2)) {
            return this->reportError<bool>("expected fileName.h");
        }
        string skClassName = fileName.substr(0, fileName.length() - 2);
        this->reset();
        fOut = fopen(fileName.c_str(), "wb");
        if (!fOut) {
            SkDebugf("could not open output file %s\n", fileName.c_str());
            return false;
        }
        RootDefinition* root =
                bmhParser.fClassMap.end() == bmhParser.fClassMap.find(skClassName) ?
                nullptr : &bmhParser.fClassMap[skClassName];
        fBmhParser = &bmhParser;
        if (root) {
            fRootTopic = root->fParent;
            root->clearVisited();
        } else {
            SkASSERT("Sk" == skClassName.substr(0, 2));
            string topicName = skClassName.substr(2);
            auto topicIter = bmhParser.fTopicMap.find(topicName);
            SkASSERT(bmhParser.fTopicMap.end() != topicIter);
            fRootTopic = topicIter->second->asRoot();
            fFirstWrite = true;   // write file information after includes
        }
        fFileName = includeMapper.second.fFileName;
        this->setStartBack(includeMapper.second.fContentStart, &includeMapper.second);
        fEnd = includeMapper.second.fContentEnd;
        fAnonymousEnumCount = 1;
        this->writeHeader(includeMapper);
        allPassed &= this->populate(&includeMapper.second, nullptr, root);
        this->writeBlock((int) (fEnd - fStart), fStart);
#if 0
        if (fIndentStack.size() > 0) {
            this->indentOut();
        }
        SkASSERT(!fIndent);
#else
        fIndent = 0;
#endif
        this->lfcr();
        this->writePending();
        fclose(fOut);
        fflush(fOut);
        size_t slash = fFileName.find_last_of('/');
        if (string::npos == slash) {
            slash = 0;
        }
        size_t back = fFileName.find_last_of('\\');
        if (string::npos == back) {
            back = 0;
        }
        string dir = fFileName.substr(0, SkTMax(slash, back) + 1);
        string readname = dir + fileName;
        if (this->writtenFileDiffers(fileName, readname)) {
            SkDebugf("wrote updated %s\n", fileName.c_str());
        } else {
            remove(fileName.c_str());
        }
    }
    return allPassed;
}

// change Xxx_Xxx to xxx xxx
static string ConvertRef(const string str, bool first) {
    string substitute;
    for (char c : str) {
        if ('_' == c) {
            c = ' ';  // change Xxx_Xxx to xxx xxx
        } else if (isupper(c) && !first) {
            c = tolower(c);
        }
        substitute += c;
        first = false;
    }
    return substitute;
}

string IncludeWriter::resolveMethod(const char* start, const char* end, bool first) {
    string methodname(start, end - start);
    if (string::npos != methodname.find("()")) {
        return "";
    }
    string substitute;
    auto rootDefIter = fBmhParser->fMethodMap.find(methodname);
    if (fBmhParser->fMethodMap.end() != rootDefIter) {
        substitute = methodname + "()";
    } else {
        RootDefinition* parent = nullptr;
        for (auto candidate : fRootTopic->fChildren) {
            if (MarkType::kClass == candidate->fMarkType
                    || MarkType::kStruct == candidate->fMarkType) {
                parent = candidate->asRoot();
                break;
            }
        }
        if (parent) {
            auto defRef = parent->find(parent->fName + "::" + methodname,
                    RootDefinition::AllowParens::kNo);
            if (defRef && MarkType::kMethod == defRef->fMarkType) {
                substitute = methodname + "()";
            } else {
                auto defineIter = fBmhParser->fDefineMap.find(methodname);
                if (fBmhParser->fDefineMap.end() != defineIter) {
                    const RootDefinition& defineDef = defineIter->second;
                    auto codeIter = std::find_if(defineDef.fChildren.begin(),
                            defineDef.fChildren.end(),
                            [](Definition* child){ return MarkType::kCode == child->fMarkType; } );
                    if (defineDef.fChildren.end() != codeIter) {
                        const Definition* codeDef = *codeIter;
                        string codeContents(codeDef->fContentStart, codeDef->length());
                        size_t namePos = codeContents.find(methodname);
                        if (string::npos != namePos) {
                            size_t parenPos = namePos + methodname.length();
                            if (parenPos < codeContents.length() && '(' == codeContents[parenPos]) {
                                substitute = methodname + "()";
                            }
                        }
                    }
                }
            }
        }
    }
    if (fMethodDef && methodname == fMethodDef->fName) {
        TextParser report(fBmhMethod);
        report.reportError("method should not include references to itself");
        return "";
    }
    if (fBmhMethod) {
        for (auto child : fBmhMethod->fChildren) {
            if (MarkType::kParam != child->fMarkType) {
                continue;
            }
            if (methodname == child->fName) {
                return "";
            }
        }
    }
    return substitute;
}

string IncludeWriter::resolveAlias(const Definition* def) {
    for (auto child : def->fChildren) {
        if (MarkType::kSubstitute == child->fMarkType) {
            return string(child->fContentStart, (int) (child->fContentEnd - child->fContentStart));
        }
        if (MarkType::kAlias == child->fMarkType && def->fName == child->fName) {
            return this->resolveAlias(child);
        }
    }
    return "";
}

string IncludeWriter::resolveRef(const char* start, const char* end, bool first,
        RefType* refType) {
        // look up Xxx_Xxx
    string undername(start, end - start);
    for (const auto& external : fBmhParser->fExternals) {
        if (external.fName == undername) {
            *refType = RefType::kExternal;
            return external.fName;
        }
    }
    *refType = RefType::kNormal;
    SkASSERT(string::npos == undername.find(' '));
    const Definition* rootDef = nullptr;
    string substitute;
    {
        auto rootDefIter = fBmhParser->fTopicMap.find(undername);
        if (fBmhParser->fTopicMap.end() != rootDefIter) {
            rootDef = rootDefIter->second;
        } else {
            string prefixedName = fRootTopic->fName + '_' + undername;
            rootDefIter = fBmhParser->fTopicMap.find(prefixedName);
            if (fBmhParser->fTopicMap.end() != rootDefIter) {
                rootDef = rootDefIter->second;
            } else if (fBmhStructDef) {
                string localPrefix = fBmhStructDef->fFiddle + '_' + undername;
                rootDefIter = fBmhParser->fTopicMap.find(localPrefix);
                if (fBmhParser->fTopicMap.end() != rootDefIter) {
                    rootDef = rootDefIter->second;
                }
                if (!rootDef) {
                    size_t doubleColon = fBmhStructDef->fName.rfind("::");
                    if (string::npos != doubleColon && undername
                            == fBmhStructDef->fName.substr(doubleColon + 2)) {
                        substitute = fBmhStructDef->fName;
                    }
                }
            }
            if (!rootDef && fEnumDef && "Sk" + prefixedName == fEnumDef->fFiddle) {
                rootDef = fEnumDef;
            }
            if (!rootDef && !substitute.length()) {
                auto aliasIter = fBmhParser->fAliasMap.find(undername);
                if (fBmhParser->fAliasMap.end() != aliasIter) {
                    rootDef = aliasIter->second;
                } else if (fInEnum && fEnumDef && this->findEnumSubtopic(undername, &rootDef)) {
                    ;
                } else if (!first) {
                    this->fChar = start;
                    this->fLine = start;
                    this->reportError("reference unfound");
                    return "";
                }
            }
        }
    }
    if (rootDef) {
        MarkType rootType = rootDef->fMarkType;
        if (MarkType::kSubtopic == rootType || MarkType::kTopic == rootType
                || MarkType::kAlias == rootType) {
            substitute = this->resolveAlias(rootDef);
        }
        if (!substitute.length()) {
            string match = rootDef->fName;
            size_t index;
            while (string::npos != (index = match.find('_'))) {
                match.erase(index, 1);
            }
            string skmatch = "Sk" + match;
            auto parent = MarkType::kAlias == rootType ? rootDef->fParent : rootDef;
            for (auto child : parent->fChildren) {
                // there may be more than one
                // prefer the one mostly closely matching in text
                if ((MarkType::kClass == child->fMarkType ||
                    MarkType::kStruct == child->fMarkType ||
                    (MarkType::kEnum == child->fMarkType && !child->fAnonymous) ||
                    MarkType::kEnumClass == child->fMarkType) && (match == child->fName ||
                    skmatch == child->fName)) {
                    substitute = child->fName;
                    break;
                }
            }
        }
        if (!substitute.length()) {
            for (auto child : rootDef->fChildren) {
                // there may be more than one
                // if so, it's a bug since it's unknown which is the right one
                if (MarkType::kClass == child->fMarkType ||
                        MarkType::kStruct == child->fMarkType ||
                        (MarkType::kEnum == child->fMarkType && !child->fAnonymous) ||
                        MarkType::kEnumClass == child->fMarkType) {
                    SkASSERT("" == substitute);
                    substitute = child->fName;
                    if (MarkType::kEnum == child->fMarkType) {
                        size_t parentClassEnd = substitute.find("::");
                        SkASSERT(string::npos != parentClassEnd);
                        string subEnd = substitute.substr(parentClassEnd + 2);
                        if (fInEnum) {
                            substitute = subEnd;
                        }
                        if (subEnd == undername) {
                            break;
                        }
                    }
                }
            }
        }
        if (!substitute.length()) {
            const Definition* parent = rootDef;
            do {
                parent = parent->fParent;
            } while (parent && (MarkType::kSubtopic == parent->fMarkType
                        || MarkType::kTopic == parent->fMarkType));
            if (parent) {
                if (MarkType::kClass == parent->fMarkType ||
                        MarkType::kStruct == parent->fMarkType ||
                        (MarkType::kEnum == parent->fMarkType && !parent->fAnonymous) ||
                        MarkType::kEnumClass == parent->fMarkType) {
                    if (parent->fParent != fRootTopic) {
                        substitute = parent->fName;
                        substitute += ' ';
                        substitute += ConvertRef(rootDef->fName, false);
                    } else {
                        size_t underpos = undername.find('_');
                        if (string::npos != underpos) {
                            string parentName = undername.substr(0, underpos);
                            string skName = "Sk" + parentName;
                            if (skName == parent->fName) {
                                SkASSERT(start >= fLastDescription->fContentStart);
                                string lastDescription = string(fLastDescription->fContentStart,
                                        (int) (start - fLastDescription->fContentStart));
                                size_t lineStart = lastDescription.rfind('\n');
                                SkASSERT(string::npos != lineStart);
                                fLine = fLastDescription->fContentStart + lineStart + 1;
                                fChar = start;
                                fEnd = end;
                                return this->reportError<string>("remove underline");
                            }
                        }
                        substitute += ConvertRef(undername, first);
                    }
                }
            }
        }
    }
    // Ensure first word after period is capitalized if substitute is lower cased.
    if (first && isupper(start[0]) && substitute.length() > 0 && islower(substitute[0])) {
        substitute[0] = start[0];
    }
    return substitute;
}

int IncludeWriter::lookupMethod(const PunctuationState punctuation, const Word word,
        const int lastSpace, const int run, int lastWrite, const char* data,
        bool hasIndirection) {
    int wordStart = lastSpace;
    while (' ' >= data[wordStart]) {
        ++wordStart;
    }
    const int wordEnd = PunctuationState::kDelimiter == punctuation ||
            PunctuationState::kParen == punctuation ||
            PunctuationState::kPeriod == punctuation ? run - 1 : run;
    string temp;
    if (hasIndirection && '(' != data[wordEnd - 1] && ')' != data[wordEnd - 1]) {
        // FIXME: hard-coded to assume a.b or a->b is a.b() or a->b().
        // need to check class a for member b to see if this is so
        TextParser parser(fFileName, &data[wordStart], &data[wordEnd], fLineCount);
        const char* indirection = parser.anyOf(".>");
        if (&data[wordEnd] <= &indirection[2] || 'f' != indirection[1] ||
                !isupper(indirection[2])) {
            temp = string(&data[wordStart], wordEnd - wordStart) + "()";
        }
    } else {
        temp = this->resolveMethod(&data[wordStart], &data[wordEnd], Word::kFirst == word);
    }
    if (temp.length()) {
        if (wordStart > lastWrite) {
            SkASSERT(data[wordStart - 1] >= ' ');
            if (' ' == data[lastWrite]) {
                this->writeSpace();
            }
            this->firstBlockTrim(wordStart - lastWrite, &data[lastWrite]);
            if (' ' == data[wordStart - 1]) {
                this->writeSpace();
            }
        }
        SkASSERT(temp[temp.length() - 1] > ' ');
        this->writeString(temp.c_str());
        lastWrite = wordEnd;
    }
    return lastWrite;
}

int IncludeWriter::lookupReference(const PunctuationState punctuation, const Word word,
        const int start, const int run, int lastWrite, const char last, const char* data) {
    const int end = PunctuationState::kDelimiter == punctuation ||
            PunctuationState::kParen == punctuation ||
            PunctuationState::kPeriod == punctuation ? run - 1 : run;
    RefType refType = RefType::kUndefined;
    string resolved = string(&data[start], (size_t) (end - start));
    string temp = this->resolveRef(&data[start], &data[end], Word::kFirst == word, &refType);
    if (!temp.length()) {
        if (Word::kFirst != word && '_' != last) {
            temp = ConvertRef(resolved, false);
        }
    }
    if (temp.length()) {
        if (start > lastWrite) {
            SkASSERT(data[start - 1] >= ' ');
            if (' ' == data[lastWrite]) {
                this->writeSpace();
            }
            this->firstBlockTrim(start - lastWrite, &data[lastWrite]);
            if (' ' == data[start - 1]) {
                this->writeSpace();
            }
        }
        SkASSERT(temp[temp.length() - 1] > ' ');
        this->writeString(temp.c_str());
        lastWrite = end;
    }
    return lastWrite;
}

/* returns true if rewriteBlock wrote linefeeds */
IncludeWriter::Wrote IncludeWriter::rewriteBlock(int size, const char* data, Phrase phrase) {
    bool wroteLineFeeds = false;
    while (size > 0 && data[0] <= ' ') {
        --size;
        ++data;
    }
    while (size > 0 && data[size - 1] <= ' ') {
        --size;
    }
    if (0 == size) {
        return Wrote::kNone;
    }
    if (fReturnOnWrite) {
        return Wrote::kChars;
    }
    int run = 0;
    Word word = Word::kStart;
    PunctuationState punctuation = Phrase::kNo == phrase ?
            PunctuationState::kStart : PunctuationState::kSpace;
    int start = 0;
    int lastWrite = 0;
    int lineFeeds = 0;
    int lastPrintable = 0;
    int lastSpace = -1;
    char c = 0;
    char last = 0;
    bool embeddedIndirection = false;
    bool embeddedSymbol = false;
    bool hasLower = false;
    bool hasUpper = false;
    bool hasIndirection = false;
    bool hasSymbol = false;
    while (run < size) {
        last = c;
        c = data[run];
        SkASSERT(' ' <= c || '\n' == c);
        if (lineFeeds && ' ' < c) {
            if (lastPrintable >= lastWrite) {
                if (' ' == data[lastWrite]) {
                    this->writeSpace();
                    lastWrite++;
                }
                this->writeBlock(lastPrintable - lastWrite + 1, &data[lastWrite]);
            }
            if (lineFeeds > 1) {
                this->lf(2);
            }
            this->lfcr(); // defer the indent until non-whitespace is seen
            lastWrite = run;
            lineFeeds = 0;
        }
        if (' ' < c) {
            lastPrintable = run;
        }
        switch (c) {
            case '\n':
                ++lineFeeds;
                wroteLineFeeds = true;
            case ' ':
                switch (word) {
                    case Word::kStart:
                        break;
                    case Word::kUnderline:
                    case Word::kCap:
                    case Word::kFirst:
                        if (!hasLower) {
                            break;
                        }
                        lastWrite = this->lookupReference(punctuation, word, start, run,
                                lastWrite, last, data);
                        break;
                    case Word::kMixed:
                        if (hasUpper && hasLower && !hasSymbol && lastSpace > 0) {
                            lastWrite = this->lookupMethod(punctuation, word, lastSpace, run,
                                    lastWrite, data, hasIndirection);
                        }
                        break;
                    default:
                        SkASSERT(0);
                }
                punctuation = PunctuationState::kPeriod == punctuation ||
                        (PunctuationState::kStart == punctuation && ' ' >= last) ?
                        PunctuationState::kStart : PunctuationState::kSpace;
                word = Word::kStart;
                embeddedIndirection = false;
                embeddedSymbol = false;
                hasLower = false;
                hasUpper = false;
                hasIndirection = false;
                hasSymbol = false;
                lastSpace = run;
                break;
            case '.': case ',': case ';': case ':': case ')':
                switch (word) {
                    case Word::kStart:
                        punctuation = PunctuationState::kDelimiter;
                    case Word::kCap:
                    case Word::kFirst:
                    case Word::kUnderline:
                    case Word::kMixed:
                        if (PunctuationState::kDelimiter == punctuation ||
                                PunctuationState::kPeriod == punctuation) {
                            word = Word::kMixed;
                        }
                        punctuation = '.' == c ? PunctuationState::kPeriod :
                                PunctuationState::kDelimiter;
                        break;
                    default:
                        SkASSERT(0);
                }
                ('.' == c ? embeddedIndirection : embeddedSymbol) = true;
                break;
            case '>':
                if ('-' == last) {
                    embeddedIndirection = true;
                    break;
                }
            case '\'': // possessive apostrophe isn't treated as delimiting punctation
            case '\"': // quote is passed straight through
            case '=':
            case '!':  // assumed not to be punctuation, but a programming symbol
            case '&': case '<': case '{': case '}': case '/': case '*': case '[': case ']':
                word = Word::kMixed;
                embeddedSymbol = true;
                break;
            case '(':
                if (' ' == last) {
                    punctuation = PunctuationState::kParen;
                } else {
                    word = Word::kMixed;
                }
                embeddedSymbol = true;
                break;
            case '_':
                switch (word) {
                    case Word::kStart:
                        word = Word::kMixed;
                        break;
                    case Word::kCap:
                    case Word::kFirst:
                    case Word::kUnderline:
                        word = Word::kUnderline;
                        break;
                    case Word::kMixed:
                        break;
                    default:
                        SkASSERT(0);
                }
                hasSymbol |= embeddedSymbol;
                break;
            case '+':
                // hackery to allow C++
                SkASSERT('C' == last || '+' == last);  // FIXME: don't allow + outside of #Formula
                break;
            case 'A': case 'B': case 'C': case 'D': case 'E':
            case 'F': case 'G': case 'H': case 'I': case 'J':
            case 'K': case 'L': case 'M': case 'N': case 'O':
            case 'P': case 'Q': case 'R': case 'S': case 'T':
            case 'U': case 'V': case 'W': case 'X': case 'Y':
            case 'Z':
                switch (word) {
                    case Word::kStart:
                        word = PunctuationState::kStart == punctuation ? Word::kFirst : Word::kCap;
                        start = run;
                        break;
                    case Word::kCap:
                    case Word::kFirst:
                        if (!isupper(last) && '~' != last) {
                            word = Word::kMixed;
                        }
                        break;
                    case Word::kUnderline:
                        // some word in Xxx_XXX_Xxx can be all upper, but all can't: XXX_XXX
                        if ('_' != last && !isupper(last)) {
                            word = Word::kMixed;
                        }
                        break;
                    case Word::kMixed:
                        break;
                    default:
                        SkASSERT(0);
                }
                hasUpper = true;
                if (PunctuationState::kPeriod == punctuation ||
                        PunctuationState::kDelimiter == punctuation) {
                    word = Word::kMixed;
                }
                hasIndirection |= embeddedIndirection;
                hasSymbol |= embeddedSymbol;
                break;
            case 'a': case 'b': case 'c': case 'd': case 'e':
            case 'f': case 'g': case 'h': case 'i': case 'j':
            case 'k': case 'l': case 'm': case 'n': case 'o':
            case 'p': case 'q': case 'r': case 's': case 't':
            case 'u': case 'v': case 'w': case 'x': case 'y':
            case 'z':
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
            case '%':  // to do : ensure that preceding is a number
            case '-':
                switch (word) {
                    case Word::kStart:
                        word = Word::kMixed;
                        break;
                    case Word::kMixed:
                    case Word::kCap:
                    case Word::kFirst:
                    case Word::kUnderline:
                        break;
                    default:
                        SkASSERT(0);
                }
                hasLower = true;
                punctuation = PunctuationState::kStart;
                hasIndirection |= embeddedIndirection;
                hasSymbol |= embeddedSymbol;
                break;
            case '~':
                SkASSERT(Word::kStart == word);
                word = PunctuationState::kStart == punctuation ? Word::kFirst : Word::kCap;
                start = run;
                hasUpper = true;
                hasIndirection |= embeddedIndirection;
                hasSymbol |= embeddedSymbol;
                break;
            default:
                SkASSERT(0);
        }
        ++run;
    }
    if ((word == Word::kCap || word == Word::kFirst || word == Word::kUnderline) && hasLower) {
        lastWrite = this->lookupReference(punctuation, word, start, run, lastWrite, last, data);
    } else if (word == Word::kMixed && hasUpper && hasLower && !hasSymbol && lastSpace > 0) {
        lastWrite = this->lookupMethod(punctuation, word, lastSpace, run, lastWrite, data,
                hasIndirection && !hasSymbol);
    }
    if (run > lastWrite) {
        if (' ' == data[lastWrite]) {
            this->writeSpace();
        }
        this->writeBlock(run - lastWrite, &data[lastWrite]);
    }
    return wroteLineFeeds ? Wrote::kLF : Wrote::kChars;
}

static string paddedString(int num) {
    auto padded = std::to_string(num);
    padded.insert(0, 2U - std::min(string::size_type(2), padded.length()), '0');
    return padded;
}

bool IncludeWriter::writeHeader(std::pair<const string, Definition>& include) {
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    time_t tt = std::chrono::system_clock::to_time_t(now);
    tm local_tm = *localtime(&tt);

    // find end of copyright header
    fChar = fStart;
    if (!this->skipExact(
            "/*\n"
            " * Copyright ")) {
        return this->reportError<bool>("copyright mismatch 1");
    }
    const char* date = fChar;
    this->skipToSpace();
    string yearStr(date, fChar - date);
    int year = stoi(yearStr);
    if (year < 2005 || year > local_tm.tm_year + 1900) {
        return this->reportError<bool>("copyright year out of range");
    }
    this->skipSpace();
    const char android[] = "The Android Open Source Project";
    const char google[] = "Google Inc.";
    if (this->startsWith(android)) {
        this->skipExact(android);
    } else if (!this->skipExact(google)) {
        return this->reportError<bool>("copyright mismatch 2");
    }
    if (!this->skipExact(
            "\n"
            " *\n"
            " * Use of this source code is governed by a BSD-style license that can be\n"
            " * found in the LICENSE file.\n"
            " */\n"
            "\n"
            )) {
        return this->reportError<bool>("copyright mismatch 2");
    }
    this->writeBlock(fChar - fStart, fStart);
    this->lf(2);
    this->writeString("/* Generated by tools/bookmaker from");
    this->writeSpace();
    string includeName = include.first;
    std::replace(includeName.begin(), includeName.end(), '\\', '/');
    this->writeString(includeName);
    this->writeSpace();
    this->writeString("and");
    this->writeSpace();
    string bmhName = fRootTopic->fFileName;
    std::replace(bmhName.begin(), bmhName.end(), '\\', '/');
    this->writeString(bmhName);
    this->lfcr();
    fIndent = 3;
    string dateTimeStr = std::to_string(local_tm.tm_year + 1900) + "-"
            + paddedString(local_tm.tm_mon + 1) + "-"
            + paddedString(local_tm.tm_mday) + " "
            + paddedString(local_tm.tm_hour) + ":"
            + paddedString(local_tm.tm_min) + ":"
            + paddedString(local_tm.tm_sec);
    this->writeString("on");
    this->writeSpace();
    this->writeString(dateTimeStr);
    this->writeString(". Additional documentation and examples can be found at:");
    this->lfcr();
    this->writeString("https://skia.org/user/api/");
    size_t bmhPageStart = bmhName.rfind('/');
    size_t bmhPageEnd = bmhName.rfind('.');
    if (string::npos == bmhPageStart || string::npos == bmhPageEnd) {
        return this->reportError<bool>("badly formed bmh page name");
    }
    ++bmhPageStart;
    string bmhPage = bmhName.substr(bmhPageStart, bmhPageEnd - bmhPageStart);
    this->writeString(bmhPage);
    this->lf(2);
    this->writeString("You may edit either file directly. Structural changes to public interfaces require");
    this->lfcr();
    this->writeString("editing both files. After editing");
    this->writeSpace();
    this->writeString(bmhName);
    this->writeSpace();
    this->writeString(", run:");
    this->lfcr();
    fIndent += 4;
    this->writeString("bookmaker -b docs -i");
    this->writeSpace();
    this->writeString(includeName);
    this->writeSpace();
    this->writeString("-p");
    this->lfcr();
    fIndent -= 4;
    this->writeString("to create an updated version of this file.");
    this->lfcr();
    fIndent = 1;
    this->writeString("*/");
    this->lf(2);
    fIndent = 0;
    if (this->startsWith("/* Generated by tools/bookmaker from")) {
        this->skipToEndBracket("*/");
        if (!this->skipExact("*/\n\n")) {
            return this->reportError<bool>("malformed generated comment");
        }
    }
    fStart = fChar;

    return true;
}
