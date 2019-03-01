/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bmhParser.h"

const string kSpellingFileName("spelling.txt");

#define M(mt) (1LL << (int) MarkType::k##mt)
#define M_D M(Description)
#define M_CS M(Class) | M(Struct)
#define M_MD M(Method) | M(Define)
#define M_MDCM M_MD | M(Const) | M(Member)
#define M_ST M(Subtopic) | M(Topic)
#define M_CSST M_CS | M_ST
#ifdef M_E
#undef M_E
#endif
#define M_E M(Enum) | M(EnumClass)

#define R_Y Resolvable::kYes
#define R_N Resolvable::kNo
#define R_O Resolvable::kOut
#define R_K Resolvable::kCode
#define R_F Resolvable::kFormula
#define R_C Resolvable::kClone

#define E_Y Exemplary::kYes
#define E_N Exemplary::kNo
#define E_O Exemplary::kOptional

// ToDo: add column to denote which marks are one-liners
BmhParser::MarkProps BmhParser::kMarkProps[] = {
// names without formal definitions (e.g. Column) aren't included
  { "",             MarkType::kNone,         R_Y, E_N, 0 }
, { "A",            MarkType::kAnchor,       R_N, E_N, 0 }
, { "Alias",        MarkType::kAlias,        R_N, E_N, M_ST | M(Const) }
, { "Bug",          MarkType::kBug,          R_N, E_N, M_CSST | M_MDCM | M_E
                                                     | M(Example) | M(NoExample) }
, { "Class",        MarkType::kClass,        R_Y, E_O, M_CSST }
, { "Code",         MarkType::kCode,         R_K, E_N, M_CSST | M_E | M_MD | M(Typedef) }
, { "",             MarkType::kColumn,       R_Y, E_N, M(Row) }
, { "",             MarkType::kComment,      R_N, E_N, 0 }
, { "Const",        MarkType::kConst,        R_Y, E_O, M_E | M_CSST  }
, { "Define",       MarkType::kDefine,       R_O, E_Y, M_ST }
, { "Description",  MarkType::kDescription,  R_Y, E_N, M(Example) | M(NoExample) }
, { "Details",      MarkType::kDetails,      R_N, E_N, M(Const) }
, { "Duration",     MarkType::kDuration,     R_N, E_N, M(Example) | M(NoExample) }
, { "Enum",         MarkType::kEnum,         R_Y, E_O, M_CSST }
, { "EnumClass",    MarkType::kEnumClass,    R_Y, E_O, M_CSST }
, { "Example",      MarkType::kExample,      R_O, E_N, M_CSST | M_E | M_MD | M(Const) }
, { "External",     MarkType::kExternal,     R_Y, E_N, 0 }
, { "File",         MarkType::kFile,         R_Y, E_N, M(Topic) }
, { "Filter",       MarkType::kFilter,       R_N, E_N, M(Subtopic) | M(Code) }
, { "Formula",      MarkType::kFormula,      R_F, E_N, M(Column) | M(Description)
                                                     | M_E | M_ST | M_MDCM }
, { "Function",     MarkType::kFunction,     R_O, E_N, M(Example) | M(NoExample) }
, { "Height",       MarkType::kHeight,       R_N, E_N, M(Example) | M(NoExample) }
, { "Illustration", MarkType::kIllustration, R_N, E_N, M_CSST | M_MD }
, { "Image",        MarkType::kImage,        R_N, E_N, M(Example) | M(NoExample) }
, { "In",           MarkType::kIn,           R_N, E_N, M_CSST | M_E | M(Method) | M(Typedef) | M(Code) }
, { "Legend",       MarkType::kLegend,       R_Y, E_N, M(Table) }
, { "Line",         MarkType::kLine,         R_N, E_N, M_CSST | M_E | M(Method) | M(Typedef) }
, { "",             MarkType::kLink,         R_N, E_N, M(Anchor) }
, { "List",         MarkType::kList,         R_Y, E_N, M(Method) | M_CSST | M_E | M_D }
, { "Literal",      MarkType::kLiteral,      R_N, E_N, M(Code) }
, { "",             MarkType::kMarkChar,     R_N, E_N, 0 }
, { "Member",       MarkType::kMember,       R_Y, E_O, M_CSST }
, { "Method",       MarkType::kMethod,       R_Y, E_Y, M_CSST }
, { "NoExample",    MarkType::kNoExample,    R_N, E_N, M_CSST | M_E | M_MD }
, { "NoJustify",    MarkType::kNoJustify,    R_N, E_N, M(Const) | M(Member) }
, { "Outdent",      MarkType::kOutdent,      R_N, E_N, M(Code) }
, { "Param",        MarkType::kParam,        R_Y, E_N, M(Method) | M(Define) }
, { "PhraseDef",    MarkType::kPhraseDef,    R_Y, E_N, M_ST }
, { "",             MarkType::kPhraseParam,  R_Y, E_N, 0 }
, { "",             MarkType::kPhraseRef,    R_N, E_N, 0 }
, { "Platform",     MarkType::kPlatform,     R_N, E_N, M(Example) | M(NoExample) }
, { "Populate",     MarkType::kPopulate,     R_N, E_N, M(Code) | M(Method) }
, { "Return",       MarkType::kReturn,       R_Y, E_N, M(Method) }
, { "",             MarkType::kRow,          R_Y, E_N, M(Table) | M(List) }
, { "SeeAlso",      MarkType::kSeeAlso,      R_C, E_N, M_CSST | M_E | M_MD | M(Typedef) }
, { "Set",          MarkType::kSet,          R_N, E_N, M(Example) | M(NoExample) }
, { "StdOut",       MarkType::kStdOut,       R_N, E_N, M(Example) | M(NoExample) }
, { "Struct",       MarkType::kStruct,       R_Y, E_O, M(Class) | M_ST }
, { "Substitute",   MarkType::kSubstitute,   R_N, E_N, M(Alias) | M_ST }
, { "Subtopic",     MarkType::kSubtopic,     R_Y, E_Y, M_CSST | M_E }
, { "Table",        MarkType::kTable,        R_Y, E_N, M(Method) | M_CSST | M_E }
, { "Template",     MarkType::kTemplate,     R_Y, E_N, M_CSST }
, { "",             MarkType::kText,         R_N, E_N, 0 }
, { "ToDo",         MarkType::kToDo,         R_N, E_N, 0 }
, { "Topic",        MarkType::kTopic,        R_Y, E_Y, 0 }
, { "Typedef",      MarkType::kTypedef,      R_Y, E_O, M_CSST | M_E }
, { "Union",        MarkType::kUnion,        R_Y, E_N, M_CSST }
, { "Using",        MarkType::kUsing,        R_Y, E_O, M_CSST }
, { "Volatile",     MarkType::kVolatile,     R_N, E_N, M(StdOut) }
, { "Width",        MarkType::kWidth,        R_N, E_N, M(Example) | M(NoExample) }
};

#undef R_O
#undef R_N
#undef R_Y
#undef R_K
#undef R_F
#undef R_C

#undef M_E
#undef M_CSST
#undef M_ST
#undef M_CS
#undef M_MCD
#undef M_D
#undef M

#undef E_Y
#undef E_N
#undef E_O

