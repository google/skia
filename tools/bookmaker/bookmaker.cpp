/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bookmaker.h"
#include "SkOSPath.h"

#ifdef SK_BUILD_FOR_WIN
#include <Windows.h>
#endif

DEFINE_string2(status, a, "", "File containing status of documentation. (Use in place of -b -i)");
DEFINE_string2(bmh, b, "", "Path to a *.bmh file or a directory.");
DEFINE_bool2(catalog, c, false, "Write example catalog.htm. (Requires -b -f -r)");
DEFINE_string2(examples, e, "", "File of fiddlecli input, usually fiddle.json (For now, disables -r -f -s)");
DEFINE_string2(fiddle, f, "", "File of fiddlecli output, usually fiddleout.json.");
DEFINE_bool2(hack, H, false, "Do a find/replace hack to update all *.bmh files. (Requires -b)");
// h is reserved for help
DEFINE_string2(include, i, "", "Path to a *.h file or a directory.");
DEFINE_bool2(selfcheck, k, false, "Check bmh against itself. (Requires -b)");
DEFINE_bool2(stdout, o, false, "Write file out to standard out.");
DEFINE_bool2(populate, p, false, "Populate include from bmh. (Requires -b -i)");
// q is reserved for quiet
DEFINE_string2(ref, r, "", "Resolve refs and write *.md files to path. (Requires -b -f)");
DEFINE_string2(spellcheck, s, "", "Spell-check [once, all, mispelling]. (Requires -b)");
DEFINE_bool2(tokens, t, false, "Write bmh from include. (Requires -b -i)");
DEFINE_bool2(crosscheck, x, false, "Check bmh against includes. (Requires -b -i)");
// v is reserved for verbose
DEFINE_bool2(validate, V, false, "Validate that all anchor references have definitions. (Requires -r)");
DEFINE_bool2(skip, z, false, "Skip degenerate missed in legacy preprocessor.");

/* todos:

add new markup to associate enum SaveLayerFlagsSet with typedef SaveLayerFlags, if needed.

should Return be on same line as 'Return Value'?
#Member lost all formatting
#List needs '# content ##', formatting
consts like enum members need fully qualfied refs to make a valid link
enum comments should be disallowed unless after #Enum and before first #Const
    ... or, should look for enum comments in other places
trouble with aliases, plurals
    need to keep first letter of includeWriter @param / @return lowercase
    Quad -> quad, Quads -> quads
deprecated methods should be sorted down in md out, and show include "Deprecated." text body.
rewrap text to fit in some number of columns
#Literal is inflexible, making the entire #Code block link-less (see $Literal in SkImageInfo)
     would rather keep links for body above #Literal, and/or make it a block and not a one-liner
add check to require #Const to contain #Code block if defining const or constexpr (enum consts have
     #Code blocks inside the #Enum def)

There are a number of formatting bugs with ad hoc patches where a substitution doesn't keep
the space before or after, or the linefeeds before or after. The rules are not very good either.
Linefeeds in the bmh file are intended to be respected, but #Formula tends to start on a new line
even if the contents is intended to be inlined. Probably need to require it to be, e.g.:

    array length must be #Formula # (fXCount + 1) * (fYCount + 1) ##.

where there is always a space between prior words and formula (i.e., between "be" and "(fXCount";
and, an absense of a space after ## denotes no space between "+ 1)" and ".". These rules preserve
that # commands are always preceded by a whitespace character. Similarly, #PhraseDef/Ref
need to be inline or create new paragraphs. #phrase_ref# is sufficiently flexible that it can be
treated as a word without trailing whitespace, adapting the whitespace of its context. It also must
always have leading whitespace.

It's awkward that phrase param is a child of the phrase def. Since phrase refs may also be children,
there is special case code to skip phrase def when looking for additional substitutions in the
phrase def. Could put it in the token list instead I guess, or make a definition subclass used
by phrase def with an additional slot...

#Deprecated soon
##
should emit the text "To be deprecated soon." (right now you get just "soon")

SkCanvas_ColorBehavior_kLegacy missing </table> in md out

rearrange const out for md so that const / value / short description comes first in a table,
followed by more elaborate descriptions, examples, seealso. In md.cpp, look to see if #Subtopic
has #Const children. If so, generate a summary table first.
Or, only allow #Line and moderate text description in #Const. Put more verbose text, example,
seealso, in subsequent #SubTopic. Alpha_Type does this and it looks good.

picture reference subclass AbortCallback has empty subtopics, and fails to show the full
prototype for ~AbortCallback in the md out generation.

see head of selfCheck.cpp for additional todos
see head of spellCheck.cpp for additional todos
 */

