/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bookmaker.h"

void IncludeWriter::enumHeaderOut(const RootDefinition* root,
        const Definition& child) {
    const Definition* enumDef = nullptr;
    const char* bodyEnd = fDeferComment ? fDeferComment->fContentStart - 1 :
            child.fContentStart;
    this->writeBlockTrim((int) (bodyEnd - fStart), fStart);  // may write nothing
    this->lf(2);
    fDeferComment = nullptr;
    fStart = child.fContentStart;
    const auto& nameDef = child.fTokens.front();
    string fullName;
    if (nullptr != nameDef.fContentEnd) {
        string enumName(nameDef.fContentStart,
                (int) (nameDef.fContentEnd - nameDef.fContentStart));
        fullName = root->fName + "::" + enumName;
        enumDef = root->find(enumName);
        if (!enumDef) {
            enumDef = root->find(fullName);
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
        enumDef = root->find(enumName);
        SkASSERT(enumDef);
        ++fAnonymousEnumCount;
    }
    Definition* codeBlock = nullptr;
    const char* commentStart = nullptr;
    bool wroteHeader = false;
    SkDEBUGCODE(bool foundConst = false);
    for (auto test : enumDef->fChildren) {
        if (MarkType::kCode == test->fMarkType) {
            SkASSERT(!codeBlock);  // FIXME: check enum for correct order earlier
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
            this->writeCommentHeader();
            this->writeString("\\enum");
            this->writeSpace();
            this->writeString(fullName.c_str());
            fIndent += 4;
            this->lfcr();
            wroteHeader = true;
        }
        this->rewriteBlock((int) (commentEnd - commentStart), commentStart);
        if (MarkType::kAnchor == test->fMarkType) {
            commentStart = test->fContentStart;
            commentEnd = test->fChildren[0]->fStart;
            this->writeSpace();
            this->rewriteBlock((int) (commentEnd - commentStart), commentStart);
            this->writeSpace();
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
        fIndent -= 4;
        this->lfcr();
        this->writeCommentTrailer();
    }
    bodyEnd = child.fChildren[0]->fContentStart;
    SkASSERT('{' == bodyEnd[0]);
    ++bodyEnd;
    this->lfcr();
    this->writeBlock((int) (bodyEnd - fStart), fStart); // write include "enum Name {"
    fIndent += 4;
    this->singleLF();
    fStart = bodyEnd;
    fEnumDef = enumDef;
}

void IncludeWriter::enumMembersOut(const RootDefinition* root, const Definition& child) {
    // iterate through include tokens and find how much remains for 1 line comments
    // put ones that fit on same line, ones that are too big on preceding line?
    const Definition* currentEnumItem = nullptr;
    const char* commentStart = nullptr;
    const char* lastEnd = nullptr;
    int commentLen = 0;
    enum class State {
        kNoItem,
        kItemName,
        kItemValue,
        kItemComment,
    };
    State state = State::kNoItem;
    // can't use (auto& token : child.fTokens) 'cause we need state one past end 
    auto tokenIter = child.fTokens.begin();
    for (int onePast = 0; onePast < 2; onePast += tokenIter == child.fTokens.end()) {
        const Definition* token = onePast ? nullptr : &*tokenIter++;
        if (token && Definition::Type::kBracket == token->fType) {
            if (Bracket::kSlashSlash == token->fBracket) {
                fStart = token->fContentEnd;
                continue;  // ignore old inline comments
            }
            if (Bracket::kSlashStar == token->fBracket) {
                fStart = token->fContentEnd + 1;
                continue;  // ignore old inline comments
            }
            SkASSERT(0); // incomplete
        }
        if (token && Definition::Type::kWord != token->fType) {
            SkASSERT(0); // incomplete
        }
        if (token && State::kItemName == state) {
            TextParser enumLine(token->fFileName, lastEnd,
                    token->fContentStart, token->fLineCount);
            const char* end = enumLine.anyOf(",}=");
            SkASSERT(end);
            state = '=' == *end ? State::kItemValue : State::kItemComment;
            if (State::kItemValue == state) {  // write enum value
                this->indentToColumn(fEnumItemValueTab);
                this->writeString("=");
                this->writeSpace();
                lastEnd = token->fContentEnd;
                this->writeBlock((int) (lastEnd - token->fContentStart),
                        token->fContentStart); // write const value if any
                continue;
            }
        }
        if (token && State::kItemValue == state) {
            TextParser valueEnd(token->fFileName, lastEnd,
                    token->fContentStart, token->fLineCount);
            const char* end = valueEnd.anyOf(",}");
            if (!end) {  // write expression continuation
                if (' ' == lastEnd[0]) {
                    this->writeSpace();
                }
                this->writeBlock((int) (token->fContentEnd - lastEnd), lastEnd); 
                continue;
            }
        }
        if (State::kNoItem != state) {
            this->writeString(",");
            SkASSERT(currentEnumItem);
            if (currentEnumItem->fShort) {
                this->indentToColumn(fEnumItemCommentTab);
                this->writeString("//!<");
                this->writeSpace();
                this->rewriteBlock(commentLen, commentStart);
            }
            if (onePast) {
                fIndent -= 4;
            }
            this->lfcr();
            if (token && State::kItemValue == state) {
                fStart = token->fContentStart;
            }
            state = State::kNoItem;
        }
        SkASSERT(State::kNoItem == state);
        if (onePast) {
            break;
        }
        SkASSERT(token);
        string itemName = root->fName + "::" + string(token->fContentStart,
                (int) (token->fContentEnd - token->fContentStart));
        for (auto& enumItem : fEnumDef->fChildren) {
            if (MarkType::kConst != enumItem->fMarkType) {
                continue;
            }
            if (itemName != enumItem->fName) {
                continue;
            }
            currentEnumItem = enumItem;
            break;
        }
        SkASSERT(currentEnumItem);
        // if description fits, it goes after item
        commentStart = currentEnumItem->fContentStart;
        const char* commentEnd;
        if (currentEnumItem->fChildren.size() > 0) {
            commentEnd = currentEnumItem->fChildren[0]->fStart;
        } else {
            commentEnd = currentEnumItem->fContentEnd;
        }
        TextParser enumComment(fFileName, commentStart, commentEnd, currentEnumItem->fLineCount);
        if (enumComment.skipToLineStart()) {  // skip const value
            commentStart = enumComment.fChar;
            commentLen = (int) (commentEnd - commentStart);
        } else {
            const Definition* privateDef = currentEnumItem->fChildren[0];
            SkASSERT(MarkType::kPrivate == privateDef->fMarkType);
            commentStart = privateDef->fContentStart;
            commentLen = (int) (privateDef->fContentEnd - privateDef->fContentStart);
        }
        SkASSERT(commentLen > 0 && commentLen < 1000);
        if (!currentEnumItem->fShort) {
            this->writeCommentHeader();
            fIndent += 4;
            bool wroteLineFeed = Wrote::kLF == this->rewriteBlock(commentLen, commentStart);
            fIndent -= 4;
            if (wroteLineFeed || fColumn > 100 - 3 /* space * / */ ) {
                this->lfcr();
            } else {
                this->writeSpace();
            }
            this->writeCommentTrailer();
        }
        lastEnd = token->fContentEnd;
        this->lfcr();
        if (',' == fStart[0]) {
            ++fStart;
        }
        this->writeBlock((int) (lastEnd - fStart), fStart);  // enum item name
        fStart = token->fContentEnd;
        state = State::kItemName;
    }
}

void IncludeWriter::enumSizeItems(const Definition& child) {
    enum class State {
        kNoItem,
        kItemName,
        kItemValue,
        kItemComment,
    };
    State state = State::kNoItem;
    int longestName = 0;
    int longestValue = 0;
    int valueLen = 0;
    const char* lastEnd = nullptr;
    SkASSERT(child.fChildren.size() == 1 || child.fChildren.size() == 2);
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
            SkASSERT(0); // incomplete
        }
        if (Definition::Type::kWord != token.fType) {
            SkASSERT(0); // incomplete
        }
        if (State::kItemName == state) {
            TextParser enumLine(token.fFileName, lastEnd,
                    token.fContentStart, token.fLineCount);
            const char* end = enumLine.anyOf(",}=");
            SkASSERT(end);
            state = '=' == *end ? State::kItemValue : State::kItemComment;
            if (State::kItemValue == state) {
                valueLen = (int) (token.fContentEnd - token.fContentStart);
                lastEnd = token.fContentEnd;
                continue;
            }
        }
        if (State::kItemValue == state) {
            TextParser valueEnd(token.fFileName, lastEnd,
                    token.fContentStart, token.fLineCount);
            const char* end = valueEnd.anyOf(",}");
            if (!end) {  // write expression continuation
                valueLen += (int) (token.fContentEnd - lastEnd); 
                continue;
            }
        }
        if (State::kNoItem != state) {
            longestValue = SkTMax(longestValue, valueLen);
            state = State::kNoItem;
        }
        SkASSERT(State::kNoItem == state);
        lastEnd = token.fContentEnd;
        longestName = SkTMax(longestName, (int) (lastEnd - token.fContentStart));
        state = State::kItemName;
    }
    if (State::kItemValue == state) {
        longestValue = SkTMax(longestValue, valueLen);
    }
    fEnumItemValueTab = longestName + fIndent + 1 /* space before = */ ;
    if (longestValue) {
        longestValue += 3; /* = space , */
    }
    fEnumItemCommentTab = fEnumItemValueTab + longestValue + 1 /* space before //!< */ ;
    // iterate through bmh children and see which comments fit on include lines
    for (auto& enumItem : fEnumDef->fChildren) {
        if (MarkType::kConst != enumItem->fMarkType) {
            continue;
        }
        TextParser enumLine(enumItem);
        enumLine.trimEnd();
        enumLine.skipToLineStart(); // skip const value
        const char* commentStart = enumLine.fChar;
        enumLine.skipLine();
        ptrdiff_t lineLen = enumLine.fChar - commentStart + 5 /* //!< space */ ;
        if (!enumLine.eof()) {
            enumLine.skipWhiteSpace();
        }
        enumItem->fShort = enumLine.eof() && fEnumItemCommentTab + lineLen < 100;
    }
}