bool BmhParser::addDefinition(const char* defStart, bool hasEnd, MarkType markType,
        const vector<string>& typeNameBuilder, HasTag hasTag) {
    Definition* definition = nullptr;
    switch (markType) {
        case MarkType::kComment:
            if (!this->skipToDefinitionEnd(markType)) {
                return false;
            }
            return true;
        // these types may be referred to by name
        case MarkType::kClass:
        case MarkType::kStruct:
        case MarkType::kConst:
        case MarkType::kDefine:
        case MarkType::kEnum:
        case MarkType::kEnumClass:
        case MarkType::kMember:
        case MarkType::kMethod:
        case MarkType::kTemplate:
        case MarkType::kTypedef: {
            if (!typeNameBuilder.size()) {
                return this->reportError<bool>("unnamed markup");
            }
            if (typeNameBuilder.size() > 1) {
                return this->reportError<bool>("expected one name only");
            }
            string name = typeNameBuilder[0];
            if (nullptr == fRoot) {
                fRoot = this->findBmhObject(markType, name);
                fRoot->fFileName = fFileName;
                fRoot->fName = name;
                fRoot->fNames.fName = name;
                fRoot->fNames.fParent = &fGlobalNames;
                definition = fRoot;
            } else {
                if (nullptr == fParent) {
                    return this->reportError<bool>("expected parent");
                }
                if (fParent == fRoot && hasEnd) {
                    RootDefinition* rootParent = fRoot->rootParent();
                    if (rootParent) {
                        fRoot = rootParent;
                    }
                    definition = fParent;
                } else {
                    if (!hasEnd && fRoot->find(name, RootDefinition::AllowParens::kNo)) {
                        return this->reportError<bool>("duplicate symbol");
                    }
                    if (MarkType::kStruct == markType || MarkType::kClass == markType
                            || MarkType::kEnumClass == markType) {
                        // if class or struct, build fRoot hierarchy
                        // and change isDefined to search all parents of fRoot
                        SkASSERT(!hasEnd);
                        RootDefinition* childRoot = new RootDefinition;
                        (fRoot->fBranches)[name] = childRoot;
                        childRoot->setRootParent(fRoot);
                        childRoot->fFileName = fFileName;
                        SkASSERT(MarkType::kSubtopic != fRoot->fMarkType
                                && MarkType::kTopic != fRoot->fMarkType);
                        childRoot->fNames.fName = name;
                        childRoot->fNames.fParent = &fRoot->fNames;
                        fRoot = childRoot;
                        definition = fRoot;
                    } else {
                        definition = &fRoot->fLeaves[name];
                    }
                }
            }
            if (hasEnd) {
                Exemplary hasExample = Exemplary::kNo;
                bool hasExcluder = false;
                for (auto child : definition->fChildren) {
                     if (MarkType::kExample == child->fMarkType) {
                        hasExample = Exemplary::kYes;
                     }
                     hasExcluder |= MarkType::kNoExample == child->fMarkType;
                }
                if (kMarkProps[(int) markType].fExemplary != hasExample
                        && kMarkProps[(int) markType].fExemplary != Exemplary::kOptional) {
                    if (string::npos == fFileName.find("undocumented")
                            && !hasExcluder) {
                        hasExample == Exemplary::kNo ?
                                this->reportWarning("missing example") :
                                this->reportWarning("unexpected example");
                    }

                }
                if (MarkType::kMethod == markType) {
                    if (fCheckMethods && !definition->checkMethod()) {
                        return false;
                    }
                }
                if (HasTag::kYes == hasTag) {
                    if (!this->checkEndMarker(markType, definition->fName)) {
                        return false;
                    }
                }
                if (!this->popParentStack(definition)) {
                    return false;
                }
                if (fRoot == definition) {
                    fRoot = nullptr;
                }
            } else {
                definition->fStart = defStart;
                this->skipSpace();
                definition->fFileName = fFileName;
                definition->fContentStart = fChar;
                definition->fLineCount = fLineCount;
                definition->fClone = fCloned;
                if (MarkType::kConst == markType) {
                    // todo: require that fChar points to def on same line as markup
                    // additionally add definition to class children if it is not already there
                    if (definition->fParent != fRoot) {
//                        fRoot->fChildren.push_back(definition);
                    }
                }
                SkASSERT(string::npos == name.find('\n'));
                definition->fName = name;
                if (MarkType::kMethod == markType) {
                    if (string::npos != name.find(':', 0)) {
                        definition->setCanonicalFiddle();
                    } else {
                        definition->fFiddle = name;
                    }
                } else {
                    definition->fFiddle = Definition::NormalizedName(name);
                }
                definition->fMarkType = markType;
                definition->fAnonymous = fAnonymous;
                this->setAsParent(definition);
            }
            } break;
        case MarkType::kTopic:
        case MarkType::kSubtopic:
            SkASSERT(1 == typeNameBuilder.size());
            if (!hasEnd) {
                if (!typeNameBuilder.size()) {
                    return this->reportError<bool>("unnamed topic");
                }
                fTopics.emplace_front(markType, defStart, fLineCount, fParent, fMC);
                RootDefinition* rootDefinition = &fTopics.front();
                definition = rootDefinition;
                definition->fFileName = fFileName;
                definition->fContentStart = fChar;
                if (MarkType::kTopic == markType) {
                    if (fParent) {
                        return this->reportError<bool>("#Topic must be root");
                    }
                    // topic name is unappended
                    definition->fName = typeNameBuilder[0];
                } else {
                    if (!fParent) {
                        return this->reportError<bool>("#Subtopic may not be root");
                    }
                    Definition* parent = fParent;
                    while (MarkType::kTopic != parent->fMarkType && MarkType::kSubtopic != parent->fMarkType) {
                        parent = parent->fParent;
                        if (!parent) {
                            // subtopic must have subtopic or topic in parent chain
                            return this->reportError<bool>("#Subtopic missing parent");
                        }
                    }
                    if (MarkType::kSubtopic == parent->fMarkType) {
                        // subtopic prepends parent subtopic name, but not parent topic name
                        definition->fName = parent->fName + '_';
                    }
                    definition->fName += typeNameBuilder[0];
                    definition->fFiddle = parent->fFiddle + '_';
                }
                rootDefinition->fNames.fName = rootDefinition->fName;
                rootDefinition->fNames.fParent = &fGlobalNames;
                definition->fFiddle += Definition::NormalizedName(typeNameBuilder[0]);
                this->setAsParent(definition);
            }
            {
                SkASSERT(hasEnd ? fParent : definition);
                string fullTopic = hasEnd ? fParent->fFiddle : definition->fFiddle;
                Definition* defPtr = fTopicMap[fullTopic];
                if (hasEnd) {
                    if (HasTag::kYes == hasTag && !this->checkEndMarker(markType, fullTopic)) {
                        return false;
                    }
                    if (!definition) {
                        definition = defPtr;
                    } else if (definition != defPtr) {
                        return this->reportError<bool>("mismatched topic");
                    }
                } else {
                    if (nullptr != defPtr) {
                        return this->reportError<bool>("already declared topic");
                    }
                    fTopicMap[fullTopic] = definition;
                }
            }
            if (hasEnd) {
                if (!this->popParentStack(definition)) {
                    return false;
                }
            }
            break;
        case MarkType::kFormula:
            // hasEnd : single line / multiple line
            if (!fParent || MarkType::kFormula != fParent->fMarkType) {
                SkASSERT(!definition || MarkType::kFormula == definition->fMarkType);
                fMarkup.emplace_front(markType, defStart, fLineCount, fParent, fMC);
                definition = &fMarkup.front();
                definition->fContentStart = fChar;
                definition->fName = typeNameBuilder[0];
                definition->fFiddle = fParent->fFiddle;
                fParent = definition;
            } else {
                SkASSERT(fParent && MarkType::kFormula == fParent->fMarkType);
                SkASSERT(fMC == defStart[0]);
                SkASSERT(fMC == defStart[-1]);
                definition = fParent;
                definition->fTerminator = fChar;
                if (!this->popParentStack(definition)) {
                    return false;
                }
                this->parseHashFormula(definition);
                fParent->fChildren.push_back(definition);
            }
            break;
        // these types are children of parents, but are not in named maps
        case MarkType::kDescription:
        case MarkType::kStdOut:
        // may be one-liner
        case MarkType::kAlias:
        case MarkType::kNoExample:
        case MarkType::kParam:
        case MarkType::kPhraseDef:
        case MarkType::kReturn:
        case MarkType::kToDo:
            if (hasEnd) {
                if (markType == fParent->fMarkType) {
                    definition = fParent;
                    if (MarkType::kBug == markType || MarkType::kReturn == markType
                            || MarkType::kToDo == markType) {
                        this->skipNoName();
                    }
                    if (!this->popParentStack(fParent)) { // if not one liner, pop
                        return false;
                    }
                    if (MarkType::kParam == markType || MarkType::kReturn == markType
                            || MarkType::kPhraseDef == markType) {
                        if (!this->checkParamReturn(definition)) {
                            return false;
                        }
                    }
                    if (MarkType::kPhraseDef == markType) {
                        string key = definition->fName;
                        if (fPhraseMap.end() != fPhraseMap.find(key)) {
                            this->reportError<bool>("duplicate phrase key");
                        }
                        fPhraseMap[key] = definition;
                    }
                } else {
                    fMarkup.emplace_front(markType, defStart, fLineCount, fParent, fMC);
                    definition = &fMarkup.front();
                    definition->fName = typeNameBuilder[0];
                    definition->fFiddle = fParent->fFiddle;
                    definition->fContentStart = fChar;
                    string endBracket;
                    endBracket += fMC;
                    endBracket += fMC;
                    definition->fContentEnd = this->trimmedBracketEnd(endBracket);
                    this->skipToEndBracket(endBracket.c_str());
                    SkAssertResult(fMC == this->next());
                    SkAssertResult(fMC == this->next());
                    definition->fTerminator = fChar;
                    TextParser checkForChildren(definition);
                    if (checkForChildren.strnchr(fMC, definition->fContentEnd)) {
                        this->reportError<bool>("put ## on separate line");
                    }
                    fParent->fChildren.push_back(definition);
                }
                if (MarkType::kAlias == markType) {
                    const char* end = definition->fChildren.size() > 0 ?
                            definition->fChildren[0]->fStart : definition->fContentEnd;
                    TextParser parser(definition->fFileName, definition->fContentStart, end,
                            definition->fLineCount);
                    parser.trimEnd();
                    string key = string(parser.fStart, parser.lineLength());
                    if (fAliasMap.end() != fAliasMap.find(key)) {
                        return this->reportError<bool>("duplicate alias");
                    }
                    fAliasMap[key] = definition;
                    definition->fFiddle = definition->fParent->fFiddle;
                }
                break;
            } else if (MarkType::kPhraseDef == markType) {
                bool hasParams = '(' == this->next();
                fMarkup.emplace_front(markType, defStart, fLineCount, fParent, fMC);
                definition = &fMarkup.front();
                definition->fName = typeNameBuilder[0];
                definition->fFiddle = fParent->fFiddle;
                definition->fContentStart = fChar;
                if (hasParams) {
                    char lastChar;
                    do {
                        const char* subEnd = this->anyOf(",)\n");
                        if (!subEnd || '\n' == *subEnd) {
                            return this->reportError<bool>("unexpected phrase list end");
                        }
                        fMarkup.emplace_front(MarkType::kPhraseParam, fChar, fLineCount, fParent,
                                fMC);
                        Definition* phraseParam = &fMarkup.front();
                        phraseParam->fContentStart = fChar;
                        phraseParam->fContentEnd = subEnd;
                        phraseParam->fName = string(fChar, subEnd - fChar);
                        definition->fChildren.push_back(phraseParam);
                        this->skipTo(subEnd);
                        lastChar = this->next();
                        phraseParam->fTerminator = fChar;
                    } while (')' != lastChar);
                    this->skipWhiteSpace();
                    definition->fContentStart = fChar;
                }
                this->setAsParent(definition);
                break;
            }
        // not one-liners
        case MarkType::kCode:
        case MarkType::kExample:
        case MarkType::kFile:
        case MarkType::kFunction:
        case MarkType::kLegend:
        case MarkType::kList:
        case MarkType::kTable:
            if (hasEnd) {
                definition = fParent;
                if (markType != fParent->fMarkType) {
                    return this->reportError<bool>("end element mismatch");
                } else if (!this->popParentStack(fParent)) {
                    return false;
                }
                if (MarkType::kExample == markType) {
                    if (definition->fChildren.size() == 0) {
                        TextParser emptyCheck(definition);
                        if (emptyCheck.eof() || !emptyCheck.skipWhiteSpace()) {
                            return this->reportError<bool>("missing example body");
                        }
                    }
// can't do this here; phrase refs may not have been defined yet
//                    this->setWrapper(definition);
                }
            } else {
                fMarkup.emplace_front(markType, defStart, fLineCount, fParent, fMC);
                definition = &fMarkup.front();
                definition->fContentStart = fChar;
                definition->fName = typeNameBuilder[0];
                definition->fFiddle = fParent->fFiddle;
                char suffix = '\0';
                bool tryAgain;
                do {
                    tryAgain = false;
                    for (const auto& child : fParent->fChildren) {
                        if (child->fFiddle == definition->fFiddle) {
                            if (MarkType::kExample != child->fMarkType) {
                                continue;
                            }
                            if ('\0' == suffix) {
                                suffix = 'a';
                            } else if (++suffix > 'z') {
                                return reportError<bool>("too many examples");
                            }
                            definition->fFiddle = fParent->fFiddle + '_';
                            definition->fFiddle += suffix;
                            tryAgain = true;
                            break;
                        }
                    }
                } while (tryAgain);
                this->setAsParent(definition);
            }
            break;
            // always treated as one-liners (can't detect misuse easily)
        case MarkType::kAnchor:
        case MarkType::kBug:
        case MarkType::kDetails:
        case MarkType::kDuration:
        case MarkType::kFilter:
        case MarkType::kHeight:
        case MarkType::kIllustration:
        case MarkType::kImage:
		case MarkType::kIn:
		case MarkType::kLine:
		case MarkType::kLiteral:
        case MarkType::kNoJustify:
        case MarkType::kOutdent:
        case MarkType::kPlatform:
        case MarkType::kPopulate:
        case MarkType::kSeeAlso:
        case MarkType::kSet:
        case MarkType::kSubstitute:
        case MarkType::kVolatile:
        case MarkType::kWidth:
            // todo : add check disallowing children?
            if (hasEnd && MarkType::kAnchor != markType && MarkType::kLine != markType) {
                return this->reportError<bool>("one liners omit end element");
            } else if (!hasEnd && MarkType::kAnchor == markType) {
                return this->reportError<bool>("anchor line must have end element last");
            }
            fMarkup.emplace_front(markType, defStart, fLineCount, fParent, fMC);
            definition = &fMarkup.front();
            definition->fName = typeNameBuilder[0];
            definition->fFiddle = Definition::NormalizedName(typeNameBuilder[0]);
            definition->fContentStart = fChar;
            definition->fContentEnd = this->trimmedBracketEnd('\n');
            definition->fTerminator = this->lineEnd() - 1;
            fParent->fChildren.push_back(definition);
            if (MarkType::kAnchor == markType) {
                this->parseHashAnchor(definition);
			} else if (MarkType::kLine == markType) {
                this->parseHashLine(definition);
			}
            break;
        case MarkType::kExternal:
            (void) this->collectExternals();  // FIXME: detect errors in external defs?
            break;
        default:
            SkASSERT(0);  // fixme : don't let any types be invisible
            return true;
    }
    if (fParent) {
        SkASSERT(definition);
        SkASSERT(definition->fName.length() > 0);
    }
    return true;
}