/*
  class contains named struct, enum, enum-member, method, topic, subtopic
     everything contained by class is uniquely named
     contained names may be reused by other classes
  method contains named parameters
     parameters may be reused in other methods
 */

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
, { "Code",         MarkType::kCode,         R_F, E_N, M_CSST | M_E | M_MD | M(Typedef) }
, { "",             MarkType::kColumn,       R_Y, E_N, M(Row) }
, { "",             MarkType::kComment,      R_N, E_N, 0 }
, { "Const",        MarkType::kConst,        R_Y, E_O, M_E | M_CSST  }
, { "Define",       MarkType::kDefine,       R_O, E_Y, M_ST }
, { "Deprecated",   MarkType::kDeprecated,   R_Y, E_N, M_CS | M_MDCM | M_E }
, { "Description",  MarkType::kDescription,  R_Y, E_N, M(Example) | M(NoExample) }
, { "Details",      MarkType::kDetails,      R_N, E_N, M(Const) }
, { "Duration",     MarkType::kDuration,     R_N, E_N, M(Example) | M(NoExample) }
, { "Enum",         MarkType::kEnum,         R_Y, E_O, M_CSST }
, { "EnumClass",    MarkType::kEnumClass,    R_Y, E_O, M_CSST }
, { "Example",      MarkType::kExample,      R_O, E_N, M_CSST | M_E | M_MD | M(Const) }
, { "Experimental", MarkType::kExperimental, R_Y, E_N, M_CS | M_MDCM | M_E }
, { "External",     MarkType::kExternal,     R_Y, E_N, 0 }
, { "File",         MarkType::kFile,         R_Y, E_N, M(Topic) }
, { "Formula",      MarkType::kFormula,      R_F, E_N, M(Column) | M(Description)
                                                     | M_E | M_ST | M_MDCM }
, { "Function",     MarkType::kFunction,     R_O, E_N, M(Example) | M(NoExample) }
, { "Height",       MarkType::kHeight,       R_N, E_N, M(Example) | M(NoExample) }
, { "Illustration", MarkType::kIllustration, R_N, E_N, M_CSST | M_MD }
, { "Image",        MarkType::kImage,        R_N, E_N, M(Example) | M(NoExample) }
, { "In",           MarkType::kIn,           R_N, E_N, M_CSST | M_E | M(Method) | M(Typedef) }
, { "Legend",       MarkType::kLegend,       R_Y, E_N, M(Table) }
, { "Line",         MarkType::kLine,         R_N, E_N, M_CSST | M_E | M(Method) | M(Typedef) }
, { "",             MarkType::kLink,         R_N, E_N, M(Anchor) }
, { "List",         MarkType::kList,         R_Y, E_N, M(Method) | M_CSST | M_E | M_D }
, { "Literal",      MarkType::kLiteral,      R_N, E_N, M(Code) }
, { "",             MarkType::kMarkChar,     R_N, E_N, 0 }
, { "Member",       MarkType::kMember,       R_Y, E_N, M_CSST }
, { "Method",       MarkType::kMethod,       R_Y, E_Y, M_CSST }
, { "NoExample",    MarkType::kNoExample,    R_N, E_N, M_CSST | M_E | M_MD }
, { "NoJustify",    MarkType::kNoJustify,    R_N, E_N, M(Const) | M(Member) }
, { "Outdent",      MarkType::kOutdent,      R_N, E_N, M(Code) }
, { "Param",        MarkType::kParam,        R_Y, E_N, M(Method) | M(Define) }
, { "PhraseDef",    MarkType::kPhraseDef,    R_Y, E_N, M_ST }
, { "",             MarkType::kPhraseParam,  R_Y, E_N, 0 }
, { "",             MarkType::kPhraseRef,    R_N, E_N, 0 }
, { "Platform",     MarkType::kPlatform,     R_N, E_N, M(Example) | M(NoExample) }
, { "Populate",     MarkType::kPopulate,     R_N, E_N, M(Subtopic) }
, { "Private",      MarkType::kPrivate,      R_N, E_N, M_CSST | M_MDCM | M_E }
, { "Return",       MarkType::kReturn,       R_Y, E_N, M(Method) }
, { "",             MarkType::kRow,          R_Y, E_N, M(Table) | M(List) }
, { "SeeAlso",      MarkType::kSeeAlso,      R_C, E_N, M_CSST | M_E | M_MD | M(Typedef) }
, { "Set",          MarkType::kSet,          R_N, E_N, M(Example) | M(NoExample) }
, { "StdOut",       MarkType::kStdOut,       R_N, E_N, M(Example) | M(NoExample) }
, { "Struct",       MarkType::kStruct,       R_Y, E_O, M(Class) | M_ST }
, { "Substitute",   MarkType::kSubstitute,   R_N, E_N, M(Alias) | M_ST }
, { "Subtopic",     MarkType::kSubtopic,     R_Y, E_Y, M_CSST }
, { "Table",        MarkType::kTable,        R_Y, E_N, M(Method) | M_CSST | M_E }
, { "Template",     MarkType::kTemplate,     R_Y, E_N, M_CSST }
, { "",             MarkType::kText,         R_N, E_N, 0 }
, { "ToDo",         MarkType::kToDo,         R_N, E_N, 0 }
, { "Topic",        MarkType::kTopic,        R_Y, E_Y, 0 }
, { "Typedef",      MarkType::kTypedef,      R_Y, E_N, M_CSST | M_E }
, { "Union",        MarkType::kUnion,        R_Y, E_N, M_CSST }
, { "Volatile",     MarkType::kVolatile,     R_N, E_N, M(StdOut) }
, { "Width",        MarkType::kWidth,        R_N, E_N, M(Example) | M(NoExample) }
};