// walk children and output complete method doxygen description
void IncludeWriter::methodOut(const Definition* method) {
    fContinuation = nullptr;
    fDeferComment = nullptr;
    if (0 == fIndent) {
        fIndent = 4;
    }
    this->writeCommentHeader();
    fIndent += 4;
    const char* commentStart = method->fContentStart;
    int commentLen = (int) (method->fContentEnd - commentStart);
    bool breakOut = false;
    for (auto methodProp : method->fChildren) {
        switch (methodProp->fMarkType) {
            case MarkType::kDefinedBy:
                commentStart = methodProp->fTerminator;
                break;
            case MarkType::kDeprecated: 
            case MarkType::kPrivate:
                commentLen = (int) (methodProp->fStart - commentStart);
                if (commentLen > 0) {
                    SkASSERT(commentLen < 1000);
                    if (Wrote::kNone != this->rewriteBlock(commentLen, commentStart)) {
                        this->lfcr();
                    }
                }
                commentStart = methodProp->fContentStart;
                commentLen = (int) (methodProp->fContentEnd - commentStart);
                if (commentLen > 0) {
                    if (Wrote::kNone != this->rewriteBlock(commentLen, commentStart)) {
                        this->lfcr();
                    }
                }
                commentStart = methodProp->fTerminator;
                commentLen = (int) (method->fContentEnd - commentStart);
            break;
            default:
                commentLen = (int) (methodProp->fStart - commentStart);
                breakOut = true;
        }
        if (breakOut) {
            break;
        }
    }
    SkASSERT(commentLen > 0 && commentLen < 1000);
    this->rewriteBlock(commentLen, commentStart);
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
            const char* partStart = methodPart->fContentStart;
            const char* partEnd = methodPart->fContentEnd; 
            if (MarkType::kParam == methodPart->fMarkType) {
                this->writeString("@param");
                this->writeSpace();
                this->writeString(methodPart->fName.c_str());
            } else if (MarkType::kReturn == methodPart->fMarkType) {
                this->writeString("@return");
            } else {
                continue;
            }
            while ('\n' == partEnd[-1]) {
                --partEnd;
            }
            while ('#' == partEnd[-1]) { // FIXME: so wrong; should not be before fContentEnd
                --partEnd;
            }
            this->indentToColumn(column);
            int partLen = (int) (partEnd - partStart);
            SkASSERT(partLen > 0 && partLen < 200);
            fIndent = column;
            this->rewriteBlock(partLen, partStart);
            fIndent = saveIndent;
            this->lfcr();
        }
    } else {
        this->lfcr();
    }
    fIndent -= 4;
    this->lfcr();
    this->writeCommentTrailer();
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
    this->rewriteBlock((int) (commentEnd - commentStart), commentStart);
    fIndent -= 4;
    this->lfcr();
    this->writeCommentTrailer();
}