void BmhParser::reportDuplicates(const Definition& def, string dup) const {
    if (MarkType::kExample == def.fMarkType && dup == def.fFiddle) {
        TextParser reporter(&def);
        reporter.reportError("duplicate example name");
    }
    for (auto& child : def.fChildren ) {
        reportDuplicates(*child, dup);
    }
}


static Definition* find_fiddle(Definition* def, string name) {
    if (MarkType::kExample == def->fMarkType && name == def->fFiddle) {
        return def;
    }
    for (auto& child : def->fChildren) {
        Definition* result = find_fiddle(child, name);
        if (result) {
            return result;
        }
    }
    return nullptr;
}

Definition* BmhParser::findExample(string name) const {
    for (const auto& topic : fTopicMap) {
        if (topic.second->fParent) {
            continue;
        }
        Definition* def = find_fiddle(topic.second, name);
        if (def) {
            return def;
        }
    }
    return nullptr;
}

static bool check_example_hashes(Definition* def) {
    if (MarkType::kExample == def->fMarkType) {
        if (def->fHash.length()) {
            return true;
        }
        for (auto child : def->fChildren) {
            if (MarkType::kPlatform == child->fMarkType) {
                if (string::npos != string(child->fContentStart, child->length()).find("!fiddle")) {
                    return true;
                }
            }
        }
        return def->reportError<bool>("missing hash");
    }
    for (auto& child : def->fChildren) {
        if (!check_example_hashes(child)) {
            return false;
        }
    }
    return true;
}

bool BmhParser::checkExampleHashes() const {
    for (const auto& topic : fTopicMap) {
        if (!topic.second->fParent && !check_example_hashes(topic.second)) {
            return false;
        }
    }
    return true;
}

static void reset_example_hashes(Definition* def) {
    if (MarkType::kExample == def->fMarkType) {
        def->fHash.clear();
        return;
    }
    for (auto& child : def->fChildren) {
        reset_example_hashes(child);
    }
}

void BmhParser::resetExampleHashes() {
    for (const auto& topic : fTopicMap) {
        if (!topic.second->fParent) {
            reset_example_hashes(topic.second);
        }
    }
}

static void find_examples(const Definition& def, vector<string>* exampleNames) {
    if (MarkType::kExample == def.fMarkType) {
        exampleNames->push_back(def.fFiddle);
    }
    for (auto& child : def.fChildren ) {
        find_examples(*child, exampleNames);
    }
}

bool BmhParser::checkEndMarker(MarkType markType, string match) const {
    TextParser tp(fFileName, fLine, fChar, fLineCount);
    tp.skipSpace();
    if (fMC != tp.next()) {
        return this->reportError<bool>("mismatched end marker expect #");
    }
    const char* nameStart = tp.fChar;
    tp.skipToNonName();
    string markName(nameStart, tp.fChar - nameStart);
    if (kMarkProps[(int) markType].fName != markName) {
        return this->reportError<bool>("expected #XXX ## to match");
    }
    tp.skipSpace();
    nameStart = tp.fChar;
    tp.skipToNonName();
    markName = string(nameStart, tp.fChar - nameStart);
    if ("" == markName) {
        if (fMC != tp.next() || fMC != tp.next()) {
            return this->reportError<bool>("expected ##");
        }
        return true;
    }
    std::replace(markName.begin(), markName.end(), '-', '_');
    auto defPos = match.rfind(markName);
    if (string::npos == defPos) {
        return this->reportError<bool>("mismatched end marker v1");
    }
    if (markName.size() != match.size() - defPos) {
        return this->reportError<bool>("mismatched end marker v2");
    }
    return true;
}

bool BmhParser::checkExamples() const {
    vector<string> exampleNames;
    for (const auto& topic : fTopicMap) {
        if (topic.second->fParent) {
            continue;
        }
        find_examples(*topic.second, &exampleNames);
    }
    std::sort(exampleNames.begin(), exampleNames.end());
    string* last = nullptr;
    string reported;
    bool checkOK = true;
    for (auto& nameIter : exampleNames) {
        if (last && *last == nameIter && reported != *last) {
            reported = *last;
            SkDebugf("%s\n", reported.c_str());
            for (const auto& topic : fTopicMap) {
                if (topic.second->fParent) {
                    continue;
                }
                this->reportDuplicates(*topic.second, reported);
            }
            checkOK = false;
        }
        last = &nameIter;
    }
    return checkOK;
}

bool BmhParser::checkParamReturn(const Definition* definition) const {
    const char* parmEndCheck = definition->fContentEnd;
    while (parmEndCheck < definition->fTerminator) {
        if (fMC == parmEndCheck[0]) {
            break;
        }
        if (' ' < parmEndCheck[0]) {
            this->reportError<bool>(
                    "use full end marker on multiline #Param and #Return");
        }
        ++parmEndCheck;
    }
    return true;
}

bool BmhParser::childOf(MarkType markType) const {
    auto childError = [this](MarkType markType) -> bool {
        string errStr = "expected ";
        errStr += kMarkProps[(int) markType].fName;
        errStr += " parent";
        return this->reportError<bool>(errStr.c_str());
    };

    if (markType == fParent->fMarkType) {
        return true;
    }
    if (this->hasEndToken()) {
        if (!fParent->fParent) {
            return this->reportError<bool>("expected grandparent");
        }
        if (markType == fParent->fParent->fMarkType) {
            return true;
        }
    }
    return childError(markType);
}

string BmhParser::className(MarkType markType) {
    const char* end = this->lineEnd();
    const char* mc = this->strnchr(fMC, end);
    string classID;
    TextParserSave savePlace(this);
    this->skipSpace();
    const char* wordStart = fChar;
    this->skipToNonName();
    const char* wordEnd = fChar;
    classID = string(wordStart, wordEnd - wordStart);
    if (!mc) {
        savePlace.restore();
    }
    string builder;
    const Definition* parent = this->parentSpace();
    if (parent && parent->fName != classID) {
        builder += parent->fName;
    }
    if (mc) {
        if (mc + 1 < fEnd && fMC == mc[1]) {  // if ##
            if (markType != fParent->fMarkType) {
                return this->reportError<string>("unbalanced method");
            }
            if (builder.length() > 0 && classID.size() > 0) {
                if (builder != fParent->fName) {
                    builder += "::";
                    builder += classID;
                    if (builder != fParent->fName) {
                        return this->reportError<string>("name mismatch");
                    }
                }
            }
            this->skipLine();
            return fParent->fName;
        } else if (' ' ==  mc[1] && MarkType::kConst == markType && fParent
                && (MarkType::kEnum == fParent->fMarkType
                || MarkType::kEnumClass == fParent->fMarkType)) {
            this->skipToEndBracket('\n');
            return builder + "::" + string(wordStart, wordEnd - wordStart);
        }
        fChar = mc;
        this->next();
    }
    this->skipWhiteSpace();
    if (MarkType::kEnum == markType && fChar >= end) {
        fAnonymous = true;
        builder += "::_anonymous";
        return uniqueRootName(builder, markType);
    }
    builder = this->word(builder, "::");
    return builder;
}

bool BmhParser::collectExternals() {
    do {
        this->skipWhiteSpace();
        if (this->eof()) {
            break;
        }
        if (fMC == this->peek()) {
            this->next();
            if (this->eof()) {
                break;
            }
            if (fMC == this->peek()) {
                this->skipLine();
                break;
            }
            if (' ' >= this->peek()) {
                this->skipLine();
                continue;
            }
            if (this->startsWith(kMarkProps[(int) MarkType::kExternal].fName)) {
                this->skipToNonName();
                continue;
            }
        }
        this->skipToAlpha();
        const char* wordStart = fChar;
        this->skipToWhiteSpace();
        if (fChar - wordStart > 0) {
            fExternals.emplace_front(MarkType::kExternal, wordStart, fChar, fLineCount, fParent,
                    fMC);
            RootDefinition* definition = &fExternals.front();
            definition->fFileName = fFileName;
            definition->fName = string(wordStart ,fChar - wordStart);
            definition->fFiddle = Definition::NormalizedName(definition->fName);
        }
    } while (!this->eof());
    return true;
}

