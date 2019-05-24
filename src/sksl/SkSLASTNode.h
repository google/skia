/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTNODE
#define SKSL_ASTNODE

#include "SkSLCompiler.h"
#include "SkSLLexer.h"
#include "SkSLString.h"
#include "ir/SkSLModifiers.h"

#include <vector>

namespace SkSL {

/**
 * Represents a node in the abstract syntax tree (AST). The AST is based directly on the parse tree;
 * it is a parsed-but-not-yet-analyzed version of the program.
 */
struct ASTNode {
    enum class Kind {
        // data: operator(Token), children: left, right
        kBinary,
        // children: statements
        kBlock,
        // data: value(bool)
        kBool,
        kBreak,
        // children: target, arg1, arg2...
        kCall,
        kContinue,
        kDiscard,
        // children: statement, test
        kDo,
        // data: name(StringFragment), children: enumCases
        kEnum,
        // data: name(StringFragment), children: value?
        kEnumCase,
        // data: name(StringFragment)
        kExtension,
        // data: field(StringFragment), children: base
        kField,
        // data: value(float)
        kFloat,
        // children: init, test, next, statement
        kFor,
        // data: FunctionData, children: returnType, parameters, statement?
        kFunction,
        // data: name(StringFragment)
        kIdentifier,
        // children: base, index?
        kIndex,
        // data: isStatic(bool), children: test, ifTrue, ifFalse?
        kIf,
        // value(data): int
        kInt,
        // data: InterfaceBlockData, children: declaration1, declaration2, ..., size1, size2, ...
        kInterfaceBlock,
        // data: Modifiers
        kModifiers,
        kNull,
        // data: VarData, children: type, arraySize1, arraySize2, ..., value?
        kParameter,
        // data: operator(Token), children: operand
        kPostfix,
        // data: operator(Token), children: operand
        kPrefix,
        // children: value
        kReturn,
        // ...
        kSection,
        // children: value, statement 1, statement 2...
        kSwitchCase,
        // children: value, case 1, case 2...
        kSwitch,
        // children: test, ifTrue, ifFalse
        kTernary,
        // data: TypeData, children: sizes
        kType,
        // data: VarData, children: arraySize1, arraySize2, ..., value?
        kVarDeclaration,
        // children: modifiers, type, varDeclaration1, varDeclaration2, ...
        kVarDeclarations,
        // children: test, statement
        kWhile,
    };

    struct TypeData {
        StringFragment fName;
        bool fIsStructDeclaration;
        bool fIsNullable;
    };

    struct ParameterData {
        Modifiers fModifiers;
        StringFragment fName;
        size_t fSizeCount;
    };

    struct VarData {
        StringFragment fName;
        size_t fSizeCount;
    };

    struct FunctionData {
        Modifiers fModifiers;
        StringFragment fName;
        size_t fParameterCount;
    };

    struct InterfaceBlockData {
        Modifiers fModifiers;
        StringFragment fTypeName;
        size_t fDeclarationCount;
        StringFragment fInstanceName;
        size_t fSizeCount;
    };

    struct SectionData {
        StringFragment fName;
        StringFragment fArgument;
        StringFragment fText;
    };

    struct NodeData {
        enum class Kind {
            kToken,
            kStringFragment,
            kBool,
            kInt,
            kFloat,
            kModifiers,
            kTypeData,
            kFunctionData,
            kParameterData,
            kVarData,
            kInterfaceBlockData,
            kSectionData
        } fKind;

        uint8_t fBytes[std::max(sizeof(Token), std::max(sizeof(StringFragment),
                       std::max(sizeof(bool), std::max(sizeof(SKSL_INT),
                       std::max(sizeof(SKSL_FLOAT), std::max(sizeof(Modifiers),
                       std::max(sizeof(TypeData), std::max(sizeof(FunctionData),
                       std::max(sizeof(ParameterData), std::max(sizeof(VarData),
                       std::max(sizeof(InterfaceBlockData), sizeof(SectionData))))))))))))];

        NodeData() = default;

        NodeData(Token data)
        : fKind(Kind::kToken) {
            memcpy(fBytes, &data, sizeof(data));
        }

        NodeData(StringFragment data)
        : fKind(Kind::kStringFragment) {
            memcpy(fBytes, &data, sizeof(data));
        }

        NodeData(bool data)
        : fKind(Kind::kBool) {
            memcpy(fBytes, &data, sizeof(data));
        }