void IncludeWriter::structMemberOut(const Definition* memberStart, const Definition& child) {
    const char* commentStart = nullptr;
    ptrdiff_t commentLen = 0;
    string name(child.fContentStart, (int) (child.fContentEnd - child.fContentStart));
    bool isShort;
    for (auto memberDef : fStructDef->fChildren)  {
        if (memberDef->fName.length() - name.length() == memberDef->fName.find(name)) {
            commentStart = memberDef->fContentStart;
            commentLen = memberDef->fContentEnd - memberDef->fContentStart;
            isShort = memberDef->fShort;
            break;
        }
    }
    if (!isShort) {
        this->writeCommentHeader();
        fIndent += 4;
        bool wroteLineFeed = Wrote::kLF == this->rewriteBlock(commentLen, commentStart);
        fIndent -= 4;
        if (wroteLineFeed || fColumn > 100 - 3 /* space * / */ ) {
            this->lfcr();
        } else {
            this->writeSpace();
        }
        this->writeCommentTrailer();
    }
    this->lfcr();
    this->writeBlock((int) (memberStart->fContentEnd - memberStart->fContentStart),
            memberStart->fContentStart);
    this->indentToColumn(fStructMemberTab);
    this->writeString(name.c_str());
    this->writeString(";");
    if (isShort) {
        this->indentToColumn(fStructCommentTab);
        this->writeString("//!<");
        this->writeSpace();
        this->rewriteBlock(commentLen, commentStart);
        this->lfcr();
    }
}