bool BmhParser::dumpExamples(FILE* fiddleOut, Definition& def, bool* continuation) const {
    if (MarkType::kExample == def.fMarkType) {
        string result;
        if (!this->exampleToScript(&def, BmhParser::ExampleOptions::kAll, &result)) {
            return false;
        }
        if (result.length() > 0) {
            result += "\n";
            result += "}";
            if (*continuation) {
                fprintf(fiddleOut, ",\n");
            } else {
                *continuation = true;
            }
            fprintf(fiddleOut, "%s", result.c_str());
        }
        return true;
    }
    for (auto& child : def.fChildren ) {
        if (!this->dumpExamples(fiddleOut, *child, continuation)) {
            return false;
        }
    }
    return true;
}

bool BmhParser::dumpExamples(const char* fiddleJsonFileName) const {
    string oldFiddle(fiddleJsonFileName);
    string newFiddle(fiddleJsonFileName);
    newFiddle += "_new";
    FILE* fiddleOut = fopen(newFiddle.c_str(), "wb");
    if (!fiddleOut) {
        SkDebugf("could not open output file %s\n", newFiddle.c_str());
        return false;
    }
    fprintf(fiddleOut, "{\n");
    bool continuation = false;
    for (const auto& topic : fTopicMap) {
        if (topic.second->fParent) {
            continue;
        }
        this->dumpExamples(fiddleOut, *topic.second, &continuation);
    }
    fprintf(fiddleOut, "\n}\n");
    fclose(fiddleOut);
    if (ParserCommon::WrittenFileDiffers(oldFiddle, newFiddle)) {
        ParserCommon::CopyToFile(oldFiddle, newFiddle);
        SkDebugf("wrote %s\n", fiddleJsonFileName);
    } else {
        remove(newFiddle.c_str());
    }
    return true;
}

int BmhParser::endHashCount() const {
    const char* end = fLine + this->lineLength();
    int count = 0;
    while (fLine < end && fMC == *--end) {
        count++;
    }
    return count;
}

bool BmhParser::endTableColumn(const char* end, const char* terminator) {
    if (!this->popParentStack(fParent)) {
        return false;
    }
    fWorkingColumn->fContentEnd = end;
    fWorkingColumn->fTerminator = terminator;
    fColStart = fChar - 1;
    this->skipSpace();
    fTableState = TableState::kColumnStart;
    return true;
}

static size_t count_indent(string text, size_t test, size_t end) {
    size_t result = test;
    while (test < end) {
        if (' ' != text[test]) {
            break;
        }
        ++test;
    }
    return test - result;
}

static void add_code(string text, int pos, int end,
    size_t outIndent, size_t textIndent, string& example) {
    do {
        // fix this to move whole paragraph in, out, but preserve doc indent
        int nextIndent = count_indent(text, pos, end);
        size_t len = text.find('\n', pos);
        if (string::npos == len) {
            len = end;
        }
        if ((size_t) (pos + nextIndent) < len) {
            size_t indent = outIndent + nextIndent;
            SkASSERT(indent >= textIndent);
            indent -= textIndent;
            for (size_t index = 0; index < indent; ++index) {
                example += ' ';
            }
            pos += nextIndent;
            while ((size_t) pos < len) {
                example += '"' == text[pos] ? "\\\"" :
                    '\\' == text[pos] ? "\\\\" :
                    text.substr(pos, 1);
                ++pos;
            }
            example += "\\n";
        } else {
            pos += nextIndent;
        }
        if ('\n' == text[pos]) {
            ++pos;
        }
    } while (pos < end);
}

bool BmhParser::IsExemplary(const Definition* def) {
    return kMarkProps[(int) def->fMarkType].fExemplary != Exemplary::kNo;
}

bool BmhParser::exampleToScript(Definition* def, ExampleOptions exampleOptions,
        string* result) const {
    bool hasFiddle = true;
    const Definition* platform = def->hasChild(MarkType::kPlatform);
    if (platform) {
        TextParser platParse(platform);
        hasFiddle = !platParse.strnstr("!fiddle", platParse.fEnd);
    }
    if (!hasFiddle) {
        *result = "";
        return true;
    }
    string text = this->extractText(def, TrimExtract::kNo);
    bool textOut = string::npos != text.find("SkDebugf(")
        || string::npos != text.find("dump(")
        || string::npos != text.find("dumpHex(");
    string heightStr = "256";
    string widthStr = "256";
    string normalizedName(def->fFiddle);
    string code;
    string imageStr = "0";
    string srgbStr = "false";
    string durationStr = "0";
    for (auto iter : def->fChildren) {
        switch (iter->fMarkType) {
        case MarkType::kDuration:
            durationStr = string(iter->fContentStart, iter->fContentEnd - iter->fContentStart);
            break;
        case MarkType::kHeight:
            heightStr = string(iter->fContentStart, iter->fContentEnd - iter->fContentStart);
            break;
        case MarkType::kWidth:
            widthStr = string(iter->fContentStart, iter->fContentEnd - iter->fContentStart);
            break;
        case MarkType::kDescription:
            // ignore for now
            break;
        case MarkType::kFunction: {
            // emit this, but don't wrap this in draw()
            string funcText = this->extractText(&*iter, TrimExtract::kNo);
            size_t pos = 0;
            while (pos < funcText.length() && ' ' > funcText[pos]) {
                ++pos;
            }
            size_t indent = count_indent(funcText, pos, funcText.length());
            add_code(funcText, pos, funcText.length(), 0, indent, code);
            code += "\\n";
        } break;
        case MarkType::kComment:
            break;
        case MarkType::kImage:
            imageStr = string(iter->fContentStart, iter->fContentEnd - iter->fContentStart);
            break;
        case MarkType::kToDo:
            break;
        case MarkType::kBug:
        case MarkType::kMarkChar:
        case MarkType::kPlatform:
        case MarkType::kPhraseRef:
            // ignore for now
            break;
        case MarkType::kSet:
            if ("sRGB" == string(iter->fContentStart,
                iter->fContentEnd - iter->fContentStart)) {
                srgbStr = "true";
            } else {
                SkASSERT(0);   // more work to do
                return false;
            }
            break;
        case MarkType::kStdOut:
            textOut = true;
            break;
        default:
            SkASSERT(0);  // more coding to do
        }
    }
    string animatedStr = "0" != durationStr ? "true" : "false";
    string textOutStr = textOut ? "true" : "false";
    size_t pos = 0;
    while (pos < text.length() && ' ' > text[pos]) {
        ++pos;
    }
    size_t end = text.length();
    size_t outIndent = 0;
    size_t textIndent = count_indent(text, pos, end);
    if ("" == def->fWrapper) {
        this->setWrapper(def);
    }
    if (def->fWrapper.length() > 0) {
        code += def->fWrapper;
        code += "\\n";
        outIndent = 4;
    }
    add_code(text, pos, end, outIndent, textIndent, code);
    if (def->fWrapper.length() > 0) {
        code += "}";
    }
    string example = "\"" + normalizedName + "\": {\n";
    string filename = def->fileName();
    string baseFile = filename.substr(0, filename.length() - 4);
    if (ExampleOptions::kText == exampleOptions) {
        example += "    \"code\": \"" + code + "\",\n";
        example += "    \"hash\": \"" + def->fHash + "\",\n";
        example += "    \"file\": \"" + baseFile + "\",\n";
        example += "    \"name\": \"" + def->fName + "\",";
    } else {
        example += "    \"code\": \"" + code + "\",\n";
        if (ExampleOptions::kPng == exampleOptions) {
            example += "    \"width\": " + widthStr + ",\n";
            example += "    \"height\": " + heightStr + ",\n";
            example += "    \"hash\": \"" + def->fHash + "\",\n";
            example += "    \"file\": \"" + baseFile + "\",\n";
            example += "    \"name\": \"" + def->fName + "\"\n";
            example += "}";
        } else {
            example += "    \"options\": {\n";
            example += "        \"width\": " + widthStr + ",\n";
            example += "        \"height\": " + heightStr + ",\n";
            example += "        \"source\": " + imageStr + ",\n";
            example += "        \"srgb\": " + srgbStr + ",\n";
            example += "        \"f16\": false,\n";
            example += "        \"textOnly\": " + textOutStr + ",\n";
            example += "        \"animated\": " + animatedStr + ",\n";
            example += "        \"duration\": " + durationStr + "\n";
            example += "    },\n";
            example += "    \"fast\": true";
        }
    }
    *result = example;
    return true;
}

string BmhParser::extractText(const Definition* def, TrimExtract trimExtract) const {
    string result;
    TextParser parser(def);
    auto childIter = def->fChildren.begin();
    while (!parser.eof()) {
        const char* end = def->fChildren.end() == childIter ? parser.fEnd : (*childIter)->fStart;
        string fragment(parser.fChar, end - parser.fChar);
        trim_end(fragment);
        if (TrimExtract::kYes == trimExtract) {
            trim_start(fragment);
            if (result.length()) {
                result += '\n';
                result += '\n';
            }
        }
        if (TrimExtract::kYes == trimExtract || has_nonwhitespace(fragment)) {
            result += fragment;
        }
        parser.skipTo(end);
        if (def->fChildren.end() != childIter) {
            Definition* child = *childIter;
            if (MarkType::kPhraseRef == child->fMarkType) {
                auto phraseIter = fPhraseMap.find(child->fName);
                if (fPhraseMap.end() == phraseIter) {
                    return def->reportError<string>("missing phrase definition");
                }
                Definition* phrase = phraseIter->second;
                // count indent of last line in result
                size_t lastLF = result.rfind('\n');
                size_t startPos = string::npos == lastLF ? 0 : lastLF;
                size_t lastLen = result.length() - startPos;
                size_t indent = count_indent(result, startPos, result.length()) + 4;
                string phraseStr = this->extractText(phrase, TrimExtract::kNo);
                startPos = 0;
                bool firstTime = true;
                size_t endPos;
                do {
                    endPos = phraseStr.find('\n', startPos);
                    size_t len = (string::npos != endPos ? endPos : phraseStr.length()) - startPos;
                    if (firstTime && lastLen + len + 1 < 100) {  // FIXME: make 100 global const or something
                        result += ' ';
                    } else {
                        result += '\n';
                        result += string(indent, ' ');
                    }
                    firstTime = false;
                    string tmp = phraseStr.substr(startPos, len);
                    result += tmp;
                    startPos = endPos + 1;
                } while (string::npos != endPos);
                result += '\n';
            }
            parser.skipTo(child->fTerminator);
            std::advance(childIter, 1);
        }
    }
    return result;
}