        NodeData(SKSL_INT data)
        : fKind(Kind::kInt) {
            memcpy(fBytes, &data, sizeof(data));
        }

        NodeData(SKSL_FLOAT data)
        : fKind(Kind::kFloat) {
            memcpy(fBytes, &data, sizeof(data));
        }

        NodeData(Modifiers data)
        : fKind(Kind::kModifiers) {
            memcpy(fBytes, &data, sizeof(data));
        }

        NodeData(TypeData data)
        : fKind(Kind::kTypeData) {
            memcpy(fBytes, &data, sizeof(data));
        }

        NodeData(FunctionData data)
        : fKind(Kind::kFunctionData) {
            memcpy(fBytes, &data, sizeof(data));
        }

        NodeData(VarData data)
        : fKind(Kind::kVarData) {
            memcpy(fBytes, &data, sizeof(data));
        }

        NodeData(ParameterData data)
        : fKind(Kind::kParameterData) {
            memcpy(fBytes, &data, sizeof(data));
        }

        NodeData(InterfaceBlockData data)
        : fKind(Kind::kInterfaceBlockData) {
            memcpy(fBytes, &data, sizeof(data));
        }

        NodeData(SectionData data)
        : fKind(Kind::kSectionData) {
            memcpy(fBytes, &data, sizeof(data));
        }
    };

    ASTNode()
    : fOffset(-1)
    , fKind(Kind::kNull) {}

    ASTNode(int offset, Kind kind)
    : fOffset(offset)
    , fKind(kind) {}

    ASTNode(int offset, Kind kind, std::vector<ASTNode> children)
    : fOffset(offset)
    , fKind(kind)
    , fChildren(std::move(children)) {}

    ASTNode(int offset, Kind kind, Token t, std::vector<ASTNode> children)
    : fOffset(offset)
    , fKind(kind)
    , fData(t)
    , fChildren(std::move(children)) {}

    ASTNode(int offset, Kind kind, StringFragment s)
    : fOffset(offset)
    , fKind(kind)
    , fData(s) {}

    ASTNode(int offset, Kind kind, StringFragment s, std::vector<ASTNode> children)
    : fOffset(offset)
    , fKind(kind)
    , fData(s)
    , fChildren(std::move(children)) {}

    ASTNode(int offset, Kind kind, const char* s)
    : fOffset(offset)
    , fKind(kind)
    , fData(StringFragment(s)) {}

    ASTNode(int offset, Kind kind, const char* s, std::vector<ASTNode> children)
    : fOffset(offset)
    , fKind(kind)
    , fData(StringFragment(s))
    , fChildren(std::move(children)) {}

    ASTNode(int offset, Kind kind, bool b)
    : fOffset(offset)
    , fKind(kind)
    , fData(b) {}

    ASTNode(int offset, Kind kind, bool b, std::vector<ASTNode> children)
    : fOffset(offset)
    , fKind(kind)
    , fData(b)
    , fChildren(children) {}

    ASTNode(int offset, Kind kind, SKSL_INT i)
    : fOffset(offset)
    , fKind(kind)
    , fData(i) {}

    ASTNode(int offset, Kind kind, SKSL_FLOAT f)
    : fOffset(offset)
    , fKind(kind)
    , fData(f) {}

    ASTNode(int offset, Kind kind, Modifiers m)
    : fOffset(offset)
    , fKind(kind)
    , fData(m) {}

    ASTNode(int offset, Kind kind, FunctionData f, std::vector<ASTNode> children)
    : fOffset(offset)
    , fKind(kind)
    , fData(f)
    , fChildren(std::move(children)) {}

    ASTNode(int offset, Kind kind, TypeData td)
    : fOffset(offset)
    , fKind(kind)
    , fData(td) {}

    ASTNode(int offset, Kind kind, TypeData td, std::vector<ASTNode> children)
    : fOffset(offset)
    , fKind(kind)
    , fData(td)
    , fChildren(std::move(children)) {}

    ASTNode(int offset, Kind kind, VarData v)
    : fOffset(offset)
    , fKind(kind)
    , fData(v) {}

    ASTNode(int offset, Kind kind, ParameterData p, std::vector<ASTNode> children)
    : fOffset(offset)
    , fKind(kind)
    , fData(p)
    , fChildren(std::move(children)) {}