void IncludeWriter::structSizeMembers(Definition& child) {
    int longestType = 0;
    Definition* typeStart = nullptr;
    int longestName = 0;
    SkASSERT(child.fChildren.size() == 1 || child.fChildren.size() == 2);
    bool inEnum = false;
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
                case KeyWord::kUint32_t:
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
            continue;
        }
        if (Definition::Type::kWord != token.fType) {
            SkASSERT(0); // incomplete
        }
        if (MarkType::kMember == token.fMarkType) {
            TextParser typeStr(token.fFileName, typeStart->fContentStart, token.fContentStart,
                    token.fLineCount);
            typeStr.trimEnd();
            longestType = SkTMax(longestType, (int) (typeStr.fEnd - typeStart->fContentStart));
            longestName = SkTMax(longestName, (int) (token.fContentEnd - token.fContentStart));
            typeStart->fMemberStart = true;
            typeStart = nullptr;
            continue;
        }
        SkASSERT(MarkType::kNone == token.fMarkType);
        if (!typeStart) {
            typeStart = &token;
        }
    }
    fStructMemberTab = longestType + fIndent + 1 /* space before name */ ;
    fStructCommentTab = fStructMemberTab + longestName + 2 /* ; space */ ;
    // iterate through bmh children and see which comments fit on include lines
    for (auto& member : fStructDef->fChildren) {
        if (MarkType::kMember != member->fMarkType) {
            continue;
        }
        TextParser memberLine(member);
        memberLine.trimEnd();
        const char* commentStart = memberLine.fChar;
        memberLine.skipLine();
        ptrdiff_t lineLen = memberLine.fChar - commentStart + 5 /* //!< space */ ;
        if (!memberLine.eof()) {
            memberLine.skipWhiteSpace();
        }
        member->fShort = memberLine.eof() && fStructCommentTab + lineLen < 100;
    }
}