string BmhParser::loweredTopic(string name, Definition* def) {
    string lowered;
    SkASSERT('_' != name[0]);
    char last = '_';
    for (char c : name) {
        SkASSERT(' ' != c);
        if (isupper(last)) {
            lowered += islower(c) ? tolower(last) : last;
            last = '\0';
        }
        if ('_' == c) {
            last = c;
            c = ' ';
        } else if ('_' == last && isupper(c)) {
            last = c;
            continue;
        }
        lowered += c;
        if (' ' == c) {
            this->setUpPartialSubstitute(lowered);
        }
    }
    if (isupper(last)) {
        lowered += tolower(last);
    }
    return lowered;
}

void BmhParser::setUpGlobalSubstitutes() {
    for (auto& entry : fExternals) {
        string externalName = entry.fName;
        SkASSERT(fGlobalNames.fRefMap.end() == fGlobalNames.fRefMap.find(externalName));
        fGlobalNames.fRefMap[externalName] = nullptr;
    }
    for (auto bMap : { &fClassMap, &fConstMap, &fDefineMap, &fEnumMap, &fMethodMap,
            &fTypedefMap } ) {
        for (auto& entry : *bMap) {
            Definition* parent = &entry.second;
            string name = parent->fName;
            SkASSERT(fGlobalNames.fLinkMap.end() == fGlobalNames.fLinkMap.find(name));
            string ref = ParserCommon::HtmlFileName(parent->fFileName) + '#' + parent->fFiddle;
            fGlobalNames.fLinkMap[name] = ref;
            SkASSERT(fGlobalNames.fRefMap.end() == fGlobalNames.fRefMap.find(name));
            fGlobalNames.fRefMap[name] = parent;
            NameMap* names = MarkType::kClass == parent->fMarkType
                    || MarkType::kStruct == parent->fMarkType
                    || MarkType::kEnumClass == parent->fMarkType ? &parent->asRoot()->fNames :
                    &fGlobalNames;
            this->setUpSubstitutes(parent, names);
            if (names != &fGlobalNames) {
                names->copyToParent(&fGlobalNames);
            }
        }
    }
    for (auto& tEntry : fTypedefMap) {
        Definition* typeDef = &tEntry.second;
        string defName = typeDef->fName;
        TextParser parser(typeDef->fFileName, typeDef->fStart, typeDef->fContentStart,
                typeDef->fLineCount);
        parser.skipExact("#Typedef");
        parser.skipWhiteSpace();
        const char* refStart = parser.fChar;
        parser.skipToWhiteSpace();
        string refName(refStart, parser.fChar - refStart);
        auto structIter = fClassMap.find(refName);
        if (fClassMap.end() == structIter) {
            continue;
        }
        for (auto& sEntry : structIter->second.fLeaves) {
            Definition* def = &sEntry.second;
            string name = def->fName;
            size_t colonPos = name.find("::");
            SkASSERT(string::npos != colonPos);
            string typedName = defName + "::" + name.substr(colonPos + 2);
            string ref = ParserCommon::HtmlFileName(def->fFileName) + '#' + def->fFiddle;
            SkASSERT(fGlobalNames.fLinkMap.end() == fGlobalNames.fLinkMap.find(typedName));
            fGlobalNames.fLinkMap[typedName] = ref;
            SkASSERT(fGlobalNames.fRefMap.end() == fGlobalNames.fRefMap.find(typedName));
            fGlobalNames.fRefMap[typedName] = def;
        }
    }
    for (auto& topic : fTopicMap) {
        bool hasSubstitute = false;
        for (auto& child : topic.second->fChildren) {
            bool isAlias = MarkType::kAlias == child->fMarkType;
            bool isSubstitute = MarkType::kSubstitute == child->fMarkType;
            if (!isAlias && !isSubstitute) {
                continue;
            }
            hasSubstitute |= isSubstitute;
            string name(child->fContentStart, child->length());
            if (isAlias) {
                name = ParserCommon::ConvertRef(name, false);
                for (auto aliasChild : child->fChildren) {
                    if (MarkType::kSubstitute == aliasChild->fMarkType) {
                        string sub(aliasChild->fContentStart, aliasChild->length());
                        this->setUpSubstitute(sub, topic.second);
                    }
                }
            }
            this->setUpSubstitute(name, topic.second);
        }
        if (hasSubstitute) {
            continue;
        }
        string lowered = this->loweredTopic(topic.first, topic.second);
        SkDEBUGCODE(auto globalIter = fGlobalNames.fLinkMap.find(lowered));
        SkASSERT(fGlobalNames.fLinkMap.end() == globalIter);
        fGlobalNames.fLinkMap[lowered] =
                ParserCommon::HtmlFileName(topic.second->fFileName) + '#' + topic.first;
        SkASSERT(fGlobalNames.fRefMap.end() == fGlobalNames.fRefMap.find(lowered));
        fGlobalNames.fRefMap[lowered] = topic.second;
    }
    size_t slash = fRawFilePathDir.rfind('/');
    size_t bslash = fRawFilePathDir.rfind('\\');
    string spellFile;
    if (string::npos == slash && string::npos == bslash) {
        spellFile = fRawFilePathDir;
    } else {
        if (string::npos != bslash && bslash > slash) {
            slash = bslash;
        }
        spellFile = fRawFilePathDir.substr(0, slash);
    }
    spellFile += '/';
    spellFile += kSpellingFileName;
    FILE* file = fopen(spellFile.c_str(), "r");
    if (!file) {
        SkDebugf("missing %s\n", spellFile.c_str());
        return;
    }
    fseek(file, 0L, SEEK_END);
    int sz = (int) ftell(file);
    rewind(file);
    char* buffer = new char[sz];
    memset(buffer, ' ', sz);
    int read = (int)fread(buffer, 1, sz, file);
    SkAssertResult(read > 0);
    sz = read;  // FIXME: ? why are sz and read different?
    fclose(file);
    int i = 0;
    int start = i;
    string last = " ";
    string word;
    do {
        if (' ' < buffer[i]) {
            ++i;
            continue;
        }
        last = word;
        word = string(&buffer[start], i - start);
#ifdef SK_DEBUG
        SkASSERT(last.compare(word) < 0);
        for (char c : word) {
            SkASSERT(islower(c) || '-' == c);
        }
#endif
        if (fGlobalNames.fRefMap.end() == fGlobalNames.fRefMap.find(word)) {
            fGlobalNames.fRefMap[word] = nullptr;
        } else {
            SkDebugf("%s ", word.c_str());  // debugging: word missing from spelling list
        }
        do {
            ++i;
        } while (i < sz && ' ' >= buffer[i]);
        start = i;
    } while (i < sz);
    delete[] buffer;
}

void BmhParser::setUpSubstitutes(const Definition* parent, NameMap* names) {
    for (const auto& child : parent->fChildren) {
        MarkType markType = child->fMarkType;
        if (MarkType::kAlias == markType) {
            continue;
        }
        if (MarkType::kSubstitute == markType) {
            continue;
        }
        string name(child->fName);
        if (&fGlobalNames != names) {
            size_t lastDC = name.rfind("::");
            if (string::npos != lastDC) {
                name = name.substr(lastDC + 2);
            }
            if ("" == name) {
                continue;
            }
        }
        string ref;
        if (&fGlobalNames == names) {
            ref = ParserCommon::HtmlFileName(child->fFileName);
        }
        ref += '#' + child->fFiddle;
        if (MarkType::kClass == markType || MarkType::kStruct == markType
                || (MarkType::kMethod == markType && !child->fClone)
                || MarkType::kEnum == markType
                || MarkType::kEnumClass == markType || MarkType::kConst == markType
                || MarkType::kMember == markType || MarkType::kDefine == markType
                || MarkType::kTypedef == markType) {
            SkASSERT(names->fLinkMap.end() == names->fLinkMap.find(name));
            names->fLinkMap[name] = ref;
            SkASSERT(names->fRefMap.end() == names->fRefMap.find(name));
            names->fRefMap[name] = child;
        }
        if (MarkType::kClass == markType || MarkType::kStruct == markType
                || MarkType::kEnumClass == markType) {
            RootDefinition* rootDef = child->asRoot();
            NameMap* nameMap = &rootDef->fNames;
            this->setUpSubstitutes(child, nameMap);
            nameMap->copyToParent(names);
        }
        if (MarkType::kEnum == markType) {
            this->setUpSubstitutes(child, names);
        }
        if (MarkType::kMethod == markType) {
            if (child->fClone || child->fCloned) {
                TextParser parser(child->fFileName, child->fStart, child->fContentStart,
                        child->fLineCount);
                parser.skipExact("#Method");
                parser.skipSpace();
                string name = child->methodName();
                const char* nameInParser = parser.strnstr(name.c_str(), parser.fEnd);
                parser.skipTo(nameInParser);
                const char* paren = parser.strnchr('(', parser.fEnd);
                parser.skipTo(paren);
                parser.skipToBalancedEndBracket('(', ')');
                if ("()" != string(paren, parser.fChar - paren)) {
                    string fullName =
                            trim_inline_spaces(string(nameInParser, parser.fChar - nameInParser));
                    SkASSERT(names->fLinkMap.end() == names->fLinkMap.find(fullName));
                    names->fLinkMap[fullName] = ref;
                    SkASSERT(names->fRefMap.end() == names->fRefMap.find(fullName));
                    names->fRefMap[fullName] = child;
                }
            }
        }
        if (MarkType::kSubtopic == markType) {
            if (&fGlobalNames != names && string::npos != child->fName.find('_')) {
                string lowered = this->loweredTopic(child->fName, child);
                SkDEBUGCODE(auto refIter = names->fRefMap.find(lowered));
                SkDEBUGCODE(auto iter = names->fLinkMap.find(lowered));
                SkASSERT(names->fLinkMap.end() == iter);
                names->fLinkMap[lowered] = '#' + child->fName;
                SkASSERT(names->fRefMap.end() == refIter);
                names->fRefMap[lowered] = child;
            }
            this->setUpSubstitutes(child, names);
        }
    }
}

void BmhParser::setUpPartialSubstitute(string name) {
    auto iter = fGlobalNames.fRefMap.find(name);
    if (fGlobalNames.fRefMap.end() != iter) {
        SkASSERT(nullptr == iter->second);
        return;
    }
    fGlobalNames.fRefMap[name] = nullptr;
}

