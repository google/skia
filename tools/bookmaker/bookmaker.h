/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef bookmaker_DEFINED
#define bookmaker_DEFINED

#include <algorithm>
#include <cmath>
#include <cctype>
#include <cstring>
#include <forward_list>
#include <list>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "SkTypes.h"

using std::forward_list;
using std::list;
using std::string;
using std::unordered_map;
using std::vector;

class Definition;

class NonAssignable {
public:
    NonAssignable(NonAssignable const&) = delete;
    NonAssignable& operator=(NonAssignable const&) = delete;
    NonAssignable() {}
};

#define FPRINTF(...)                \
    if (fDebugOut) {                \
        SkDebugf(__VA_ARGS__);      \
    }                               \
    fprintf(fOut, __VA_ARGS__)

// std::to_string isn't implemented on android
template <typename T>
string to_string(T value)
{
    std::ostringstream os ;
    os << value ;
    return os.str() ;
}

enum class KeyWord {
    kNone,
    kSK_API,
    kSK_BEGIN_REQUIRE_DENSE,
    kAlignAs,
    kBool,
    kChar,
    kClass,
    kConst,
    kConstExpr,
    kDefine,
    kDouble,
    kElif,
    kElse,
    kEndif,
    kEnum,
    kError,
    kFloat,
    kFriend,
    kIf,
    kIfdef,
    kIfndef,
    kInclude,
    kInline,
    kInt,
    kOperator,
    kPrivate,
    kProtected,
    kPublic,
    kSigned,
    kSize_t,
    kStatic,
    kStruct,
    kTemplate,
    kTypedef,
    kTypename,
    kUint16_t,
    kUint32_t,
    kUint64_t,
    kUint8_t,
    kUintPtr_t,
    kUnion,
    kUnsigned,
    kUsing,
    kVoid,
};

enum class MarkType {
    kNone,
    kAnchor,
    kAlias,
    kBug,
    kClass,
    kCode,
    kColumn,
    kComment,
    kConst,
    kDefine,
    kDescription,
    kDetails,  // used by #Const to specify #Subtopic details with examples and so on
    kDuration,
    kEnum,
    kEnumClass,
    kExample,
    kExternal,
    kFile,
    kFilter,
    kFormula,
    kFunction,
    kHeight,
    kIllustration,
    kImage,
	kIn,
    kLegend,
	kLine,
    kLink,     // used internally by #Anchor
    kList,
    kLiteral,  // don't lookup hyperlinks, do substitution, etc
    kMarkChar,
    kMember,
    kMethod,
    kNoExample,
    kNoJustify, // don't contribute this #Line to tabular comment measure, even if it fits
    kOutdent,
    kParam,
    kPhraseDef,
    kPhraseParam,
    kPhraseRef,
    kPlatform,
    kPopulate,
    kReturn,
    kRow,
    kSeeAlso,
    kSet,
    kStdOut,
    kStruct,
    kSubstitute,
    kSubtopic,
    kTable,
    kTemplate,
    kText,
    kToDo,
    kTopic,
    kTypedef,
    kUnion,
    kUsing,
    kVolatile,
    kWidth,
};

enum {
    Last_MarkType = (int) MarkType::kWidth,
};

enum class Bracket {
    kNone,
    kParen,
    kSquare,
    kBrace,
    kAngle,
    kString,
    kChar,
    kSlashStar,
    kSlashSlash,
    kPound,
    kColon,
    kDebugCode,  // parens get special treatment so SkDEBUGCODE( isn't treated as method
};

enum class Punctuation {  // catch-all for misc symbols tracked in C
    kNone,
    kAsterisk,  // for pointer-to
    kSemicolon,  // e.g., to delinate xxx() const ; const int* yyy()
    kLeftBrace,
    kColon,     // for foo() : bar(1), baz(2) {}
};

enum class KeyProperty {
    kNone,
    kClassSection,
    kFunction,
    kModifier,
    kNumber,
    kObject,
    kPreprocessor,
};

struct IncludeKey {
    const char* fName;
    KeyWord fKeyWord;
    KeyProperty fProperty;
};

extern const IncludeKey kKeyWords[];

struct NameMap {
    void copyToParent(NameMap* parent) const;
    void setParams(Definition* bmhDef, Definition* iMethod);

    string fName;
    NameMap* fParent = nullptr;
    unordered_map<string, string> fLinkMap;   // from SkRect to #Rect
    // ref map includes "xxx", "xxx ", "xxx yyy", "xxx zzz", etc.
    unordered_map<string, Definition*> fRefMap;    // e.g., from #Substitute entry to #Topic entry
};

enum class Resolvable {
    kNo,      // neither resolved nor output
    kYes,     // resolved, output
    kOut,     // mostly resolved, output (FIXME: is this really different from kYes?)
    kCode,    // resolve methods as they are used, not as they are prototyped
    kFormula, // kCode, plus make most spaces non-breaking
    kLiteral, // output untouched
	kClone,   // resolved, output, with references to clones as well
    kSimple,  // resolve simple words (used to resolve method declarations)
    kInclude, // like simple, plus reverse resolve SkXXX to XXX
};

#endif