    ASTNode(int offset, Kind kind, VarData v, std::vector<ASTNode> children)
    : fOffset(offset)
    , fKind(kind)
    , fData(v)
    , fChildren(std::move(children)) {}

    ASTNode(int offset, Kind kind, InterfaceBlockData ib, std::vector<ASTNode> children)
    : fOffset(offset)
    , fKind(kind)
    , fData(ib)
    , fChildren(std::move(children)) {}

    ASTNode(int offset, Kind kind, SectionData s)
    : fOffset(offset)
    , fKind(kind)
    , fData(s) {}

    operator bool() const {
        return fKind != Kind::kNull;
    }

    Token getToken() const {
        SkASSERT(fData.fKind == NodeData::Kind::kToken);
        return *(Token*) fData.fBytes;
    }

    bool getBool() const {
        SkASSERT(fData.fKind == NodeData::Kind::kBool);
        return *(bool*) fData.fBytes;
    }

    SKSL_INT getInt() const {
        SkASSERT(fData.fKind == NodeData::Kind::kInt);
        return *(SKSL_INT*) fData.fBytes;
    }

    SKSL_FLOAT getFloat() const {
        SkASSERT(fData.fKind == NodeData::Kind::kFloat);
        return *(SKSL_FLOAT*) fData.fBytes;
    }

    StringFragment getString() const {
        SkASSERT(fData.fKind == NodeData::Kind::kStringFragment);
        return *(StringFragment*) fData.fBytes;
    }

    Modifiers getModifiers() const {
        SkASSERT(fData.fKind == NodeData::Kind::kModifiers);
        return *(Modifiers*) fData.fBytes;
    }

