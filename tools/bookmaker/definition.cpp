/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bookmaker.h"
#include "SkOSPath.h"

static size_t count_indent(const string& text, size_t test, size_t end) {
    size_t result = test;
    while (test < end) {
        if (' ' != text[test]) {
            break;
        }
        ++test;
    }
    return test - result;
}

static void add_code(const string& text, int pos, int end,
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

#ifdef CONST
#undef CONST
#endif

#ifdef FRIEND
#undef FRIEND
#endif

#ifdef BLANK
#undef BLANK
#endif

#ifdef ANY
#undef ANY
#endif

#ifdef DEFOP
#undef DEFOP
#endif

#define CONST 1
#define STATIC 2
#define BLANK  0
#define ANY -1
#define DEFOP Definition::Operator

enum class OpType : int8_t {
    kNone,
    kVoid,
    kBool,
    kChar,
    kFloat,
    kInt,
    kScalar,
    kSizeT,
    kThis,
    kAny,
};

enum class OpMod : int8_t {
    kNone,
    kArray,
    kMove,
    kPointer,
    kReference,
    kAny,
};

const struct OperatorParser {
    DEFOP fOperator;
    const char* fSymbol;
    const char* fName;
    int8_t fFriend;
    OpType fReturnType;
    OpMod fReturnMod;
    int8_t fConstMethod;
    struct Param {
        int8_t fConst;
        OpType fType;
        OpMod fMod;
    } fParams[2];
} opData[] = {
    { DEFOP::kUnknown, "??", "???",    BLANK,  OpType::kNone,   OpMod::kNone,         BLANK,
                                    { } },
    { DEFOP::kAdd,     "+",  "add",    BLANK,  OpType::kThis,   OpMod::kNone,         BLANK,
                                    {{ CONST,  OpType::kThis,   OpMod::kReference, },
                                     { CONST,  OpType::kThis,   OpMod::kReference, }}},
    { DEFOP::kAddTo,   "+=", "addto",  BLANK,  OpType::kVoid,   OpMod::kNone,         BLANK,
                                    {{ CONST,  OpType::kThis,   OpMod::kReference, }}},
    { DEFOP::kAddTo,   "+=", "addto1", BLANK,  OpType::kThis,   OpMod::kReference,    BLANK,
                                    {{ CONST,  OpType::kThis,   OpMod::kReference, }}},
    { DEFOP::kAddTo,   "+=", "addto2", BLANK,  OpType::kThis,   OpMod::kReference,    BLANK,
                                    {{ CONST,  OpType::kChar,   OpMod::kArray, }}},
    { DEFOP::kAddTo,   "+=", "addto3", BLANK,  OpType::kThis,   OpMod::kReference,    BLANK,
                                    {{ CONST,  OpType::kChar,   OpMod::kNone, }}},
    { DEFOP::kArray,   "[]", "array",  BLANK,  OpType::kScalar, OpMod::kNone,         CONST,
                                    {{ BLANK,  OpType::kInt,    OpMod::kNone, }}},
    { DEFOP::kArray,   "[]", "array1", BLANK,  OpType::kScalar, OpMod::kReference,    BLANK,
                                    {{ BLANK,  OpType::kInt,    OpMod::kNone, }}},
    { DEFOP::kArray,   "[]", "array2", BLANK,  OpType::kChar,   OpMod::kNone,         CONST,
                                    {{ BLANK,  OpType::kSizeT,  OpMod::kNone, }}},
    { DEFOP::kArray,   "[]", "array3", BLANK,  OpType::kChar,   OpMod::kReference,    BLANK,
                                    {{ BLANK,  OpType::kSizeT,  OpMod::kNone, }}},
    { DEFOP::kCast,    "()", "cast",   BLANK,  OpType::kAny,    OpMod::kAny,          ANY,
                                    {{ ANY,    OpType::kAny,    OpMod::kAny,  }}},
    { DEFOP::kCopy,    "=",  "copy",   BLANK,  OpType::kThis,   OpMod::kReference,    BLANK,
                                    {{ CONST,  OpType::kThis,   OpMod::kReference, }}},
    { DEFOP::kCopy,    "=",  "copy1",  BLANK,  OpType::kThis,   OpMod::kReference,    BLANK,
                                    {{ CONST,  OpType::kChar,   OpMod::kArray, }}},
    { DEFOP::kDelete,  "delete", "delete",  BLANK,  OpType::kVoid,   OpMod::kNone,    BLANK,
                                    {{ BLANK,  OpType::kVoid,   OpMod::kPointer, }}},
    { DEFOP::kDereference, "->", "deref",  ANY,  OpType::kThis, OpMod::kPointer,      CONST,
                                    { } },
    { DEFOP::kDereference, "*", "deref", BLANK,  OpType::kThis, OpMod::kReference,    CONST,
                                    { } },
    { DEFOP::kEqual,   "==", "equal",  BLANK,  OpType::kBool,   OpMod::kNone,         BLANK,
                                    {{ CONST,  OpType::kThis,   OpMod::kReference, },
                                     { CONST,  OpType::kThis,   OpMod::kReference, }}},
    { DEFOP::kEqual,   "==", "equal1",  BLANK,  OpType::kBool,   OpMod::kNone,         CONST,
                                    {{ CONST,  OpType::kThis,   OpMod::kReference, }}},
    { DEFOP::kEqual,   "==", "equal2", ANY,    OpType::kBool,   OpMod::kNone,         BLANK,
                                    {{ CONST,  OpType::kThis,   OpMod::kReference, },
                                     { CONST,  OpType::kThis,   OpMod::kReference, }}},
    { DEFOP::kMinus,   "-",  "minus",  BLANK,  OpType::kThis,   OpMod::kNone,         CONST,
                                    { } },
    { DEFOP::kMove,    "=",  "move",   BLANK,  OpType::kThis,   OpMod::kReference,    BLANK,
                                    {{ BLANK,  OpType::kThis,   OpMod::kMove, }}},
    { DEFOP::kMultiply, "*", "multiply", BLANK, OpType::kThis, OpMod::kNone,         CONST,
                                    {{ BLANK,  OpType::kScalar, OpMod::kNone, }}},
    { DEFOP::kMultiply, "*", "multiply1", BLANK, OpType::kThis, OpMod::kNone,         BLANK,
                                    {{ CONST,  OpType::kThis,   OpMod::kReference, },
                                     { CONST,  OpType::kThis,   OpMod::kReference, }}},
    { DEFOP::kMultiplyBy, "*=", "multiplyby", BLANK,  OpType::kThis, OpMod::kReference, BLANK,
                                    {{ BLANK,  OpType::kScalar, OpMod::kNone, }}},
    { DEFOP::kNew,     "new", "new",   BLANK,  OpType::kVoid,   OpMod::kPointer,      BLANK,
                                    {{ BLANK,  OpType::kSizeT,  OpMod::kNone, }}},
    { DEFOP::kNotEqual, "!=", "notequal", BLANK, OpType::kBool,   OpMod::kNone,      BLANK,
                                    {{ CONST,  OpType::kThis,   OpMod::kReference, },
                                     { CONST,  OpType::kThis,   OpMod::kReference, }}},
    { DEFOP::kNotEqual, "!=", "notequal1", BLANK,  OpType::kBool,   OpMod::kNone,     CONST,
                                    {{ CONST,  OpType::kThis,   OpMod::kReference, }}},
    { DEFOP::kNotEqual, "!=", "notequal2", ANY, OpType::kBool,   OpMod::kNone,     BLANK,
                                    {{ CONST,  OpType::kThis,   OpMod::kReference, },
                                     { CONST,  OpType::kThis,   OpMod::kReference, }}},
    { DEFOP::kSubtract, "-", "subtract", BLANK, OpType::kThis, OpMod::kNone,         BLANK,
                                    {{ CONST,  OpType::kThis,   OpMod::kReference, },
                                     { CONST,  OpType::kThis,   OpMod::kReference, }}},
    { DEFOP::kSubtractFrom, "-=", "subtractfrom",  BLANK,  OpType::kVoid,   OpMod::kNone, BLANK,
                                    {{ CONST,  OpType::kThis,   OpMod::kReference, }}},
};

OpType lookup_type(const string& typeWord, const string& name) {
    if (typeWord == name || (typeWord == "SkIVector" && name == "SkIPoint")
                         || (typeWord == "SkVector" && name == "SkPoint")) {
        return OpType::kThis;
    }
    const char* keyWords[] = { "void", "bool", "char", "float", "int", "SkScalar", "size_t" };
    for (unsigned i = 0; i < SK_ARRAY_COUNT(keyWords); ++i) {
        if (typeWord == keyWords[i]) {
            return (OpType) (i + 1);
        }
    }
    return OpType::kNone;
}

OpMod lookup_mod(TextParser& iParser) {
    OpMod mod = OpMod::kNone;
    if ('&' == iParser.peek()) {
        mod = OpMod::kReference;
        iParser.next();
        if ('&' == iParser.peek()) {
            mod = OpMod::kMove;
            iParser.next();
        }
    } else if ('*' == iParser.peek()) {
        mod = OpMod::kPointer;
        iParser.next();
    }
    iParser.skipWhiteSpace();
    return mod;
}

bool Definition::parseOperator(size_t doubleColons, string& result) {
    const char operatorStr[] = "operator";
    size_t opPos = fName.find(operatorStr, doubleColons);
    if (string::npos == opPos) {
        return false;
    }
    string className(fName, 0, doubleColons - 2);
    TextParser iParser(fFileName, fStart, fContentStart, fLineCount);
    SkAssertResult(iParser.skipWord("#Method"));
    iParser.skipExact("SK_API");
    iParser.skipWhiteSpace();
    bool isStatic = iParser.skipExact("static");
    iParser.skipWhiteSpace();
    iParser.skipExact("SK_API");
    iParser.skipWhiteSpace();
    bool returnsConst = iParser.skipExact("const");
    if (returnsConst) {
        SkASSERT(0);  // incomplete
    }
    SkASSERT(isStatic == false || returnsConst == false);
    iParser.skipWhiteSpace();
    const char* returnTypeStart = iParser.fChar;
    iParser.skipToNonAlphaNum();
    SkASSERT(iParser.fChar > returnTypeStart);
    string returnType(returnTypeStart, iParser.fChar - returnTypeStart);
    OpType returnOpType = lookup_type(returnType, className);
    iParser.skipWhiteSpace();
    OpMod returnMod = lookup_mod(iParser);
    SkAssertResult(iParser.skipExact("operator"));
    iParser.skipWhiteSpace();
    fMethodType = Definition::MethodType::kOperator;
    TextParser::Save save(&iParser);
    for (auto parser : opData) {
        save.restore();
        if (!iParser.skipExact(parser.fSymbol)) {
            continue;
        }
        iParser.skipWhiteSpace();
        if ('(' != iParser.peek()) {
            continue;
        }
        if (parser.fFriend != ANY && (parser.fFriend == STATIC) != isStatic) {
            continue;
        }
        if (parser.fReturnType != OpType::kAny && parser.fReturnType != returnOpType) {
            continue;
        }
        if (parser.fReturnMod != OpMod::kAny && parser.fReturnMod != returnMod) {
            continue;
        }
        iParser.next();  // skip '('
        iParser.skipWhiteSpace();
        int parserCount = (parser.fParams[0].fType != OpType::kNone) +
            (parser.fParams[1].fType != OpType::kNone);
        bool countsMatch = true;
        for (int pIndex = 0; pIndex < 2; ++pIndex) {
            if (')' == iParser.peek()) {
                countsMatch = pIndex == parserCount;
                break;
            }
            if (',' == iParser.peek()) {
                iParser.next();
                iParser.skipWhiteSpace();
            }
            bool paramConst = iParser.skipExact("const");
            if (parser.fParams[pIndex].fConst != ANY &&
                    paramConst != (parser.fParams[pIndex].fConst == CONST)) {
                countsMatch = false;
                break;
            }
            iParser.skipWhiteSpace();
            const char* paramStart = iParser.fChar;
            iParser.skipToNonAlphaNum();
            SkASSERT(iParser.fChar > paramStart);
            string paramType(paramStart, iParser.fChar - paramStart);
            OpType paramOpType = lookup_type(paramType, className);
            if (parser.fParams[pIndex].fType != OpType::kAny &&
                    parser.fParams[pIndex].fType != paramOpType) {
                countsMatch = false;
                break;
            }
            iParser.skipWhiteSpace();
            OpMod paramMod = lookup_mod(iParser);
            if (parser.fParams[pIndex].fMod != OpMod::kAny &&
                    parser.fParams[pIndex].fMod != paramMod) {
                countsMatch = false;
                break;
            }
            iParser.skipToNonAlphaNum();
            if ('[' == iParser.peek()) {
                paramMod = OpMod::kArray;
                SkAssertResult(iParser.skipExact("[]"));
            }
            iParser.skipWhiteSpace();
        }
        if (!countsMatch) {
            continue;
        }
        if (')' != iParser.peek()) {
            continue;
        }
        iParser.next();
        bool constMethod = iParser.skipExact("_const");
        if (parser.fConstMethod != ANY && (parser.fConstMethod == CONST) != constMethod) {
            continue;
        }
        result += parser.fName;
        result += "_operator";
        fOperator = parser.fOperator;
        fOperatorConst = constMethod;
        return true;
    }
    SkASSERT(0); // incomplete
    return false;
#if 0
    if ('!' == fName[opPos]) {
        SkASSERT('=' == fName[opPos + 1]);
        result += "not_equal_operator";
    } else if ('=' == fName[opPos]) {
        if ('(' == fName[opPos + 1]) {
            result += isMove ? "move_" : "copy_";
            result += "assignment_operator";
        } else {
            SkASSERT('=' == fName[opPos + 1]);
            result += "equal_operator";
        }
    } else if ('[' == fName[opPos]) {
        result += "subscript_operator";
        const char* end = fContentStart;
        while (end > fStart && ' ' >= end[-1]) {
            --end;
        }
        string constCheck(fStart, end - fStart);
        size_t constPos = constCheck.rfind("const");
        if (constCheck.length() == constPos + 5) {
            result += "_const";
        }
    } else if ('*' == fName[opPos]) {
        result += "multiply_operator";
    } else if ('-' == fName[opPos]) {
        result += "subtract_operator";
    } else if ('+' == fName[opPos]) {
        result += "add_operator";
    } else {
        SkASSERT(0);  // todo: incomplete
    }
#endif
    return true;
}

#undef CONST
#undef FRIEND
#undef BLANK
#undef DEFOP

bool Definition::boilerplateIfDef(Definition* parent) {
    const Definition& label = fTokens.front();
    if (Type::kWord != label.fType) {
        return false;
    }
    fName = string(label.fContentStart, label.fContentEnd - label.fContentStart);
    return true;
}

// todo: this is matching #ifndef SkXXX_DEFINED for no particular reason
// it doesn't do anything useful with arbitrary input, e.g. #ifdef SK_SUPPORT_LEGACY_CANVAS_HELPERS
// also doesn't know what to do with SK_REQUIRE_LOCAL_VAR()
bool Definition::boilerplateDef(Definition* parent) {
    if (!this->boilerplateIfDef(parent)) {
        return false;
    }
    const char* s = fName.c_str();
    const char* e = strchr(s, '_');
    return true; // fixme: if this is trying to do something useful with define, do it here
    if (!e) {
        return false;
    }
    string prefix(s, e - s);
    const char* inName = strstr(parent->fName.c_str(), prefix.c_str());
    if (!inName) {
        return false;
    }
    if ('/' != inName[-1] && '\\' != inName[-1]) {
        return false;
    }
    if (strcmp(inName + prefix.size(), ".h")) {
        return false;
    }
    return true;
}

// fixme: this will need to be more complicated to handle all of Skia
// for now, just handle paint -- maybe fiddle will loosen naming restrictions
void Definition::setCanonicalFiddle() {
    fMethodType = Definition::MethodType::kNone;
    size_t doubleColons = fName.find("::", 0);
    SkASSERT(string::npos != doubleColons);
    string base = fName.substr(0, doubleColons);
    string result = base + "_";
    doubleColons += 2;
    if (string::npos != fName.find('~', doubleColons)) {
        fMethodType = Definition::MethodType::kDestructor;
        result += "destructor";
    } else if (!this->parseOperator(doubleColons, result)) {
        bool isMove = string::npos != fName.find("&&", doubleColons);
        size_t parens = fName.find("()", doubleColons);
        if (string::npos != parens) {
            string methodName = fName.substr(doubleColons, parens - doubleColons);
            do {
                size_t nextDouble = methodName.find("::");
                if (string::npos == nextDouble) {
                    break;
                }
                base = methodName.substr(0, nextDouble);
                result += base + '_';
                methodName = methodName.substr(nextDouble + 2);
                doubleColons += nextDouble + 2;
            } while (true);
            if (base == methodName) {
                fMethodType = Definition::MethodType::kConstructor;
                result += "empty_constructor";
            } else {
                result += fName.substr(doubleColons, fName.length() - doubleColons - 2);
            }
        } else {
            size_t openParen = fName.find('(', doubleColons);
            if (string::npos == openParen) {
                result += fName.substr(doubleColons);
            } else {
                size_t comma = fName.find(',', doubleColons);
                if (string::npos == comma) {
                    result += isMove ? "move_" : "copy_";
                }
                fMethodType = Definition::MethodType::kConstructor;
                // name them by their param types,
                //   e.g. SkCanvas__int_int_const_SkSurfaceProps_star
                // TODO: move forward until parens are balanced and terminator =,)
                TextParser params("", &fName[openParen] + 1, &*fName.end(), 0);
                bool underline = false;
                while (!params.eof()) {
//                    SkDEBUGCODE(const char* end = params.anyOf("(),="));  // unused for now
//                    SkASSERT(end[0] != '(');  // fixme: put off handling nested parentheseses
                    if (params.startsWith("const") || params.startsWith("int")
                            || params.startsWith("Sk")) {
                        const char* wordStart = params.fChar;
                        params.skipToNonAlphaNum();
                        if (underline) {
                            result += '_';
                        } else {
                            underline = true;
                        }
                        result += string(wordStart, params.fChar - wordStart);
                    } else {
                        params.skipToNonAlphaNum();
                    }
                    if (!params.eof() && '*' == params.peek()) {
                        if (underline) {
                            result += '_';
                        } else {
                            underline = true;
                        }
                        result += "star";
                        params.next();
                        params.skipSpace();
                    }
                    params.skipToAlpha();
                }
            }
        }
    }
    fFiddle = Definition::NormalizedName(result);
}

void Definition::setWrapper() {
    const char drawWrapper[] = "void draw(SkCanvas* canvas) {";
    const char drawNoCanvas[] = "void draw(SkCanvas* ) {";
    string text = this->extractText(Definition::TrimExtract::kNo);
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
        fWrapper = hasCanvas ? string(drawNoCanvas) : string(drawWrapper);
    }
}