void BmhParser::setUpSubstitute(string name, Definition* def) {
    SkASSERT(fGlobalNames.fRefMap.end() == fGlobalNames.fRefMap.find(name));
    fGlobalNames.fRefMap[name] = def;
    SkASSERT(fGlobalNames.fLinkMap.end() == fGlobalNames.fLinkMap.find(name));
    string str = ParserCommon::HtmlFileName(def->fFileName) + '#' + def->fName;
    fGlobalNames.fLinkMap[name] = str;
    size_t stop = name.length();
    do {
        size_t space = name.rfind(' ', stop);
        if (string::npos == space) {
            break;
        }
        string partial = name.substr(0, space + 1);
        stop = space - 1;
        this->setUpPartialSubstitute(partial);
    } while (true);
}

void BmhParser::setWrapper(Definition* def) const {
    const char drawWrapper[] = "void draw(SkCanvas* canvas) {";
    const char drawNoCanvas[] = "void draw(SkCanvas* ) {";
    string text = this->extractText(def, TrimExtract::kNo);
    size_t nonSpace = 0;
    while (nonSpace < text.length() && ' ' >= text[nonSpace]) {
        ++nonSpace;
    }
    bool hasFunc = !text.compare(nonSpace, sizeof(drawWrapper) - 1, drawWrapper);
    bool noCanvas = !text.compare(nonSpace, sizeof(drawNoCanvas) - 1, drawNoCanvas);
    bool hasCanvas = string::npos != text.find("SkCanvas canvas");
    SkASSERT(!hasFunc || !noCanvas);
    bool preprocessor = text[0] == '#';
    bool wrapCode = !hasFunc && !noCanvas && !preprocessor;
    if (wrapCode) {
        def->fWrapper = hasCanvas ? string(drawNoCanvas) : string(drawWrapper);
    }
}

RootDefinition* BmhParser::findBmhObject(MarkType markType, string typeName) {
    const auto& mapIter = std::find_if(fMaps.begin(), fMaps.end(),
            [markType](DefinitionMap& defMap){ return markType == defMap.fMarkType; } );
    if (mapIter == fMaps.end()) {
        return nullptr;
    }
    return &(*mapIter->fMap)[typeName];
}

// FIXME: some examples may produce different output on different platforms
// if the text output can be different, think of how to author that

bool BmhParser::findDefinitions() {
    bool lineStart = true;
    const char* lastChar = nullptr;
    const char* lastMC = nullptr;
    fParent = nullptr;
    while (!this->eof()) {
        if (this->peek() == fMC) {
            lastMC = fChar;
            this->next();
            if (this->peek() == fMC) {
                this->next();
                if (!lineStart && ' ' < this->peek()) {
                    if (!fParent || MarkType::kFormula != fParent->fMarkType) {
                        return this->reportError<bool>("expected definition");
                    }
                }
                if (this->peek() != fMC) {
                    if (MarkType::kColumn == fParent->fMarkType) {
                        SkASSERT(TableState::kColumnEnd == fTableState);
                        if (!this->endTableColumn(lastChar, lastMC)) {
                            return false;
                        }
                        SkASSERT(fRow);
                        if (!this->popParentStack(fParent)) {
                            return false;
                        }
                        fRow->fContentEnd = fWorkingColumn->fContentEnd;
                        fWorkingColumn = nullptr;
                        fRow = nullptr;
                        fTableState = TableState::kNone;
                    } else {
                        vector<string> parentName;
                        parentName.push_back(fParent->fName);
                        if (!this->addDefinition(fChar - 1, true, fParent->fMarkType, parentName,
                                HasTag::kNo)) {
                            return false;
                        }
                    }
                } else {
                    SkAssertResult(this->next() == fMC);
                    fMC = this->next();  // change markup character
                    if (' ' >= fMC) {
                        return this->reportError<bool>("illegal markup character");
                    }
                    fMarkup.emplace_front(MarkType::kMarkChar, fChar - 4, fLineCount, fParent, fMC);
                    Definition* markChar = &fMarkup.front();
                    markChar->fContentStart = fChar - 1;
                    this->skipToEndBracket('\n');
                    markChar->fContentEnd = fChar;
                    markChar->fTerminator = fChar;
                    fParent->fChildren.push_back(markChar);
                }
            } else if (this->peek() >= 'A' && this->peek() <= 'Z') {
                const char* defStart = fChar - 1;
                MarkType markType = this->getMarkType(MarkLookup::kRequire);
                bool hasEnd = this->hasEndToken();
                if (!hasEnd && fParent) {
                    MarkType parentType = fParent->fMarkType;
                    uint64_t parentMask = kMarkProps[(int) markType].fParentMask;
                    if (parentMask && !(parentMask & (1LL << (int) parentType))) {
                        return this->reportError<bool>("invalid parent");
                    }
                }
                if (!this->skipName(kMarkProps[(int) markType].fName)) {
                    return this->reportError<bool>("illegal markup character");
                }
                if (!this->skipSpace()) {
                    return this->reportError<bool>("unexpected end");
                }
                lineStart = '\n' == this->peek();
                bool expectEnd = true;
                vector<string> typeNameBuilder = this->typeName(markType, &expectEnd);
                if (fCloned && MarkType::kMethod != markType && MarkType::kExample != markType
                        && !fAnonymous) {
                    return this->reportError<bool>("duplicate name");
                }
                if (hasEnd && expectEnd) {
                    if (fMC == this->peek()) {
                        return this->reportError<bool>("missing body");
                    }
                }
                if (!this->addDefinition(defStart, hasEnd, markType, typeNameBuilder,
                        HasTag::kYes)) {
                    return false;
                }
                continue;
            } else if (this->peek() == ' ') {
                if (!fParent || (MarkType::kFormula != fParent->fMarkType
                        && MarkType::kLegend != fParent->fMarkType
                        && MarkType::kList != fParent->fMarkType
						&& MarkType::kLine != fParent->fMarkType
                        && MarkType::kTable != fParent->fMarkType)) {
                    int endHashes = this->endHashCount();
                    if (endHashes <= 1) {
                        if (fParent) {
                            if (TableState::kColumnEnd == fTableState) {
                                if (!this->endTableColumn(lastChar, lastMC)) {
                                    return false;
                                }
                            } else {  // one line comment
                                fMarkup.emplace_front(MarkType::kComment, fChar - 1, fLineCount,
                                        fParent, fMC);
                                Definition* comment = &fMarkup.front();
                                comment->fContentStart = fChar - 1;
                                this->skipToEndBracket('\n');
                                comment->fContentEnd = fChar;
                                comment->fTerminator = fChar;
                                fParent->fChildren.push_back(comment);
                            }
                        } else {
                            fChar = fLine + this->lineLength() - 1;
                        }
                    } else {  // table row
                        if (2 != endHashes) {
                            string errorStr = "expect ";
                            errorStr += fMC;
                            errorStr += fMC;
                            return this->reportError<bool>(errorStr.c_str());
                        }
                        if (!fParent || MarkType::kTable != fParent->fMarkType) {
                            return this->reportError<bool>("missing table");
                        }
                    }
                } else if (TableState::kNone == fTableState) {
                    // fixme? no nested tables for now
                    fColStart = fChar - 1;
                    fMarkup.emplace_front(MarkType::kRow, fColStart, fLineCount, fParent, fMC);
                    fRow = &fMarkup.front();
                    fRow->fName = fParent->fName;
                    this->skipWhiteSpace();
                    fRow->fContentStart = fChar;
                    this->setAsParent(fRow);
                    fTableState = TableState::kColumnStart;
                }
                if (TableState::kColumnStart == fTableState) {
                    fMarkup.emplace_front(MarkType::kColumn, fColStart, fLineCount, fParent, fMC);
                    fWorkingColumn = &fMarkup.front();
                    fWorkingColumn->fName = fParent->fName;
                    fWorkingColumn->fContentStart = fChar;
                    this->setAsParent(fWorkingColumn);
                    fTableState = TableState::kColumnEnd;
                    continue;
                }
            } else if (this->peek() >= 'a' && this->peek() <= 'z') {
                // expect zero or more letters and underscores (no spaces) then hash
                const char* phraseNameStart = fChar;
                this->skipPhraseName();
                string phraseKey = string(phraseNameStart, fChar - phraseNameStart);
                char delimiter = this->next();
                vector<string> params;
                vector<const char*> paramsLoc;
                if (fMC != delimiter) {
                    if ('(' != delimiter) {
                        return this->reportError<bool>("expect # after phrase name");
                    }
                    // phrase may take comma delimited parameter list
                    do {
                        const char* subEnd = this->anyOf(",)\n");
                        if (!subEnd || '\n' == *subEnd) {
                            return this->reportError<bool>("unexpected phrase list end");
                        }
                        params.push_back(string(fChar, subEnd - fChar));
                        paramsLoc.push_back(fChar);
                        this->skipTo(subEnd);

                    } while (')' != this->next());
                }
                const char* start = phraseNameStart;
                SkASSERT('#' == start[-1]);
                --start;
                if (start > fStart && ' ' >= start[-1]) {
                    --start;  // preserve whether to add whitespace before substitution
                }
                fMarkup.emplace_front(MarkType::kPhraseRef, start, fLineCount, fParent, fMC);
                Definition* markChar = &fMarkup.front();
                this->skipExact("#");
                markChar->fContentStart = fChar;
                markChar->fContentEnd = fChar;
                markChar->fTerminator = fChar;
                markChar->fName = phraseKey;
                fParent->fChildren.push_back(markChar);
                int paramLocIndex = 0;
                for (auto param : params) {
                    const char* paramLoc = paramsLoc[paramLocIndex++];
                    fMarkup.emplace_front(MarkType::kPhraseParam, paramLoc, fLineCount, fParent,
                            fMC);
                    Definition* phraseParam = &fMarkup.front();
                    phraseParam->fContentStart = paramLoc;
                    phraseParam->fContentEnd = paramLoc + param.length();
                    phraseParam->fTerminator = paramLoc + param.length();
                    phraseParam->fName = param;
                    markChar->fChildren.push_back(phraseParam);
                }
            }
        }
        char nextChar = this->next();
        if (' ' < nextChar) {
            lastChar = fChar;
            lineStart = false;
        } else if (nextChar == '\n') {
            lineStart = true;
        }
    }
    if (fParent) {
        return fParent->reportError<bool>("mismatched end");
    }
    return true;
}