bool IncludeWriter::populate(Definition* def, RootDefinition* root) {
    // write bulk of original include up to class, method, enum, etc., excepting preceding comment
    // find associated bmh object
    // write any associated comments in Doxygen form
    // skip include comment
    // if there is a series of same named methods, write one set of comments, then write all methods
    string methodName;
    const Definition* method;
    const Definition* clonedMethod = nullptr;
    const Definition* memberStart = nullptr;
    fContinuation = nullptr;
    bool inStruct = false;
    for (auto& child : def->fTokens) {
        if (child.fPrivate) {
            continue;
        }
        if (fContinuation) {
            if (Definition::Type::kKeyWord == child.fType) {
                if (KeyWord::kFriend == child.fKeyWord || KeyWord::kBool == child.fKeyWord) {
                    continue;
                }
            }
            if (Definition::Type::kBracket == child.fType && Bracket::kParen == child.fBracket) {
                if (!clonedMethod) {
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
                    if (params.fEnd - params.fChar >= childLen &&
                            !strncmp(params.fChar, child.fContentStart, childLen)) {
                        this->methodOut(clonedMethod);
                        break;
                    }
                    ++alternate;
                    string alternateMethod = methodName + '_' + to_string(alternate);
                    clonedMethod = root->find(alternateMethod);
                } while (clonedMethod);
                if (!clonedMethod) {
                    return this->reportError<bool>("cloned method not found");
                }
                clonedMethod = nullptr;
                continue;
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
                    Punctuation::kLeftBrace == child.fPunctuation)) {
                SkASSERT(fContinuation[0] == '(');
                const char* continueEnd = child.fContentStart;
                while (continueEnd > fContinuation && isspace(continueEnd[-1])) {
                    --continueEnd;
                }
                methodName += string(fContinuation, continueEnd - fContinuation);
                method = root->find(methodName);
                if (!method) {
                    fLineCount = child.fLineCount;
                    fclose(fOut);  // so we can see what we've written so far
                    return this->reportError<bool>("method not found");
                }
                this->methodOut(method);
                continue;
            }
            methodName += "()";
            method = root->find(methodName);
            if (MarkType::kDefinedBy == method->fMarkType) {
                method = method->fParent;
            }
            if (method) {
                this->methodOut(method);
                continue;
            }
            fLineCount = child.fLineCount;
            fclose(fOut);  // so we can see what we've written so far
            return this->reportError<bool>("method not found");
        }
        if (Bracket::kSlashSlash == child.fBracket || Bracket::kSlashStar == child.fBracket) {
            if (!fDeferComment) {
                fDeferComment = &child;
            }
            continue;
        } 
        if (MarkType::kMethod == child.fMarkType) {
            const char* bodyEnd = fDeferComment ? fDeferComment->fContentStart - 1 :
                    child.fContentStart;
            // FIXME: roll end-trimming into writeBlockTrim call
            while (fStart < bodyEnd && ' ' >= bodyEnd[-1]) {
                --bodyEnd;
            }
            int blockSize = (int) (bodyEnd - fStart);
            if (blockSize) {
                this->writeBlock(blockSize, fStart);
            }
            fStart = child.fContentStart;
            methodName = root->fName + "::" + child.fName;
            fContinuation = child.fContentEnd;
            method = root->find(methodName);
            if (!method) {
                continue;
            }
            if (method->fCloned) {
                clonedMethod = method;
                continue;
            }
            this->methodOut(method);
            continue;
        } 
        if (Definition::Type::kKeyWord == child.fType) {
            const Definition* structDef = nullptr;
            switch (child.fKeyWord) {
                case KeyWord::kStruct:
                    // if struct contains members, compute their name and comment tabs
                    inStruct = fInStruct = child.fChildren.size() > 0;
                    if (fInStruct) {
                        fIndent += 4;
                        fStructDef = root->find(child.fName);
                        if (nullptr == structDef) {
                            fStructDef = root->find(root->fName + "::" + child.fName);
                        }
                        this->structSizeMembers(child);
                        fIndent -= 4;
                    }
                case KeyWord::kClass:
                    if (child.fChildren.size() > 0) {
                        const char* bodyEnd = fDeferComment ? fDeferComment->fContentStart - 1 :
                                child.fContentStart;
                        this->writeBlock((int) (bodyEnd - fStart), fStart);
                        fStart = child.fContentStart;
                        if (child.fName == root->fName) {
                            if (Definition* parent = root->fParent) {
                                if (MarkType::kTopic == parent->fMarkType ||
                                        MarkType::kSubtopic == parent->fMarkType) {
                                    const char* commentStart = parent->fContentStart;
                                    const char* commentEnd = root->fStart;
                                    this->structOut(root, *root, commentStart, commentEnd);
                                } else {
                                    SkASSERT(0); // incomplete
                                }
                            } else {
                                SkASSERT(0); // incomplete
                            }
                        } else {
                            structDef = root->find(child.fName);
                            if (nullptr == structDef) {
                                structDef = root->find(root->fName + "::" + child.fName);
                            }
                            Definition* codeBlock = nullptr;
                            Definition* nextBlock = nullptr;
                            for (auto test : structDef->fChildren) {
                                if (MarkType::kCode == test->fMarkType) {
                                    SkASSERT(!codeBlock);  // FIXME: check enum for correct order earlier
                                    codeBlock = test;
                                    continue;
                                }
                                if (codeBlock) {
                                    nextBlock = test;
                                    break;
                                }
                            }
                            SkASSERT(nextBlock);  // FIXME: check enum for correct order earlier
                            const char* commentStart = codeBlock->fTerminator;
                            const char* commentEnd = nextBlock->fStart;
                            this->structOut(root, *structDef, commentStart, commentEnd);
                        }
                        fDeferComment = nullptr;
                    } else {
                       ; // empty forward reference, nothing to do here
                    }
                    break;
                case KeyWord::kEnum: {
                    this->fInEnum = true;
                    this->enumHeaderOut(root, child);
                    this->enumSizeItems(child);
                } break;
                case KeyWord::kConst:
                case KeyWord::kConstExpr:
                case KeyWord::kStatic:
                case KeyWord::kInt:
                case KeyWord::kUint32_t:
                case KeyWord::kSize_t:
                case KeyWord::kFloat:
                case KeyWord::kBool:
                case KeyWord::kVoid:
                    if (!memberStart) {
                        memberStart = &child;
                    }
                    break;
                case KeyWord::kPublic:
                case KeyWord::kPrivate:
                case KeyWord::kProtected:
                case KeyWord::kFriend:
                    break;
                default:
                    SkASSERT(0);
            }
            if (structDef) {
                TextParser structName(&child);
                SkAssertResult(structName.skipToEndBracket('{'));
                fStart = structName.fChar + 1;
                this->writeBlock((int) (fStart - child.fStart), child.fStart);
                this->lf(2);
                fIndent += 4;
                if (!this->populate(&child, const_cast<Definition*>(structDef)->asRoot())) {
                    return false;
                }
                // output any remaining definitions at current indent level
                const char* structEnd = child.fContentEnd;
                SkAssertResult('}' == structEnd[-1]);
                --structEnd;
                this->writeBlock((int) (structEnd - fStart), fStart);
                this->lf(2);
                fStart = structEnd;
                fIndent -= 4;
                fContinuation = nullptr;
                fDeferComment = nullptr;
            } else {
                if (!this->populate(&child, root)) {
                    return false;
                }
            }
            continue;
        } 
        if (Definition::Type::kBracket == child.fType) {
            if (KeyWord::kEnum == child.fParent->fKeyWord) {
                this->enumMembersOut(root, child);
                this->writeString("};");
                this->lf(2);
                fStart = child.fParent->fContentEnd;
                SkASSERT(';' == fStart[0]);
                ++fStart;
                fDeferComment = nullptr;
                fInEnum = false;
                continue;
            } 
            fDeferComment = nullptr;
            if (!this->populate(&child, root)) {
                return false;
            }
            continue;
        }
        if (Definition::Type::kWord == child.fType) {
            if (MarkType::kMember == child.fMarkType) {
                this->structMemberOut(memberStart, child);
                fStart = child.fContentEnd + 1;
                fDeferComment = nullptr;
            }
            if (child.fMemberStart) {
                memberStart = &child;
            }
            continue;
        }
        if (Definition::Type::kPunctuation == child.fType) {
            if (Punctuation::kSemicolon == child.fPunctuation) {
                memberStart = nullptr;
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
        if (string::npos == lastSlash || lastSlash >= includeMapper.first.length() - 1) {
            return this->reportError<bool>("malformed include name");
        }
        string fileName = includeMapper.first.substr(lastSlash + 1);
        if (".h" != fileName.substr(fileName.length() - 2)) {
            return this->reportError<bool>("expected fileName.h");
        }
        string skClassName = fileName.substr(0, fileName.length() - 2);
        fOut = fopen(fileName.c_str(), "wb");
        if (!fOut) {
            SkDebugf("could not open output file %s\n", fileName.c_str());
            return false;
        }
        if (bmhParser.fClassMap.end() == bmhParser.fClassMap.find(skClassName)) {
            return this->reportError<bool>("could not find bmh class");
        }
        fBmhParser = &bmhParser;
        RootDefinition* root = &bmhParser.fClassMap[skClassName];
        fRootTopic = root->fParent;
        root->clearVisited();
        fStart = includeMapper.second.fContentStart;
        fEnd = includeMapper.second.fContentEnd;
        allPassed &= this->populate(&includeMapper.second, root);
        this->writeBlock((int) (fEnd - fStart), fStart);
        fIndent = 0;
        this->lfcr();
        this->writePending();
        fclose(fOut);
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

// FIXME: buggy that this is called with strings containing spaces but resolveRef below is not..
string IncludeWriter::resolveMethod(const char* start, const char* end, bool first) {
    string methodname(start, end - start);
    string substitute;
    auto rootDefIter = fBmhParser->fMethodMap.find(methodname);
    if (fBmhParser->fMethodMap.end() != rootDefIter) {
        substitute = methodname + "()";
    } else {
        auto parent = fRootTopic->fChildren[0]->asRoot();
        auto defRef = parent->find(parent->fName + "::" + methodname);
        if (defRef && MarkType::kMethod == defRef->fMarkType) {
            substitute = methodname + "()";
        }
    }
    return substitute;
}

string IncludeWriter::resolveRef(const char* start, const char* end, bool first) {
        // look up Xxx_Xxx 
    string undername(start, end - start);
    SkASSERT(string::npos == undername.find(' '));
    const Definition* rootDef = nullptr;
    {
        auto rootDefIter = fBmhParser->fTopicMap.find(undername);
        if (fBmhParser->fTopicMap.end() != rootDefIter) {
            rootDef = rootDefIter->second;
        } else {
            string prefixedName = fRootTopic->fName + '_' + undername;
            rootDefIter = fBmhParser->fTopicMap.find(prefixedName);
            if (fBmhParser->fTopicMap.end() != rootDefIter) {
                rootDef = rootDefIter->second;
            } else {
                auto aliasIter = fBmhParser->fAliasMap.find(undername);
                if (fBmhParser->fAliasMap.end() != aliasIter) {
                    rootDef = aliasIter->second->fParent;
                } else if (!first) {
                    for (const auto& external : fBmhParser->fExternals) {
                        if (external.fName == undername) {
                            return external.fName;
                        }
                    }
                    SkDebugf("unfound: %s\n", undername.c_str());
                }
            }
        }
    }
    string substitute;
    if (rootDef) {
        for (auto child : rootDef->fChildren) {
            if (MarkType::kSubstitute == child->fMarkType) {
                substitute = string(child->fContentStart,
                        (int) (child->fContentEnd - child->fContentStart));
                break;
            }
            if (MarkType::kClass == child->fMarkType ||
                    MarkType::kStruct == child->fMarkType ||
                    MarkType::kEnum == child->fMarkType ||
                    MarkType::kEnumClass == child->fMarkType) {
                substitute = child->fName;
                if (MarkType::kEnum == child->fMarkType && fInEnum) {
                    size_t parentClassEnd = substitute.find("::");
                    SkASSERT(string::npos != parentClassEnd);
                    substitute = substitute.substr(parentClassEnd + 2);
                }
                break;
            }
        }
        if (!substitute.length()) {
            auto parent = rootDef->fParent;
            if (parent) {
                if (MarkType::kClass == parent->fMarkType ||
                        MarkType::kStruct == parent->fMarkType ||
                        MarkType::kEnum == parent->fMarkType ||
                        MarkType::kEnumClass == parent->fMarkType) {
                    if (parent->fParent != fRootTopic) {
                        substitute = parent->fName;
                        size_t under = undername.find('_');
                        SkASSERT(string::npos != under);
                        string secondHalf(&undername[under], (size_t) (undername.length() - under));
                        substitute += ConvertRef(secondHalf, false);
                    } else {
                        substitute += ConvertRef(undername, first);
                    }
                }
            }
        }
    }
 //   start here;
    // first I thought first meant first word after period, but the below doesn't work
//    if (first && isupper(start[0]) && substitute.length() > 0 && islower(substitute[0])) {
//        substitute[0] = start[0];
//    }
    return substitute;
}
int IncludeWriter::lookupMethod(const PunctuationState punctuation, const Word word,
        const int start, const int run, int lastWrite, const char last, const char* data) {
    const int end = PunctuationState::kDelimiter == punctuation ||
            PunctuationState::kPeriod == punctuation ? run - 1 : run;
    string temp = this->resolveMethod(&data[start], &data[end], Word::kFirst == word);
    if (temp.length()) {
        if (start > lastWrite) {
            SkASSERT(data[start - 1] >= ' ');
            if (' ' == data[lastWrite]) {
                this->writeSpace();
            }
            this->writeBlockTrim(start - lastWrite, &data[lastWrite]);
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

int IncludeWriter::lookupReference(const PunctuationState punctuation, const Word word,
        const int start, const int run, int lastWrite, const char last, const char* data) {
    const int end = PunctuationState::kDelimiter == punctuation ||
            PunctuationState::kPeriod == punctuation ? run - 1 : run;
    string temp = this->resolveRef(&data[start], &data[end], Word::kFirst == word);
    if (!temp.length()) {
        if (Word::kFirst != word && '_' != last) {
            temp = string(&data[start], (size_t) (end - start));
            temp = ConvertRef(temp, false);
        }
    }                     
    if (temp.length()) {
        if (start > lastWrite) {
            SkASSERT(data[start - 1] >= ' ');
            if (' ' == data[lastWrite]) {
                this->writeSpace();
            }
            this->writeBlockTrim(start - lastWrite, &data[lastWrite]);
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
IncludeWriter::Wrote IncludeWriter::rewriteBlock(int size, const char* data) {
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
    int run = 0;
    Word word = Word::kStart;
    PunctuationState punctuation = PunctuationState::kStart;
    int start = 0;
    int lastWrite = 0;
    int lineFeeds = 0;
    int lastPrintable = 0;
    char c = 0;
    char last;
    bool hasLower = false;
    bool hasUpper = false;
    bool hasSymbol = false;
    while (run < size) {
        last = c;
        c = data[run];
        SkASSERT(' ' <= c || '\n' == c);
        if (lineFeeds && ' ' < c) {
            if (lastPrintable >= lastWrite) {
                if (' ' == data[lastWrite]) {
                    this->writeSpace();
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
                        if (hasUpper && hasLower && !hasSymbol) {
                            lastWrite = this->lookupMethod(punctuation, word, start, run,
                                    lastWrite, last, data);
                        }
                        break;
                    default:
                        SkASSERT(0);
                }
                punctuation = PunctuationState::kStart;
                word = Word::kStart;
                hasLower = false;
                hasUpper = false;
                hasSymbol = false;
                break;
            case '.':
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
                        punctuation = PunctuationState::kPeriod;
                        break;
                    default:
                        SkASSERT(0);
                }
                hasSymbol = true;
                break;
            case ',': case ';': case ':':
                hasSymbol |= PunctuationState::kDelimiter == punctuation;
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
                        punctuation = PunctuationState::kDelimiter;
                        break;
                    default:
                        SkASSERT(0);
                }
                break;
            case '\'': // possessive apostrophe isn't treated as delimiting punctation
            case '=':
            case '!':  // assumed not to be punctuation, but a programming symbol
            case '&': case '>': case '<': case '{': case '}': case '/': case '*':
                word = Word::kMixed;
                hasSymbol = true;
                break;
            case '(':
                if (' ' == last) {
                    punctuation = PunctuationState::kDelimiter;
                } else {
                    word = Word::kMixed;
                }
                hasSymbol = true;
                break;
            case ')':   // assume word type has already been set
                punctuation = PunctuationState::kDelimiter;
                hasSymbol = true;
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
                        if (!isupper(last)) {
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
                } else {
                    punctuation = PunctuationState::kStart;
                }
                break;
            case 'a': case 'b': case 'c': case 'd': case 'e':
            case 'f': case 'g': case 'h': case 'i': case 'j':
            case 'k': case 'l': case 'm': case 'n': case 'o':
            case 'p': case 'q': case 'r': case 's': case 't':
            case 'u': case 'v': case 'w': case 'x': case 'y':
            case 'z': 
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
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
                break;
            default:
                SkASSERT(0);
        }
        ++run;
    }
    if ((word == Word::kCap || word == Word::kFirst || word == Word::kUnderline) && hasLower) {
        lastWrite = this->lookupReference(punctuation, word, start, run, lastWrite, last, data);
    }
    if (run > lastWrite) {
        if (' ' == data[lastWrite]) {
            this->writeSpace();
        }
        this->writeBlock(run - lastWrite, &data[lastWrite]);
    }
    return wroteLineFeeds ? Wrote::kLF : Wrote::kChars;
}