bool Definition::exampleToScript(string* result, ExampleOptions exampleOptions) const {
    bool hasFiddle = true;
    const Definition* platform = this->hasChild(MarkType::kPlatform);
    if (platform) {
        TextParser platParse(platform);
        hasFiddle = !platParse.strnstr("!fiddle", platParse.fEnd);
    }
    if (!hasFiddle) {
        *result = "";
        return true;
    }
    string text = this->extractText(Definition::TrimExtract::kNo);
    bool textOut = string::npos != text.find("SkDebugf(")
            || string::npos != text.find("dump(")
            || string::npos != text.find("dumpHex(");
    string heightStr = "256";
    string widthStr = "256";
    string normalizedName(fFiddle);
    string code;
    string imageStr = "0";
    string srgbStr = "false";
    string durationStr = "0";
    for (auto const& iter : fChildren) {
        switch (iter->fMarkType) {
            case MarkType::kDuration:
                durationStr = string(iter->fContentStart, iter->fContentEnd - iter->fContentStart);
                break;
            case MarkType::kError:
                result->clear();
                return true;
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
                string funcText(iter->fContentStart, iter->fContentEnd - iter->fContentStart);
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
            case MarkType::kMarkChar:
            case MarkType::kPlatform:
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
    if (fWrapper.length() > 0) {
        code += fWrapper;
        code += "\\n";
        outIndent = 4;
    }
    add_code(text, pos, end, outIndent, textIndent, code);
    if (fWrapper.length() > 0) {
        code += "}";
    }
    string example = "\"" + normalizedName + "\": {\n";
    size_t nameStart = fFileName.find(SkOSPath::SEPARATOR, 0);
    SkASSERT(string::npos != nameStart);
    string baseFile = fFileName.substr(nameStart + 1, fFileName.length() - nameStart - 5);
    if (ExampleOptions::kText == exampleOptions) {
        example += "    \"code\": \"" + code + "\",\n";
        example += "    \"hash\": \"" + fHash + "\",\n";
        example += "    \"file\": \"" + baseFile + "\",\n";
        example += "    \"name\": \"" + fName + "\",";
    } else {
        example += "    \"code\": \"" + code + "\",\n";
        if (ExampleOptions::kPng == exampleOptions) {
            example += "    \"width\": " + widthStr + ",\n";
            example += "    \"height\": " + heightStr + ",\n";
            example += "    \"hash\": \"" + fHash + "\",\n";
            example += "    \"file\": \"" + baseFile + "\",\n";
            example += "    \"name\": \"" + fName + "\"\n";
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

string Definition::extractText(TrimExtract trimExtract) const {
    string result;
    TextParser parser(fFileName, fContentStart, fContentEnd, fLineCount);
    int childIndex = 0;
    char mc = '#';
    while (parser.fChar < parser.fEnd) {
        if (TrimExtract::kYes == trimExtract && !parser.skipWhiteSpace()) {
            break;
        }
        if (parser.next() == mc) {
            if (parser.next() == mc) {
                if (parser.next() == mc) {
                    mc = parser.next();
                }
            } else {
                // fixme : more work to do if # style comment is in text
                // if in method definition, could be alternate method name
                --parser.fChar;
                if (' ' < parser.fChar[0]) {
                    if (islower(parser.fChar[0])) {
                        result += '\n';
                        parser.skipLine();
                    } else {
                        SkASSERT(isupper(parser.fChar[0]));
                        parser.skipTo(fChildren[childIndex]->fTerminator);
                        if (mc == parser.fChar[0] && mc == parser.fChar[1]) {
                            parser.next();
                            parser.next();
                        }
                        childIndex++;
                    }
                } else {
                    parser.skipLine();
                }
                continue;
            }
        } else {
            --parser.fChar;
        }
        const char* end = parser.fEnd;
        const char* mark = parser.strnchr(mc, end);
        if (mark) {
            end = mark;
        }
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
    }
    return result;
}

static void space_pad(string* str) {
    size_t len = str->length();
    if (len == 0) {
        return;
    }
    char last = (*str)[len - 1];
    if ('~' == last || ' ' >= last) {
        return;
    }
    *str += ' ';
}

//start here;
// see if it possible to abstract this a little bit so it can
// additionally be used to find params and return in method prototype that
// does not have corresponding doxygen comments
bool Definition::checkMethod() const {
    SkASSERT(MarkType::kMethod == fMarkType);
    // if method returns a value, look for a return child
    // for each parameter, look for a corresponding child
    const char* end = fContentStart;
    while (end > fStart && ' ' >= end[-1]) {
        --end;
    }
    TextParser methodParser(fFileName, fStart, end, fLineCount);
    methodParser.skipWhiteSpace();
    SkASSERT(methodParser.startsWith("#Method"));
    methodParser.skipName("#Method");
    methodParser.skipSpace();
    string name = this->methodName();
    if (MethodType::kNone == fMethodType && name.length() > 2 &&
            "()" == name.substr(name.length() - 2)) {
        name = name.substr(0, name.length() - 2);
    }
    bool expectReturn = this->methodHasReturn(name, &methodParser);
    bool foundReturn = false;
    bool foundException = false;
    for (auto& child : fChildren) {
        foundException |= MarkType::kDeprecated == child->fMarkType
                || MarkType::kExperimental == child->fMarkType;
        if (MarkType::kReturn != child->fMarkType) {
            if (MarkType::kParam == child->fMarkType) {
                child->fVisited = false;
            }
            continue;
        }
        if (!expectReturn) {
            return methodParser.reportError<bool>("no #Return expected");
        }
        if (foundReturn) {
            return methodParser.reportError<bool>("multiple #Return markers");
        }
        foundReturn = true;
    }
    if (expectReturn && !foundReturn && !foundException) {
        return methodParser.reportError<bool>("missing #Return marker");
    }
    const char* paren = methodParser.strnchr('(', methodParser.fEnd);
    if (!paren) {
        return methodParser.reportError<bool>("missing #Method function definition");
    }
    const char* nextEnd = paren;
    do {
        string paramName;
        methodParser.fChar = nextEnd + 1;
        methodParser.skipSpace();
        if (!this->nextMethodParam(&methodParser, &nextEnd, &paramName)) {
            continue;
        }
        bool foundParam = false;
        for (auto& child : fChildren) {
            if (MarkType::kParam != child->fMarkType) {
                continue;
            }
            if (paramName != child->fName) {
                continue;
            }
            if (child->fVisited) {
                return methodParser.reportError<bool>("multiple #Method param with same name");
            }
            child->fVisited = true;
            if (foundParam) {
                TextParser paramError(child);
                return methodParser.reportError<bool>("multiple #Param with same name");
            }
            foundParam = true;

        }
        if (!foundParam && !foundException) {
            return methodParser.reportError<bool>("no #Param found");
        }
        if (')' == nextEnd[0]) {
            break;
        }
    } while (')' != nextEnd[0]);
    for (auto& child : fChildren) {
        if (MarkType::kParam != child->fMarkType) {
            continue;
        }
        if (!child->fVisited) {
            TextParser paramError(child);
            return paramError.reportError<bool>("#Param without param in #Method");
        }
    }
    return true;
}

bool Definition::crossCheck2(const Definition& includeToken) const {
    TextParser parser(fFileName, fStart, fContentStart, fLineCount);
    parser.skipExact("#");
    bool isMethod = parser.skipName("Method");
    const char* contentEnd;
    if (isMethod) {
        contentEnd = fContentStart;
    } else if (parser.skipName("DefinedBy")) {
        contentEnd = fContentEnd;
        while (parser.fChar < contentEnd && ' ' >= contentEnd[-1]) {
            --contentEnd;
        }
        if (parser.fChar < contentEnd - 1 && ')' == contentEnd[-1] && '(' == contentEnd[-2]) {
            contentEnd -= 2;
        }
    } else {
        return parser.reportError<bool>("unexpected crosscheck marktype");
    }
    return crossCheckInside(parser.fChar, contentEnd, includeToken);
}

bool Definition::crossCheck(const Definition& includeToken) const {
    return crossCheckInside(fContentStart, fContentEnd, includeToken);
}

bool Definition::crossCheckInside(const char* start, const char* end,
        const Definition& includeToken) const {
    TextParser def(fFileName, start, end, fLineCount);
    TextParser inc("", includeToken.fContentStart, includeToken.fContentEnd, 0);
    if (inc.startsWith("SK_API")) {
        inc.skipWord("SK_API");
    }
    if (inc.startsWith("friend")) {
        inc.skipWord("friend");
    }
    if (inc.startsWith("SK_API")) {
        inc.skipWord("SK_API");
    }
    inc.skipExact("SkDEBUGCODE(");
    do {
        bool defEof;
        bool incEof;
        do {
            defEof = def.eof() || !def.skipWhiteSpace();
            incEof = inc.eof() || !inc.skipWhiteSpace();
            if (!incEof && '/' == inc.peek() && (defEof || '/' != def.peek())) {
                inc.next();
                if ('*' == inc.peek()) {
                    inc.skipToEndBracket("*/");
                    inc.next();
                } else if ('/' == inc.peek()) {
                    inc.skipToEndBracket('\n');
                }
            } else if (!incEof && '#' == inc.peek() && (defEof || '#' != def.peek())) {
                inc.next();
                if (inc.startsWith("if")) {
                    inc.skipToEndBracket("\n");
                } else if (inc.startsWith("endif")) {
                    inc.skipToEndBracket("\n");
                } else {
                    SkASSERT(0); // incomplete
                    return false;
                }
            } else {
                break;
            }
            inc.next();
        } while (true);
        if (defEof || incEof) {
            if (defEof == incEof || (!defEof && ';' == def.peek())) {
                return true;
            }
            return false;  // allow setting breakpoint on failure
        }
        char defCh;
        do {
            defCh = def.next();
            char incCh = inc.next();
            if (' ' >= defCh && ' ' >= incCh) {
                break;
            }
            if (defCh != incCh) {
                if ('_' != defCh || ' ' != incCh || !fOperatorConst || !def.startsWith("const")) {
                    return false;
                }
            }
            if (';' == defCh) {
                return true;
            }
        } while (!def.eof() && !inc.eof());
    } while (true);
    return false;
}

string Definition::formatFunction() const {
    const char* end = fContentStart;
    while (end > fStart && ' ' >= end[-1]) {
        --end;
    }
    TextParser methodParser(fFileName, fStart, end, fLineCount);
    methodParser.skipWhiteSpace();
    SkASSERT(methodParser.startsWith("#Method"));
    methodParser.skipName("#Method");
    methodParser.skipSpace();
    const char* lastStart = methodParser.fChar;
    const int limit = 100;  // todo: allow this to be set by caller or in global or something
    string name = this->methodName();
    const char* nameInParser = methodParser.strnstr(name.c_str(), methodParser.fEnd);
    methodParser.skipTo(nameInParser);
    const char* lastEnd = methodParser.fChar;
    const char* paren = methodParser.strnchr('(', methodParser.fEnd);
    size_t indent;
    if (paren) {
        indent = (size_t) (paren - lastStart) + 1;
    } else {
        indent = (size_t) (lastEnd - lastStart);
    }
    // trim indent so longest line doesn't exceed box width
    TextParser::Save savePlace(&methodParser);
    const char* saveStart = lastStart;
    ptrdiff_t maxLine = 0;
    do {
        const char* nextStart = lastEnd;
        const char* delimiter = methodParser.anyOf(",)");
        const char* nextEnd = delimiter ? delimiter : methodParser.fEnd;
        if (delimiter) {
            while (nextStart < nextEnd && ' ' >= nextStart[0]) {
                ++nextStart;
            }
        }
        while (nextEnd > nextStart && ' ' >= nextEnd[-1]) {
            --nextEnd;
        }
        if (delimiter) {
            nextEnd += 1;
            delimiter += 1;
        }
        if (lastEnd > lastStart) {
            maxLine = SkTMax(maxLine, lastEnd - lastStart);
        }
        if (delimiter) {
            methodParser.skipTo(delimiter);
        }
        lastStart = nextStart;
        lastEnd = nextEnd;
    } while (lastStart < lastEnd);
    savePlace.restore();
    lastStart = saveStart;
    lastEnd = methodParser.fChar;
    indent = SkTMin(indent, (size_t) (limit - maxLine));
    // write string wtih trimmmed indent
    string methodStr;
    int written = 0;
    do {
        const char* nextStart = lastEnd;
        SkASSERT(written < limit);
        const char* delimiter = methodParser.anyOf(",)");
        const char* nextEnd = delimiter ? delimiter : methodParser.fEnd;
        if (delimiter) {
            while (nextStart < nextEnd && ' ' >= nextStart[0]) {
                ++nextStart;
            }
        }
        while (nextEnd > nextStart && ' ' >= nextEnd[-1]) {
            --nextEnd;
        }
        if (delimiter) {
            nextEnd += 1;
            delimiter += 1;
        }
        if (lastEnd > lastStart) {
            if (lastStart[0] != ' ') {
                space_pad(&methodStr);
            }
            methodStr += string(lastStart, (size_t) (lastEnd - lastStart));
            written += (size_t) (lastEnd - lastStart);
        }
        if (delimiter) {
            if (nextEnd - nextStart >= (ptrdiff_t) (limit - written)) {
                written = indent;
                methodStr += '\n';
                methodStr += string(indent, ' ');
            }
            methodParser.skipTo(delimiter);
        }
        lastStart = nextStart;
        lastEnd = nextEnd;
    } while (lastStart < lastEnd);
    return methodStr;
}

string Definition::fiddleName() const {
    string result;
    size_t start = 0;
    string parent;
    const Definition* parentDef = this;
    while ((parentDef = parentDef->fParent)) {
        if (MarkType::kClass == parentDef->fMarkType || MarkType::kStruct == parentDef->fMarkType) {
            parent = parentDef->fFiddle;
            break;
        }
    }
    if (parent.length() && 0 == fFiddle.compare(0, parent.length(), parent)) {
        start = parent.length();
        while (start < fFiddle.length() && '_' == fFiddle[start]) {
            ++start;
        }
    }
    size_t end = fFiddle.find_first_of('(', start);
    return fFiddle.substr(start, end - start);
}

const Definition* Definition::hasChild(MarkType markType) const {
    for (auto iter : fChildren) {
        if (markType == iter->fMarkType) {
            return iter;
        }
    }
    return nullptr;
}

const Definition* Definition::hasParam(const string& ref) const {
    SkASSERT(MarkType::kMethod == fMarkType);
    for (auto iter : fChildren) {
        if (MarkType::kParam != iter->fMarkType) {
            continue;
        }
        if (iter->fName == ref) {
            return &*iter;
        }

    }
    return nullptr;
}

bool Definition::hasMatch(const string& name) const {
    for (auto child : fChildren) {
        if (name == child->fName) {
            return true;
        }
        if (child->hasMatch(name)) {
            return true;
        }
    }
    return false;
}

bool Definition::methodHasReturn(const string& name, TextParser* methodParser) const {
    if (methodParser->skipExact("static")) {
        methodParser->skipWhiteSpace();
    }
    const char* lastStart = methodParser->fChar;
    const char* nameInParser = methodParser->strnstr(name.c_str(), methodParser->fEnd);
    methodParser->skipTo(nameInParser);
    const char* lastEnd = methodParser->fChar;
    const char* returnEnd = lastEnd;
    while (returnEnd > lastStart && ' ' == returnEnd[-1]) {
        --returnEnd;
    }
    bool expectReturn = 4 != returnEnd - lastStart || strncmp("void", lastStart, 4);
    if (MethodType::kNone != fMethodType && MethodType::kOperator != fMethodType && !expectReturn) {
        return methodParser->reportError<bool>("unexpected void");
    }
    switch (fMethodType) {
        case MethodType::kNone:
        case MethodType::kOperator:
            // either is fine
            break;
        case MethodType::kConstructor:
            expectReturn = true;
            break;
        case MethodType::kDestructor:
            expectReturn = false;
            break;
    }
    return expectReturn;
}

string Definition::methodName() const {
    string result;
    size_t start = 0;
    string parent;
    const Definition* parentDef = this;
    while ((parentDef = parentDef->fParent)) {
        if (MarkType::kClass == parentDef->fMarkType || MarkType::kStruct == parentDef->fMarkType) {
            parent = parentDef->fName;
            break;
        }
    }
    if (parent.length() && 0 == fName.compare(0, parent.length(), parent)) {
        start = parent.length();
        while (start < fName.length() && ':' == fName[start]) {
            ++start;
        }
    }
    if (fClone) {
        int lastUnder = fName.rfind('_');
        return fName.substr(start, (size_t) (lastUnder - start));
    }
    size_t end = fName.find_first_of('(', start);
    if (string::npos == end) {
        return fName.substr(start);
    }
    return fName.substr(start, end - start);
}

bool Definition::nextMethodParam(TextParser* methodParser, const char** nextEndPtr,
        string* paramName) const {
    int parenCount = 0;
    TextParser::Save saveState(methodParser);
    while (true) {
        if (methodParser->eof()) {
            return methodParser->reportError<bool>("#Method function missing close paren");
        }
        char ch = methodParser->peek();
        if ('(' == ch) {
            ++parenCount;
        }
        if (parenCount == 0 && (')' == ch || ',' == ch)) {
            *nextEndPtr = methodParser->fChar;
            break;
        }
        if (')' == ch) {
            if (0 > --parenCount) {
                return this->reportError<bool>("mismatched parentheses");
            }
        }
        methodParser->next();
    }
    saveState.restore();
    const char* nextEnd = *nextEndPtr;
    const char* paramEnd = nextEnd;
    const char* assign = methodParser->strnstr(" = ", paramEnd);
    if (assign) {
        paramEnd = assign;
    }
    const char* closeBracket = methodParser->strnstr("]", paramEnd);
    if (closeBracket) {
        const char* openBracket = methodParser->strnstr("[", paramEnd);
        if (openBracket && openBracket < closeBracket) {
            while (openBracket < --closeBracket && isdigit(closeBracket[0]))
                ;
            if (openBracket == closeBracket) {
                paramEnd = openBracket;
            }
        }
    }
    const char* function = methodParser->strnstr(")(", paramEnd);
    if (function) {
        paramEnd = function;
    }
    while (paramEnd > methodParser->fChar && ' ' == paramEnd[-1]) {
        --paramEnd;
    }
    const char* paramStart = paramEnd;
    while (paramStart > methodParser->fChar && isalnum(paramStart[-1])) {
        --paramStart;
    }
    if (paramStart > methodParser->fChar && paramStart >= paramEnd) {
        return methodParser->reportError<bool>("#Method missing param name");
    }
    *paramName = string(paramStart, paramEnd - paramStart);
    if (!paramName->length()) {
        if (')' != nextEnd[0]) {
            return methodParser->reportError<bool>("#Method malformed param");
        }
        return false;
    }
    return true;
}

string Definition::NormalizedName(string name) {
    string normalizedName = name;
    std::replace(normalizedName.begin(), normalizedName.end(), '-', '_');
    do {
        size_t doubleColon = normalizedName.find("::", 0);
        if (string::npos == doubleColon) {
            break;
        }
        normalizedName = normalizedName.substr(0, doubleColon)
            + '_' + normalizedName.substr(doubleColon + 2);
    } while (true);
    return normalizedName;
}

bool Definition::paramsMatch(const string& match, const string& name) const {
    TextParser def(fFileName, fStart, fContentStart, fLineCount);
    const char* dName = def.strnstr(name.c_str(), fContentStart);
    if (!dName) {
        return false;
    }
    def.skipTo(dName);
    TextParser m(fFileName, &match.front(), &match.back() + 1, fLineCount);
    const char* mName = m.strnstr(name.c_str(), m.fEnd);
    if (!mName) {
        return false;
    }
    m.skipTo(mName);
    while (!def.eof() && ')' != def.peek() && !m.eof() && ')' != m.peek()) {
        const char* ds = def.fChar;
        const char* ms = m.fChar;
        const char* de = def.anyOf(") \n");
        const char* me = m.anyOf(") \n");
        def.skipTo(de);
        m.skipTo(me);
        if (def.fChar - ds != m.fChar - ms) {
            return false;
        }
        if (strncmp(ds, ms, (int) (def.fChar - ds))) {
            return false;
        }
        def.skipWhiteSpace();
        m.skipWhiteSpace();
    }
    return !def.eof() && ')' == def.peek() && !m.eof() && ')' == m.peek();
}

void RootDefinition::clearVisited() {
    fVisited = false;
    for (auto& leaf : fLeaves) {
        leaf.second.fVisited = false;
    }
    for (auto& branch : fBranches) {
        branch.second->clearVisited();
    }
}

bool RootDefinition::dumpUnVisited() {
    bool success = true;
    for (auto& leaf : fLeaves) {
        if (!leaf.second.fVisited) {
            // FIXME: bugs requiring long tail fixes, suppressed here:
            // SkBitmap::validate() is wrapped in SkDEBUGCODE in .h and not parsed
            if ("SkBitmap::validate()" == leaf.first) {
                continue;
            }
            // SkPath::pathRefIsValid in #ifdef ; prefer to remove chrome dependency to fix
            if ("SkPath::pathRefIsValid" == leaf.first) {
                continue;
            }
            // FIXME: end of long tail bugs
            SkDebugf("defined in bmh but missing in include: %s\n", leaf.first.c_str());
            success = false;
        }
    }
    for (auto& branch : fBranches) {
        success &= branch.second->dumpUnVisited();
    }
    return success;
}

const Definition* RootDefinition::find(const string& ref, AllowParens allowParens) const {
    const auto leafIter = fLeaves.find(ref);
    if (leafIter != fLeaves.end()) {
        return &leafIter->second;
    }
    if (AllowParens::kYes == allowParens && string::npos == ref.find("()")) {
        string withParens = ref + "()";
        const auto parensIter = fLeaves.find(withParens);
        if (parensIter != fLeaves.end()) {
            return &parensIter->second;
        }
    }
    const auto branchIter = fBranches.find(ref);
    if (branchIter != fBranches.end()) {
        const RootDefinition* rootDef = branchIter->second;
        return rootDef;
    }
    const Definition* result = nullptr;
    for (const auto& branch : fBranches) {
        const RootDefinition* rootDef = branch.second;
        result = rootDef->find(ref, allowParens);
        if (result) {
            break;
        }
    }
    return result;
}