MarkType BmhParser::getMarkType(MarkLookup lookup) const {
    for (int index = 0; index <= Last_MarkType; ++index) {
        int typeLen = strlen(kMarkProps[index].fName);
        if (typeLen == 0) {
            continue;
        }
        if (fChar + typeLen >= fEnd || fChar[typeLen] > ' ') {
            continue;
        }
        int chCompare = strncmp(fChar, kMarkProps[index].fName, typeLen);
        if (chCompare < 0) {
            goto fail;
        }
        if (chCompare == 0) {
            return (MarkType) index;
        }
    }
fail:
    if (MarkLookup::kRequire == lookup) {
        return this->reportError<MarkType>("unknown mark type");
    }
    return MarkType::kNone;
}

bool BmhParser::hasEndToken() const {
    const char* ptr = fLine;
    char test;
    do {
        if (ptr >= fEnd) {
            return false;
        }
        test = *ptr++;
        if ('\n' == test) {
            return false;
        }
    } while (fMC != test || fMC != *ptr);
    return true;
}

string BmhParser::memberName() {
    const char* wordStart;
    const char* prefixes[] = { "static", "const" };
    do {
        this->skipSpace();
        wordStart = fChar;
        this->skipToNonName();
    } while (this->anyOf(wordStart, prefixes, SK_ARRAY_COUNT(prefixes)));
    if ('*' == this->peek()) {
        this->next();
    }
    return this->className(MarkType::kMember);
}

string BmhParser::methodName() {
    if (this->hasEndToken()) {
        if (!fParent || !fParent->fName.length()) {
            return this->reportError<string>("missing parent method name");
        }
        SkASSERT(fMC == this->peek());
        this->next();
        SkASSERT(fMC == this->peek());
        this->next();
        SkASSERT(fMC != this->peek());
        return fParent->fName;
    }
    string builder;
    const char* end = this->lineEnd();
    const char* paren = this->strnchr('(', end);
    if (!paren) {
        return this->reportError<string>("missing method name and reference");
    }
    {
        TextParserSave endCheck(this);
        while (end < fEnd && !this->strnchr(')', end)) {
            fChar = end + 1;
            end = this->lineEnd();
        }
        if (end >= fEnd) {
            return this->reportError<string>("missing method end paren");
        }
        endCheck.restore();
    }
    const char* nameStart = paren;
    char ch;
    bool expectOperator = false;
    bool isConstructor = false;
    const char* nameEnd = nullptr;
    while (nameStart > fChar && ' ' != (ch = *--nameStart)) {
        if (!isalnum(ch) && '_' != ch) {
            if (nameEnd) {
                break;
            }
            expectOperator = true;
            continue;
        }
        if (!nameEnd) {
            nameEnd = nameStart + 1;
        }
    }
    if (!nameEnd) {
         return this->reportError<string>("unexpected method name char");
    }
    if (' ' == nameStart[0]) {
        ++nameStart;
    }
    if (nameEnd <= nameStart) {
        return this->reportError<string>("missing method name");
    }
    if (nameStart >= paren) {
        return this->reportError<string>("missing method name length");
    }
    string name(nameStart, nameEnd - nameStart);
    bool allLower = true;
    for (int index = 0; index < (int) (nameEnd - nameStart); ++index) {
        if (!islower(nameStart[index])) {
            allLower = false;
            break;
        }
    }
    if (expectOperator && "operator" != name) {
         return this->reportError<string>("expected operator");
    }
    const Definition* parent = this->parentSpace();
    if (parent && parent->fName.length() > 0) {
        size_t parentNameIndex = parent->fName.rfind(':');
        parentNameIndex = string::npos == parentNameIndex ? 0 : parentNameIndex + 1;
        string parentName = parent->fName.substr(parentNameIndex);
        if (parentName == name) {
            isConstructor = true;
        } else if ('~' == name[0]) {
            if (parentName != name.substr(1)) {
                 return this->reportError<string>("expected destructor");
            }
            isConstructor = true;
        }
        builder = parent->fName + "::";
    }
    bool addConst = false;
    if (isConstructor || expectOperator) {
        paren = this->strnchr(')', end) + 1;
        TextParserSave saveState(this);
        this->skipTo(paren);
        if (this->skipExact(" const")) {
            addConst = true;
        }
        saveState.restore();
    }
    builder.append(nameStart, paren - nameStart);
    if (addConst) {
        builder.append(" const");
    }
    if (!expectOperator && allLower) {
        builder.append("()");
    }
    int parens = 0;
    while (fChar < end || parens > 0) {
        if ('(' == this->peek()) {
            ++parens;
        } else if (')' == this->peek()) {
            --parens;
        }
        this->next();
    }
    TextParserSave saveState(this);
    this->skipWhiteSpace();
    if (this->startsWith("const")) {
        this->skipName("const");
    } else {
        saveState.restore();
    }
//    this->next();
    if (string::npos != builder.find('\n')) {
        builder.erase(std::remove(builder.begin(), builder.end(), '\n'), builder.end());
    }
    return uniqueRootName(builder, MarkType::kMethod);
}

const Definition* BmhParser::parentSpace() const {
    Definition* parent = nullptr;
    Definition* test = fParent;
    while (test) {
        if (MarkType::kClass == test->fMarkType ||
                MarkType::kEnumClass == test->fMarkType ||
                MarkType::kStruct == test->fMarkType) {
            parent = test;
            break;
        }
        test = test->fParent;
    }
    return parent;
}

// A full terminal statement is in the form:
//     \n optional-white-space #MarkType white-space #[# white-space]
//     \n optional-white-space #MarkType white-space Name white-space #[# white-space]
// MarkType must match definition->fMarkType
const char* BmhParser::checkForFullTerminal(const char* end, const Definition* definition) const {
    const char* start = end;
    while ('\n' != start[0] && start > fStart) {
        --start;
    }
    SkASSERT (start < end);
    // if end is preceeeded by \n#MarkType ## backup to there
    TextParser parser(fFileName, start, fChar, fLineCount);
    parser.skipWhiteSpace();
    if (parser.eof() || fMC != parser.next()) {
        return end;
    }
    const char* markName = kMarkProps[(int) definition->fMarkType].fName;
    if (!parser.skipExact(markName)) {
        return end;
    }
    parser.skipWhiteSpace();
    TextParser startName(fFileName, definition->fStart, definition->fContentStart,
            definition->fLineCount);
    if ('#' == startName.next()) {
        startName.skipToSpace();
        if (!startName.eof() && startName.skipSpace()) {
            const char* nameBegin = startName.fChar;
            startName.skipToWhiteSpace();
            string name(nameBegin, (int) (startName.fChar - nameBegin));
            if (fMC != parser.peek() && !parser.skipExact(name.c_str())) {
                return end;
            }
            parser.skipSpace();
        }
    }
    if (parser.eof() || fMC != parser.next()) {
        return end;
    }
    if (!parser.eof() && fMC != parser.next()) {
        return end;
    }
    SkASSERT(parser.eof());
    return start;
}

void BmhParser::parseHashAnchor(Definition* definition) {
    this->skipToEndBracket(fMC);
    fMarkup.emplace_front(MarkType::kLink, fChar, fLineCount, definition, fMC);
    SkAssertResult(fMC == this->next());
    this->skipWhiteSpace();
    Definition* link = &fMarkup.front();
    link->fContentStart = fChar;
    link->fContentEnd = this->trimmedBracketEnd(fMC);
    this->skipToEndBracket(fMC);
    SkAssertResult(fMC == this->next());
    SkAssertResult(fMC == this->next());
    link->fTerminator = fChar;
    definition->fContentEnd = link->fContentEnd;
    definition->fTerminator = fChar;
    definition->fChildren.emplace_back(link);
}

void BmhParser::parseHashFormula(Definition* definition) {
    const char* start = definition->fContentStart;
    definition->trimEnd();
	const char* end = definition->fContentEnd;
	fMarkup.emplace_front(MarkType::kText, start, fLineCount, definition, fMC);
	Definition* text = &fMarkup.front();
	text->fContentStart = start;
	text->fContentEnd = end;
	text->fTerminator = definition->fTerminator;
	definition->fChildren.emplace_back(text);
}

void BmhParser::parseHashLine(Definition* definition) {
	const char* nextLF = this->strnchr('\n', this->fEnd);
	const char* start = fChar;
	const char* end = this->trimmedBracketEnd(fMC);
	this->skipToEndBracket(fMC, nextLF);
	if (fMC != this->next() || fMC != this->next()) {
		return this->reportError<void>("expected ## to delineate line");
	}
	fMarkup.emplace_front(MarkType::kText, start, fLineCount, definition, fMC);
	Definition* text = &fMarkup.front();
    if (!islower(start[0]) && (!isdigit(start[0])
            || MarkType::kConst != definition->fParent->fMarkType)) {
        return this->reportError<void>("expect lower case start");
    }
    string contents = string(start, end - start);
    size_t firstSpace = contents.find(' ');
    if (string::npos == firstSpace || 0 == firstSpace || 's' != start[firstSpace - 1]) {
        if (MarkType::kMethod == fParent->fMarkType && "experimental" != contents
                    && "incomplete" != contents) {
            return this->reportError<void>( "expect phrase in third person present"
                    " tense (1st word should end in 's'");
        }
    }
	text->fContentStart = start;
	text->fContentEnd = end;
	text->fTerminator = fChar;
	definition->fContentEnd = text->fContentEnd;
	definition->fTerminator = fChar;
	definition->fChildren.emplace_back(text);
}

bool BmhParser::popParentStack(Definition* definition) {
    if (!fParent) {
        return this->reportError<bool>("missing parent");
    }
    if (definition != fParent) {
        return this->reportError<bool>("definition end is not parent");
    }
    if (!definition->fStart) {
        return this->reportError<bool>("definition missing start");
    }
    if (definition->fContentEnd) {
        return this->reportError<bool>("definition already ended");
    }
    // more to figure out to handle table columns, at minimum
    const char* end = fChar;
    if (fMC != end[0]) {
        while (end > definition->fContentStart && ' ' >= end[-1]) {
            --end;
        }
        SkASSERT(&end[-1] >= definition->fContentStart && fMC == end[-1]
                && (MarkType::kColumn == definition->fMarkType
                || (&end[-2] >= definition->fContentStart && fMC == end[-2])));
        end -= 2;
    }
    end = checkForFullTerminal(end, definition);
    definition->fContentEnd = end;
    definition->fTerminator = fChar;
    fParent = definition->fParent;
    if (!fParent || (MarkType::kTopic == fParent->fMarkType && !fParent->fParent)) {
        fRoot = nullptr;
    }
    return true;
}