    TypeData getTypeData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kTypeData);
        return *(TypeData*) fData.fBytes;
    }

    ParameterData getParameterData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kParameterData);
        return *(ParameterData*) fData.fBytes;
    }

    VarData getVarData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kVarData);
        return *(VarData*) fData.fBytes;
    }

    FunctionData getFunctionData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kFunctionData);
        return *(FunctionData*) fData.fBytes;
    }

    InterfaceBlockData getInterfaceBlockData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kInterfaceBlockData);
        return *(InterfaceBlockData*) fData.fBytes;
    }

    SectionData getSectionData() const {
        SkASSERT(fData.fKind == NodeData::Kind::kSectionData);
        return *(SectionData*) fData.fBytes;
    }

    String description() const {
        switch (fKind) {
            case Kind::kNull: return "";
            case Kind::kBinary: return "(" + fChildren[0].description() + " " +
                                       Compiler::OperatorName(getToken().fKind) + " " +
                                       fChildren[1].description() + ")";
            case Kind::kBlock: {
                String result = "{\n";
                for (const auto& c : fChildren) {
                    result += c.description();
                    result += "\n";
                }
                result += "}";
                return result;
            }
            case Kind::kBool:
                return getBool() ? "true" : "false";
            case Kind::kBreak:
                return "break";
            case Kind::kCall: {
                String result = fChildren[0].description();
                result += "(";
                const char* separator = "";
                for (size_t i = 1; i < fChildren.size(); ++i) {
                    result += separator;
                    result += fChildren[i].description();
                    separator = ",";
                }
                result += ")";
                return result;
            }
            case Kind::kContinue:
                return "continue";
            case Kind::kDiscard:
                return "discard";
            case Kind::kDo:
                return "do " + fChildren[0].description() + " while (" +
                       fChildren[1].description() + ")";
            case Kind::kEnum: {
                String result = "enum ";
                result += getString();
                result += " {\n";
                for (const auto& c : fChildren) {
                    result += c.description();
                    result += "\n";
                }
                result += "};";
                return result;
            }
            case Kind::kEnumCase:
                if (fChildren.size()) {
                    return String(getString()) + " = " + fChildren[0].description();
                }
                return getString();
            case Kind::kExtension:
                return "#extension " + getString();
            case Kind::kField:
                return fChildren[0].description() + "." + getString();
            case Kind::kFloat:
                return to_string(getFloat());
            case Kind::kFor:
                return "for (" + fChildren[0].description() + "; " + fChildren[1].description() +
                       "; " + fChildren[2].description() + ") " + fChildren[3].description();
            case Kind::kFunction: {
                FunctionData fd = getFunctionData();
                String result = fd.fModifiers.description();
                if (result.size()) {
                    result += " ";
                }
                result += fChildren[0].description() + " " + fd.fName + "(";
                const char* separator = "";
                for (size_t i = 0; i < fd.fParameterCount; ++i) {
                    result += separator;
                    result += fChildren[i].description();
                }
                result += ")";
                if (fChildren.size() > fd.fParameterCount) {
                    SkASSERT(fChildren.size() == fd.fParameterCount + 1);
                    result += " " + fChildren[fd.fParameterCount].description();
                }
                else {
                    result += ";";
                }
                return result;
            }
            case Kind::kIdentifier:
                return getString();
            case Kind::kIndex:
                return fChildren[0].description() + "[" + fChildren[1].description() + "]";
            case Kind::kIf: {
                String result;
                if (getBool()) {
                    result = "@";
                }
                result += "if (" + fChildren[0].description() + ") " +
                        fChildren[1].description();
                if (fChildren.size() > 2) {
                    SkASSERT(fChildren.size() == 3);
                    result += " else " + fChildren[2].description();
                }
                return result;
            }
            case Kind::kInt:
                return to_string(getInt());
            case Kind::kInterfaceBlock: {
                InterfaceBlockData id = getInterfaceBlockData();
                String result = id.fModifiers.description() + " " + id.fTypeName + " {\n";
                for (size_t i = 0; i < id.fDeclarationCount; ++i) {
                    result += fChildren[i].description() + "\n";
                }
                result += "} ";
                result += id.fInstanceName;
                for (size_t i = 0; i < id.fSizeCount; ++i) {
                    result += "[" + fChildren[id.fDeclarationCount + i].description() + "]";
                }
                result += ";";
                return result;
            }
            case Kind::kModifiers:
                return getModifiers().description();
            case Kind::kParameter: {
                ParameterData pd = getParameterData();
                String result = "(type:" + fChildren[0].description() + ":type) " + pd.fName;
                for (size_t i = 0; i < pd.fSizeCount; ++i) {
                    result += "[" + fChildren[i + 1].description() + "]";
                }
                if (fChildren.size() > pd.fSizeCount + 1) {
                    SkASSERT(fChildren.size() == pd.fSizeCount + 2);
                    result += " = " + fChildren[pd.fSizeCount + 1].description();
                }
                return result;
            }
            case Kind::kPostfix:
                return fChildren[0].description() + Compiler::OperatorName(getToken().fKind);
            case Kind::kPrefix:
                return Compiler::OperatorName(getToken().fKind) + fChildren[0].description();
            case Kind::kReturn:
                if (fChildren.size()) {
                    SkASSERT(fChildren.size() == 1);
                    return "return " + fChildren[0].description() + ";";
                }
                return "return;";
            case Kind::kSection:
                SkASSERT(false);
            case Kind::kSwitchCase: {
                String result = fChildren[0].description() + ":";
                for (size_t i = 1; i < fChildren.size(); ++i) {
                    result += "\n" + fChildren[i].description();
                }
                return result;
            }
            case Kind::kSwitch: {
                String result = "switch (" + fChildren[0].description() + ") {";
                for (size_t i = 1; i < fChildren.size(); ++i) {
                    result += fChildren[i].description() + "\n";
                }
                result += "}";
                return result;
            }
            case Kind::kTernary:
                return "(" + fChildren[0].description() + " ? " + fChildren[1].description() +
                       " : " + fChildren[2].description() + ")";
            case Kind::kType:
                return String(getTypeData().fName);
            case Kind::kVarDeclaration: {
                VarData vd = getVarData();
                String result = vd.fName;
                for (size_t i = 0; i < vd.fSizeCount; ++i) {
                    result += "[" + fChildren[i].description() + "]";
                }
                if (fChildren.size() > vd.fSizeCount) {
                    SkASSERT(fChildren.size() == vd.fSizeCount + 1);
                    result += " = " + fChildren[vd.fSizeCount].description();
                }
                return result;
            }
            case Kind::kVarDeclarations: {
                String result = fChildren[0].description();
                if (result.size()) {
                    result += " ";
                }
                result += fChildren[1].description();
                const char* separator = " ";
                for (size_t i = 2; i < fChildren.size(); ++i) {
                    result += separator + fChildren[i].description();
                    separator = ", ";
                }
                return result;
            }
            default:
                SkASSERT(false);
                return "<error>";
        }
    }

    int fOffset;

    Kind fKind;

    NodeData fData;

    std::vector<ASTNode> fChildren;
};

} // namespace

#endif