#undef R_O
#undef R_N
#undef R_Y
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
                    if (MarkType::kStruct == markType || MarkType::kClass == markType) {
                        // if class or struct, build fRoot hierarchy
                        // and change isDefined to search all parents of fRoot
                        SkASSERT(!hasEnd);
                        RootDefinition* childRoot = new RootDefinition;
                        (fRoot->fBranches)[name] = childRoot;
                        childRoot->setRootParent(fRoot);
                        childRoot->fFileName = fFileName;
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
                     hasExcluder |= MarkType::kPrivate == child->fMarkType
                            || MarkType::kDeprecated == child->fMarkType
                            || MarkType::kExperimental == child->fMarkType
                            || MarkType::kNoExample == child->fMarkType;
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
        case MarkType::kFormula:
        case MarkType::kFunction:
        case MarkType::kLegend:
        case MarkType::kList:
        case MarkType::kPrivate:
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
        case MarkType::kDeprecated:
        case MarkType::kDetails:
        case MarkType::kDuration:
        case MarkType::kExperimental:
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
			} else if (MarkType::kLine == markType) {
				const char* nextLF = this->strnchr('\n', this->fEnd);
				const char* start = fChar;
				const char* end = this->trimmedBracketEnd(fMC);
				this->skipToEndBracket(fMC, nextLF);
				if (fMC != this->next() || fMC != this->next()) {
					return this->reportError<bool>("expected ## to delineate line");
				}
				fMarkup.emplace_front(MarkType::kText, start, fLineCount, definition, fMC);
				Definition* text = &fMarkup.front();
                if (!islower(start[0]) && (!isdigit(start[0])
                        || MarkType::kConst != definition->fParent->fMarkType)) {
                    return this->reportError<bool>("expect lower case start");
                }
                string contents = string(start, end - start);
                if (string::npos != contents.find('.')) {
                    return this->reportError<bool>("expect phrase, not sentence");
                }
                size_t firstSpace = contents.find(' ');
                if (string::npos == firstSpace || 0 == firstSpace || 's' != start[firstSpace - 1]) {
                    if (MarkType::kMethod == fParent->fMarkType && "experimental" != contents
                             && "incomplete" != contents) {
                        return this->reportError<bool>( "expect phrase in third person present"
                                " tense (1st word should end in 's'");
                    }
                }
				text->fContentStart = start;
				text->fContentEnd = end;
				text->fTerminator = fChar;
				definition->fContentEnd = text->fContentEnd;
				definition->fTerminator = fChar;
				definition->fChildren.emplace_back(text);
			} else if (IncompleteAllowed(markType)) {
                 this->skipSpace();
                 fParent->fDeprecated = true;
                 fParent->fDetails =
                        this->skipExact("soon") ? Definition::Details::kSoonToBe_Deprecated :
                        this->skipExact("testing") ? Definition::Details::kTestingOnly_Experiment :
                        this->skipExact("do not use") ? Definition::Details::kDoNotUse_Experiment :
                        this->skipExact("not ready") ? Definition::Details::kNotReady_Experiment :
                        Definition::Details::kNone;
                 this->skipSpace();
                 if ('\n' != this->peek()) {
                     return this->reportError<bool>("unexpected text after #Deprecated");
                 }
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
        this->skipToNonName();
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
    FILE* fiddleOut = fopen(fiddleJsonFileName, "wb");
    if (!fiddleOut) {
        SkDebugf("could not open output file %s\n", fiddleJsonFileName);
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
    SkDebugf("wrote %s\n", fiddleJsonFileName);
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
                    return this->reportError<bool>("expected definition");
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
                if (!fParent || (MarkType::kTable != fParent->fMarkType
                        && MarkType::kLegend != fParent->fMarkType
                        && MarkType::kList != fParent->fMarkType
						&& MarkType::kLine != fParent->fMarkType)) {
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

    // write #In to show containing #Topic
	// write #Line with one liner from Member_Functions, Constructors, Operators if method,
	//    from Constants if enum, otherwise from #Subtopic containing match
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
    SkASSERT(fBmhParser.fClassMap.end() != mapEntry);
    const Definition* classMarkup = &mapEntry->second;
    const Definition* root = classMarkup->fParent;
    SkASSERT(root);
    SkASSERT(root->fTerminator);
    SkASSERT('\n' == root->fTerminator[0]);
    SkASSERT(!root->fParent);
    fStart = root->fStart;
    fChar = fStart;
    fClasses = nullptr;
    fConstants = nullptr;
    fConstructors = nullptr;
    fMemberFunctions = nullptr;
    fMembers = nullptr;
    fOperators = nullptr;
    fRelatedFunctions = nullptr;
    fStructs = nullptr;
    this->topicIter(root);
    fprintf(fOut, "%.*s", (int) (fEnd - fChar), fChar);
    fclose(fOut);
    if (this->writtenFileDiffers(filename, root->fFileName)) {
        SkDebugf("wrote %s\n", filename.c_str());
    } else {
        remove(filename.c_str());
    }
    return true;
}

string HackParser::searchTable(const Definition* tableHolder, const Definition* match) {
    if (!tableHolder) {
        return "";
    }
    string bestMatch;
    string result;
    for (auto table : tableHolder->fChildren) {
        if (MarkType::kTable == table->fMarkType) {
            for (auto row : table->fChildren) {
                if (MarkType::kRow == row->fMarkType) {
                    const Definition* col0 = row->fChildren[0];
                    size_t len = col0->fContentEnd - col0->fContentStart;
                    string method = string(col0->fContentStart, len);
                    if (len - 2 == method.find("()") && islower(method[0])
                            && Definition::MethodType::kOperator != match->fMethodType) {
                        method = method.substr(0, len - 2);
                    }
                    if (string::npos == match->fName.find(method)) {
                        continue;
                    }
                    if (bestMatch.length() < method.length()) {
                        bestMatch = method;
                        const Definition * col1 = row->fChildren[1];
                        if (col1->fContentEnd <= col1->fContentStart) {
                            SkASSERT(string::npos != col1->fFileName.find("SkImageInfo"));
                            result = "incomplete";
                        } else {
                            result = string(col1->fContentStart, col1->fContentEnd -
                                    col1->fContentStart);
                        }
                    }
                }
            }
        }
    }
    return result;
}

// returns true if topic has method
void HackParser::topicIter(const Definition* topic) {
    if (string::npos != topic->fName.find(SubtopicKeys::kClasses)) {
        SkASSERT(!fClasses);
        fClasses = topic;
    }
    if (string::npos != topic->fName.find(SubtopicKeys::kStructs)) {
        SkASSERT(!fStructs);
        fStructs = topic;
    }
    if (string::npos != topic->fName.find(SubtopicKeys::kConstants)) {
        SkASSERT(!fConstants);
        fConstants = topic;
    }
    if (string::npos != topic->fName.find(SubtopicKeys::kConstructors)) {
        SkASSERT(!fConstructors);
        fConstructors = topic;
    }
    if (string::npos != topic->fName.find(SubtopicKeys::kMemberFunctions)) {
        SkASSERT(!fMemberFunctions);
        fMemberFunctions = topic;
    }
    if (string::npos != topic->fName.find(SubtopicKeys::kMembers)) {
        SkASSERT(!fMembers);
        fMembers = topic;
    }
    if (string::npos != topic->fName.find(SubtopicKeys::kOperators)) {
        SkASSERT(!fOperators);
        fOperators = topic;
    }
    if (string::npos != topic->fName.find(SubtopicKeys::kRelatedFunctions)) {
        SkASSERT(!fRelatedFunctions);
        fRelatedFunctions = topic;
    }
    for (auto child : topic->fChildren) {
        string oneLiner;
        bool hasIn = false;
        bool hasLine = false;
        for (auto part : child->fChildren) {
            hasIn |= MarkType::kIn == part->fMarkType;
            hasLine |= MarkType::kLine == part->fMarkType;
        }
        switch (child->fMarkType) {
            case MarkType::kMethod: {
                hasIn |= MarkType::kTopic != topic->fMarkType &&
                        MarkType::kSubtopic != topic->fMarkType;  // don't write #In if parent is class
                hasLine |= child->fClone;
                if (!hasLine) {
                    // find member_functions, add entry 2nd column text to #Line
                    for (auto tableHolder : { fMemberFunctions, fConstructors, fOperators }) {
                        if (!tableHolder) {
                            continue;
                        }
                        if (Definition::MethodType::kConstructor == child->fMethodType
                                && fConstructors != tableHolder) {
                            continue;
                        }
                        if (Definition::MethodType::kOperator == child->fMethodType
                                && fOperators != tableHolder) {
                            continue;
                        }
                        string temp = this->searchTable(tableHolder, child);
                        if ("" != temp) {
                            SkASSERT("" == oneLiner || temp == oneLiner);
                            oneLiner = temp;
                        }
                    }
                    if ("" == oneLiner) {
    #ifdef SK_DEBUG
                        const Definition* rootParent = topic;
                        while (rootParent->fParent && MarkType::kClass != rootParent->fMarkType
                                 && MarkType::kStruct != rootParent->fMarkType) {
                            rootParent = rootParent->fParent;
                        }
    #endif
                        SkASSERT(rootParent);
                        SkASSERT(MarkType::kClass == rootParent->fMarkType
                                || MarkType::kStruct == rootParent->fMarkType);
                        hasLine = true;
                    }
                }

                if (hasIn && hasLine) {
                    continue;
                }
                const char* start = fChar;
                const char* end = child->fContentStart;
                fprintf(fOut, "%.*s", (int) (end - start), start);
                fChar = end;
                // write to method markup header end
                if (!hasIn) {
                    fprintf(fOut, "\n#In %s", topic->fName.c_str());
                }
                if (!hasLine) {
                    fprintf(fOut, "\n#Line # %s ##", oneLiner.c_str());
                }
                } break;
            case MarkType::kTopic:
            case MarkType::kSubtopic:
                this->addOneLiner(fRelatedFunctions, child, hasLine, true);
                this->topicIter(child);
                break;
            case MarkType::kStruct:
                this->addOneLiner(fStructs, child, hasLine, false);
                this->topicIter(child);
                break;
            case MarkType::kClass:
                this->addOneLiner(fClasses, child, hasLine, false);
                this->topicIter(child);
                break;
            case MarkType::kEnum:
            case MarkType::kEnumClass:
                this->addOneLiner(fConstants, child, hasLine, true);
                break;
            case MarkType::kMember:
                this->addOneLiner(fMembers, child, hasLine, false);
                break;
            default:
                ;
        }
    }
}

void HackParser::addOneLiner(const Definition* defTable, const Definition* child, bool hasLine,
        bool lfAfter) {
    if (hasLine) {
        return;
    }
    string oneLiner = this->searchTable(defTable, child);
    if ("" == oneLiner) {
        return;
    }
    const char* start = fChar;
    const char* end = child->fContentStart;
    fprintf(fOut, "%.*s", (int) (end - start), start);
    fChar = end;
    if (!lfAfter) {
        fprintf(fOut, "\n");
    }
    fprintf(fOut, "#Line # %s ##", oneLiner.c_str());
    if (lfAfter) {
        fprintf(fOut, "\n");
    }
}

bool BmhParser::hasEndToken() const {
    const char* last = fLine + this->lineLength();
    while (last > fLine && ' ' >= *--last)
        ;
    if (--last < fLine) {
        return false;
    }
    return last[0] == fMC && last[1] == fMC;
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
        if (this->skipExact("_const")) {
            addConst = true;
        }
        saveState.restore();
    }
    builder.append(nameStart, paren - nameStart);
    if (addConst) {
        builder.append("_const");
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

TextParser::TextParser(const Definition* definition) :
    TextParser(definition->fFileName, definition->fContentStart, definition->fContentEnd,
        definition->fLineCount) {
}

string TextParser::ReportFilename(string file) {
	string fullName;
#ifdef SK_BUILD_FOR_WIN
	TCHAR pathChars[MAX_PATH];
	DWORD pathLen = GetCurrentDirectory(MAX_PATH, pathChars);
	for (DWORD index = 0; index < pathLen; ++index) {
		fullName += pathChars[index] == (char)pathChars[index] ? (char)pathChars[index] : '?';
	}
	fullName += '\\';
#endif
	fullName += file;
    return fullName;
}

void TextParser::reportError(const char* errorStr) const {
    this->reportWarning(errorStr);
    SkDebugf("");  // convenient place to set a breakpoint
}

void TextParser::reportWarning(const char* errorStr) const {
    SkASSERT(fLine < fEnd);
    TextParser err(fFileName, fLine, fEnd, fLineCount);
    size_t lineLen = this->lineLength();
    ptrdiff_t spaces = fChar - fLine;
    while (spaces > 0 && (size_t) spaces > lineLen) {
        ++err.fLineCount;
        err.fLine += lineLen;
        spaces -= lineLen;
        lineLen = err.lineLength();
    }
	string fullName = this->ReportFilename(fFileName);
    SkDebugf("\n%s(%zd): error: %s\n", fullName.c_str(), err.fLineCount, errorStr);
    if (0 == lineLen) {
        SkDebugf("[blank line]\n");
    } else {
        while (lineLen > 0 && '\n' == err.fLine[lineLen - 1]) {
            --lineLen;
        }
        SkDebugf("%.*s\n", (int) lineLen, err.fLine);
        SkDebugf("%*s^\n", (int) spaces, "");
    }
}

void TextParser::setForErrorReporting(const Definition* definition, const char* str) {
    fFileName = definition->fFileName;
    fStart = definition->fContentStart;
    fLine = str;
    while (fLine > fStart && fLine[-1] != '\n') {
        --fLine;
    }
    fChar = str;
    fEnd = definition->fContentEnd;
    fLineCount = definition->fLineCount;
    const char* lineInc = fStart;
    while (lineInc < str) {
        fLineCount += '\n' == *lineInc++;
    }
}

string TextParser::typedefName() {
    // look for typedef as one of three forms:
    // typedef return-type (*NAME)(params);
    // typedef alias NAME;
    // typedef std::function<alias> NAME;
    string builder;
    const char* end = this->doubleLF();
    if (!end) {
       end = fEnd;
    }
    const char* altEnd = this->strnstr("#Typedef ##", end);
    if (altEnd) {
        end = this->strnchr('\n', end);
    }
    if (!end) {
        return this->reportError<string>("missing typedef std::function end bracket >");
    }
    bool stdFunction = this->startsWith("std::function");
    if (stdFunction) {
        if (!this->skipToEndBracket('>')) {
            return this->reportError<string>("missing typedef std::function end bracket >");
        }
        this->next();
        this->skipWhiteSpace();
        builder += string(fChar, end - fChar);
    } else {
        const char* paren = this->strnchr('(', end);
        if (!paren) {
            const char* lastWord = nullptr;
            do {
                this->skipToWhiteSpace();
                if (fChar < end && isspace(fChar[0])) {
                    const char* whiteStart = fChar;
                    this->skipWhiteSpace();
                    // FIXME: test should be for fMC
                    if ('#' == fChar[0]) {
                        end = whiteStart;
                        break;
                    }
                    lastWord = fChar;
                } else {
                    break;
                }
            } while (true);
            if (!lastWord) {
                return this->reportError<string>("missing typedef name");
            }
            builder += string(lastWord, end - lastWord);
        } else {
            this->skipTo(paren);
            this->next();
            if ('*' != this->next()) {
                return this->reportError<string>("missing typedef function asterisk");
            }
            const char* nameStart = fChar;
            if (!this->skipToEndBracket(')')) {
                return this->reportError<string>("missing typedef function )");
            }
            builder += string(nameStart, fChar - nameStart);
            if (!this->skipToEndBracket('(')) {
                return this->reportError<string>("missing typedef params (");
            }
            if (! this->skipToEndBracket(')')) {
                return this->reportError<string>("missing typedef params )");
            }
            this->skipTo(end);
        }
    }
    return builder;
}

bool BmhParser::skipNoName() {
    if ('\n' == this->peek()) {
        this->next();
        return true;
    }
    this->skipWhiteSpace();
    if (fMC != this->peek()) {
        return this->reportError<bool>("expected end mark");
    }
    this->next();
    if (fMC != this->peek()) {
        return this->reportError<bool>("expected end mark");
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
		return this->reportError<bool>("expected end mark");
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
        case MarkType::kFormula:
        case MarkType::kFunction:
        case MarkType::kLegend:
        case MarkType::kList:
        case MarkType::kNoExample:
        case MarkType::kPrivate:
            this->skipNoName();
            break;
		case MarkType::kLine:
			this->skipToString();
			break;
        case MarkType::kAlias:
        case MarkType::kAnchor:
        case MarkType::kBug:  // fixme: expect number
        case MarkType::kDeprecated:
        case MarkType::kDetails:
        case MarkType::kDuration:
        case MarkType::kExperimental:
        case MarkType::kFile:
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
                    if (iter->fDeprecated) {
                        iter->fClone = true;
                    } else {
                        fCloned = true;
                    }
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
            if (cloned->fDeprecated) {
                cloned->fClone = true;
            } else {
                fCloned = true;
            }
        } else {
            fCloned = true;
        }
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

// pass one: parse text, collect definitions
// pass two: lookup references

static int count_children(const Definition& def, MarkType markType) {
    int count = 0;
    if (markType == def.fMarkType) {
        ++count;
    }
    for (auto& child : def.fChildren ) {
        count += count_children(*child, markType);
    }
    return count;
}

int main(int argc, char** const argv) {
    BmhParser bmhParser(FLAGS_skip);
    bmhParser.validate();

    SkCommandLineFlags::SetUsage(
        "Common Usage: bookmaker -b path/to/bmh_files -i path/to/include.h -t\n"
        "              bookmaker -b path/to/bmh_files -e fiddle.json\n"
        "              ~/go/bin/fiddlecli --input fiddle.json --output fiddleout.json\n"
        "              bookmaker -b path/to/bmh_files -f fiddleout.json -r path/to/md_files\n"
        "              bookmaker -a path/to/status.json -x\n"
        "              bookmaker -a path/to/status.json -p\n");
    bool help = false;
    for (int i = 1; i < argc; i++) {
        if (0 == strcmp("-h", argv[i]) || 0 == strcmp("--help", argv[i])) {
            help = true;
            for (int j = i + 1; j < argc; j++) {
                if (SkStrStartsWith(argv[j], '-')) {
                    break;
                }
                help = false;
            }
            break;
        }
    }
    if (!help) {
        SkCommandLineFlags::Parse(argc, argv);
    } else {
        SkCommandLineFlags::PrintUsage();
        const char* const commands[] = { "", "-h", "bmh", "-h", "examples", "-h", "include",
            "-h", "fiddle", "-h", "ref", "-h", "status", "-h", "tokens",
            "-h", "crosscheck", "-h", "populate", "-h", "spellcheck" };
        SkCommandLineFlags::Parse(SK_ARRAY_COUNT(commands), commands);
        return 0;
    }
    if (FLAGS_bmh.isEmpty() && FLAGS_include.isEmpty() && FLAGS_status.isEmpty()) {
        SkDebugf("requires at least one of: -b -i -a\n");
        SkCommandLineFlags::PrintUsage();
        return 1;
    }
    if (!FLAGS_bmh.isEmpty() && !FLAGS_status.isEmpty()) {
        SkDebugf("requires -b or -a but not both\n");
        SkCommandLineFlags::PrintUsage();
        return 1;
    }
    if (!FLAGS_include.isEmpty() && !FLAGS_status.isEmpty()) {
        SkDebugf("requires -i or -a but not both\n");
        SkCommandLineFlags::PrintUsage();
        return 1;
    }
    if (FLAGS_bmh.isEmpty() && FLAGS_status.isEmpty() && FLAGS_catalog) {
         SkDebugf("-c requires -b or -a\n");
        SkCommandLineFlags::PrintUsage();
        return 1;
    }
    if ((FLAGS_fiddle.isEmpty() || FLAGS_ref.isEmpty()) && FLAGS_catalog) {
        SkDebugf("-c requires -f -r\n");
        SkCommandLineFlags::PrintUsage();
        return 1;
    }
    if (FLAGS_bmh.isEmpty() && FLAGS_status.isEmpty() && !FLAGS_examples.isEmpty()) {
        SkDebugf("-e requires -b or -a\n");
        SkCommandLineFlags::PrintUsage();
        return 1;
    }
    if ((FLAGS_include.isEmpty() || FLAGS_bmh.isEmpty()) && FLAGS_status.isEmpty() &&
            FLAGS_populate) {
        SkDebugf("-p requires -b -i or -a\n");
        SkCommandLineFlags::PrintUsage();
        return 1;
    }
    if (FLAGS_bmh.isEmpty() && FLAGS_status.isEmpty() && !FLAGS_ref.isEmpty()) {
        SkDebugf("-r requires -b or -a\n");
        SkCommandLineFlags::PrintUsage();
        return 1;
    }
    if (FLAGS_bmh.isEmpty() && FLAGS_status.isEmpty() && !FLAGS_spellcheck.isEmpty()) {
        SkDebugf("-s requires -b or -a\n");
        SkCommandLineFlags::PrintUsage();
        return 1;
    }
    if ((FLAGS_include.isEmpty() || FLAGS_bmh.isEmpty()) && FLAGS_tokens) {
        SkDebugf("-t requires -b -i\n");
        SkCommandLineFlags::PrintUsage();
        return 1;
    }
    if ((FLAGS_include.isEmpty() || FLAGS_bmh.isEmpty()) && FLAGS_status.isEmpty() &&
            FLAGS_crosscheck) {
        SkDebugf("-x requires -b -i or -a\n");
        SkCommandLineFlags::PrintUsage();
        return 1;
    }
    bmhParser.reset();
    if (!FLAGS_bmh.isEmpty()) {
        if (FLAGS_tokens)  {
            IncludeParser::RemoveFile(FLAGS_bmh[0], FLAGS_include[0]);
        }
        if (!bmhParser.parseFile(FLAGS_bmh[0], ".bmh", ParserCommon::OneFile::kNo)) {
            return -1;
        }
    } else if (!FLAGS_status.isEmpty()) {
        if (!bmhParser.parseStatus(FLAGS_status[0], ".bmh", StatusFilter::kInProgress)) {
            return -1;
        }
    }
    if (FLAGS_hack) {
        if (FLAGS_bmh.isEmpty()) {
            SkDebugf("-k or --hack requires -b\n");
            SkCommandLineFlags::PrintUsage();
            return 1;
        }
        HackParser hacker(bmhParser);
        if (!hacker.parseFile(FLAGS_bmh[0], ".bmh", ParserCommon::OneFile::kNo)) {
            SkDebugf("hack failed\n");
            return -1;
        }
        return 0;
    }
    if (FLAGS_selfcheck && !SelfCheck(bmhParser)) {
        return -1;
    }
    bool done = false;
    if (!FLAGS_include.isEmpty() && FLAGS_tokens) {
        IncludeParser includeParser;
        includeParser.validate();
        if (!includeParser.parseFile(FLAGS_include[0], ".h", ParserCommon::OneFile::kNo)) {
            return -1;
        }
        if (FLAGS_tokens) {
            includeParser.fDebugOut = FLAGS_stdout;
            if (includeParser.dumpTokens()) {
                bmhParser.fWroteOut = true;
            }
            done = true;
        }
    } else if (!FLAGS_include.isEmpty() || !FLAGS_status.isEmpty()) {
        if (FLAGS_crosscheck) {
            IncludeParser includeParser;
            includeParser.validate();
            if (!FLAGS_include.isEmpty() &&
                    !includeParser.parseFile(FLAGS_include[0], ".h", ParserCommon::OneFile::kNo)) {
                return -1;
            }
            if (!FLAGS_status.isEmpty() && !includeParser.parseStatus(FLAGS_status[0], ".h",
                    StatusFilter::kCompleted)) {
                return -1;
            }
            if (!includeParser.crossCheck(bmhParser)) {
                return -1;
            }
            done = true;
        } else if (FLAGS_populate) {
            IncludeWriter includeWriter;
            includeWriter.validate();
            if (!FLAGS_include.isEmpty() &&
                    !includeWriter.parseFile(FLAGS_include[0], ".h", ParserCommon::OneFile::kNo)) {
                return -1;
            }
            if (!FLAGS_status.isEmpty() && !includeWriter.parseStatus(FLAGS_status[0], ".h",
                    StatusFilter::kCompleted)) {
                return -1;
            }
            includeWriter.fDebugOut = FLAGS_stdout;
            if (!includeWriter.populate(bmhParser)) {
                return -1;
            }
            bmhParser.fWroteOut = true;
            done = true;
        }
    }
    if (!done && !FLAGS_fiddle.isEmpty() && FLAGS_examples.isEmpty()) {
        FiddleParser fparser(&bmhParser);
        if (!fparser.parseFromFile(FLAGS_fiddle[0])) {
            return -1;
        }
    }
    if (!done && FLAGS_catalog && FLAGS_examples.isEmpty()) {
        Catalog cparser(&bmhParser);
        cparser.fDebugOut = FLAGS_stdout;
        if (!FLAGS_bmh.isEmpty() && !cparser.openCatalog(FLAGS_bmh[0], FLAGS_ref[0])) {
            return -1;
        }
        if (!FLAGS_status.isEmpty() && !cparser.openStatus(FLAGS_status[0], FLAGS_ref[0])) {
            return -1;
        }
        if (!cparser.parseFile(FLAGS_fiddle[0], ".txt", ParserCommon::OneFile::kNo)) {
            return -1;
        }
        if (!cparser.closeCatalog()) {
            return -1;
        }
        bmhParser.fWroteOut = true;
        done = true;
    }
    if (!done && !FLAGS_ref.isEmpty() && FLAGS_examples.isEmpty()) {
        IncludeParser includeParser;
        includeParser.validate();
        if (!FLAGS_include.isEmpty() && !includeParser.parseFile(FLAGS_include[0], ".h",
                ParserCommon::OneFile::kYes)) {
            return -1;
        }
        MdOut mdOut(bmhParser);
        mdOut.fDebugOut = FLAGS_stdout;
        mdOut.fValidate = FLAGS_validate;
        if (!FLAGS_bmh.isEmpty() && mdOut.buildReferences(includeParser,
                FLAGS_bmh[0], FLAGS_ref[0])) {
            bmhParser.fWroteOut = true;
        }
        if (!FLAGS_status.isEmpty() && mdOut.buildStatus(FLAGS_status[0], FLAGS_ref[0])) {
            bmhParser.fWroteOut = true;
        }
        if (FLAGS_validate) {
            mdOut.checkAnchors();
        }
    }
    if (!done && !FLAGS_spellcheck.isEmpty() && FLAGS_examples.isEmpty()) {
        if (!FLAGS_bmh.isEmpty()) {
            bmhParser.spellCheck(FLAGS_bmh[0], FLAGS_spellcheck);
        }
        if (!FLAGS_status.isEmpty()) {
            bmhParser.spellStatus(FLAGS_status[0], FLAGS_spellcheck);
        }
        bmhParser.fWroteOut = true;
        done = true;
    }
    int examples = 0;
    int methods = 0;
    int topics = 0;
    if (!done && !FLAGS_examples.isEmpty()) {
        // check to see if examples have duplicate names
        if (!bmhParser.checkExamples()) {
            return -1;
        }
        bmhParser.fDebugOut = FLAGS_stdout;
        if (!bmhParser.dumpExamples(FLAGS_examples[0])) {
            return -1;
        }
        return 0;
    }
    if (!bmhParser.fWroteOut) {
        for (const auto& topic : bmhParser.fTopicMap) {
            if (topic.second->fParent) {
                continue;
            }
            examples += count_children(*topic.second, MarkType::kExample);
            methods += count_children(*topic.second, MarkType::kMethod);
            topics += count_children(*topic.second, MarkType::kSubtopic);
            topics += count_children(*topic.second, MarkType::kTopic);
        }
        SkDebugf("topics=%d classes=%d methods=%d examples=%d\n",
                bmhParser.fTopicMap.size(), bmhParser.fClassMap.size(),
                methods, examples);
    }
    return 0;
}