bool BmhParser::skipNoName() {
    if ('\n' == this->peek()) {
        this->next();
        return true;
    }
    this->skipWhiteSpace();
    if (fMC != this->peek()) {
        return this->reportError<bool>("expected end mark 1");
    }
    this->next();
    if (fMC != this->peek()) {
        return this->reportError<bool>("expected end mark 2");
    }
    this->next();
    return true;
}

bool BmhParser::skipToDefinitionEnd(MarkType markType) {
    if (this->eof()) {
        return this->reportError<bool>("missing end");
    }
    const char* start = fLine;
    int startLineCount = fLineCount;
    int stack = 1;
    ptrdiff_t lineLen;
    bool foundEnd = false;
    do {
        lineLen = this->lineLength();
        if (fMC != *fChar++) {
            continue;
        }
        if (fMC == *fChar) {
            continue;
        }
        if (' ' == *fChar) {
            continue;
        }
        MarkType nextType = this->getMarkType(MarkLookup::kAllowUnknown);
        if (markType != nextType) {
            continue;
        }
        bool hasEnd = this->hasEndToken();
        if (hasEnd) {
            if (!--stack) {
                foundEnd = true;
                continue;
            }
        } else {
            ++stack;
        }
    } while ((void) ++fLineCount, (void) (fLine += lineLen), (void) (fChar = fLine),
            !this->eof() && !foundEnd);
    if (foundEnd) {
        return true;
    }
    fLineCount = startLineCount;
    fLine = start;
    fChar = start;
    return this->reportError<bool>("unbalanced stack");
}

bool BmhParser::skipToString() {
	this->skipSpace();
	if (fMC != this->peek()) {
		return this->reportError<bool>("expected end mark 3");
	}
	this->next();
	this->skipSpace();
	// body is text from here to double fMC
		// no single fMC allowed, no linefeed allowed
	return true;
}

vector<string> BmhParser::topicName() {
    vector<string> result;
    this->skipWhiteSpace();
    const char* lineEnd = fLine + this->lineLength();
    const char* nameStart = fChar;
    while (fChar < lineEnd) {
        char ch = this->next();
        SkASSERT(',' != ch);
        if ('\n' == ch) {
            break;
        }
        if (fMC == ch) {
            break;
        }
    }
    if (fChar - 1 > nameStart) {
        string builder(nameStart, fChar - nameStart - 1);
        trim_start_end(builder);
        result.push_back(builder);
    }
    if (fChar < lineEnd && fMC == this->peek()) {
        this->next();
    }
    return result;
}

// typeName parsing rules depend on mark type
vector<string> BmhParser::typeName(MarkType markType, bool* checkEnd) {
    fAnonymous = false;
    fCloned = false;
    vector<string> result;
    string builder;
    if (fParent) {
        builder = fParent->fName;
    }
    switch (markType) {
        case MarkType::kDefine:
        case MarkType::kEnum:
            // enums may be nameless
        case MarkType::kConst:
        case MarkType::kEnumClass:
        case MarkType::kClass:
        case MarkType::kStruct:
        case MarkType::kTemplate:
            // expect name
            builder = this->className(markType);
            break;
        case MarkType::kExample:
            // check to see if one already exists -- if so, number this one
            builder = this->uniqueName(string(), markType);
            this->skipNoName();
            break;
        case MarkType::kCode:
        case MarkType::kDescription:
        case MarkType::kExternal:
        case MarkType::kFunction:
        case MarkType::kLegend:
        case MarkType::kList:
        case MarkType::kNoExample:
            this->skipNoName();
            break;
        case MarkType::kFormula:
		case MarkType::kLine:
			this->skipToString();
			break;
        case MarkType::kAlias:
        case MarkType::kAnchor:
        case MarkType::kBug:  // fixme: expect number
        case MarkType::kDetails:
        case MarkType::kDuration:
        case MarkType::kFile:
        case MarkType::kFilter:
        case MarkType::kHeight:
        case MarkType::kIllustration:
        case MarkType::kImage:
		case MarkType::kIn:
        case MarkType::kLiteral:
        case MarkType::kNoJustify:
        case MarkType::kOutdent:
        case MarkType::kPlatform:
        case MarkType::kPopulate:
        case MarkType::kReturn:
        case MarkType::kSeeAlso:
        case MarkType::kSet:
        case MarkType::kSubstitute:
        case MarkType::kToDo:
        case MarkType::kVolatile:
        case MarkType::kWidth:
            *checkEnd = false;  // no name, may have text body
            break;
        case MarkType::kStdOut:
            this->skipNoName();
            break;  // unnamed
        case MarkType::kMember:
            builder = this->memberName();
            break;
        case MarkType::kMethod:
            builder = this->methodName();
            break;
        case MarkType::kTypedef:
            builder = this->typedefName();
            break;
        case MarkType::kParam:
            // fixme: expect camelCase for param
            builder = this->word("", "");
            this->skipSpace();
            *checkEnd = false;
            break;
        case MarkType::kPhraseDef: {
            const char* nameEnd = this->anyOf("(\n");
            builder = string(fChar, nameEnd - fChar);
            this->skipLower();
            if (fChar != nameEnd) {
                this->reportError("expect lower case only");
                break;
            }
            this->skipTo(nameEnd);
            *checkEnd = false;
            } break;
        case MarkType::kTable:
            this->skipNoName();
            break;  // unnamed
        case MarkType::kSubtopic:
        case MarkType::kTopic:
            // fixme: start with cap, allow space, hyphen, stop on comma
            // one topic can have multiple type names delineated by comma
            result = this->topicName();
            if (result.size() == 0 && this->hasEndToken()) {
                break;
            }
            return result;
        default:
            // fixme: don't allow silent failures
            SkASSERT(0);
    }
    result.push_back(builder);
    return result;
}

string BmhParser::typedefName() {
    if (this->hasEndToken()) {
        if (!fParent || !fParent->fName.length()) {
            return this->reportError<string>("missing parent typedef name");
        }
        SkASSERT(fMC == this->peek());
        this->next();
        SkASSERT(fMC == this->peek());
        this->next();
        SkASSERT(fMC != this->peek());
        return fParent->fName;
    }
    string builder;
    const Definition* parent = this->parentSpace();
    if (parent && parent->fName.length() > 0) {
        builder = parent->fName + "::";
    }
    builder += TextParser::typedefName();
    return uniqueRootName(builder, MarkType::kTypedef);
}

string BmhParser::uniqueName(string base, MarkType markType) {
    string builder(base);
    if (!builder.length()) {
        builder = fParent->fName;
    }
    if (!fParent) {
        return builder;
    }
    int number = 2;
    string numBuilder(builder);
    do {
        for (auto& iter : fParent->fChildren) {
            if (markType == iter->fMarkType) {
                if (iter->fName == numBuilder) {
                    fCloned = true;
                    numBuilder = builder + '_' + to_string(number);
                    goto tryNext;
                }
            }
        }
        break;
tryNext: ;
    } while (++number);
    return numBuilder;
}

string BmhParser::uniqueRootName(string base, MarkType markType) {
    auto checkName = [markType](const Definition& def, string numBuilder) -> bool {
        return markType == def.fMarkType && def.fName == numBuilder;
    };

    string builder(base);
    if (!builder.length()) {
        builder = fParent->fName;
    }
    int number = 2;
    string numBuilder(builder);
    Definition* cloned = nullptr;
    do {
        if (fRoot) {
            for (auto& iter : fRoot->fBranches) {
                if (checkName(*iter.second, numBuilder)) {
                    cloned = iter.second;
                    goto tryNext;
                }
            }
            for (auto& iter : fRoot->fLeaves) {
                if (checkName(iter.second, numBuilder)) {
                    cloned = &iter.second;
                    goto tryNext;
                }
            }
        } else if (fParent) {
            for (auto& iter : fParent->fChildren) {
                if (checkName(*iter, numBuilder)) {
                    cloned = &*iter;
                    goto tryNext;
                }
            }
        }
        break;
tryNext: ;
        if ("()" == builder.substr(builder.length() - 2)) {
            builder = builder.substr(0, builder.length() - 2);
        }
        if (MarkType::kMethod == markType) {
            cloned->fCloned = true;
        }
        fCloned = true;
        numBuilder = builder + '_' + to_string(number);
    } while (++number);
    return numBuilder;
}

void BmhParser::validate() const {
    for (int index = 0; index <= (int) Last_MarkType; ++index) {
        SkASSERT(kMarkProps[index].fMarkType == (MarkType) index);
    }
    const char* last = "";
    for (int index = 0; index <= (int) Last_MarkType; ++index) {
        const char* next = kMarkProps[index].fName;
        if (!last[0]) {
            last = next;
            continue;
        }
        if (!next[0]) {
            continue;
        }
        SkASSERT(strcmp(last, next) < 0);
        last = next;
    }
}

string BmhParser::word(string prefix, string delimiter) {
    string builder(prefix);
    this->skipWhiteSpace();
    const char* lineEnd = fLine + this->lineLength();
    const char* nameStart = fChar;
    while (fChar < lineEnd) {
        char ch = this->next();
        if (' ' >= ch) {
            break;
        }
        if (',' == ch) {
            return this->reportError<string>("no comma needed");
            break;
        }
        if (fMC == ch) {
            return builder;
        }
        if (!isalnum(ch) && '_' != ch && ':' != ch && '-' != ch) {
            return this->reportError<string>("unexpected char");
        }
        if (':' == ch) {
            // expect pair, and expect word to start with Sk
            if (nameStart[0] != 'S' || nameStart[1] != 'k') {
                return this->reportError<string>("expected Sk");
            }
            if (':' != this->peek()) {
                return this->reportError<string>("expected ::");
            }
            this->next();
        } else if ('-' == ch) {
            // expect word not to start with Sk or kX where X is A-Z
            if (nameStart[0] == 'k' && nameStart[1] >= 'A' && nameStart[1] <= 'Z') {
                return this->reportError<string>("didn't expected kX");
            }
            if (nameStart[0] == 'S' && nameStart[1] == 'k') {
                return this->reportError<string>("expected Sk");
            }
        }
    }
    if (prefix.size()) {
        builder += delimiter;
    }
    builder.append(nameStart, fChar - nameStart - 1);
    return builder;
}
