/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLMetalCodeGenerator.h"

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLExtension.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLModifiersDeclaration.h"
#include "src/sksl/ir/SkSLNop.h"
#include "src/sksl/ir/SkSLVariableReference.h"

namespace SkSL {

void MetalCodeGenerator::setupIntrinsics() {
#define METAL(x) std::make_pair(kMetal_IntrinsicKind, k ## x ## _MetalIntrinsic)
#define SPECIAL(x) std::make_pair(kSpecial_IntrinsicKind, k ## x ## _SpecialIntrinsic)
    fIntrinsicMap[String("sample")]             = SPECIAL(Texture);
    fIntrinsicMap[String("mod")]                = SPECIAL(Mod);
    fIntrinsicMap[String("equal")]              = METAL(Equal);
    fIntrinsicMap[String("notEqual")]           = METAL(NotEqual);
    fIntrinsicMap[String("lessThan")]           = METAL(LessThan);
    fIntrinsicMap[String("lessThanEqual")]      = METAL(LessThanEqual);
    fIntrinsicMap[String("greaterThan")]        = METAL(GreaterThan);
    fIntrinsicMap[String("greaterThanEqual")]   = METAL(GreaterThanEqual);
}

void MetalCodeGenerator::write(const char* s) {
    if (!s[0]) {
        return;
    }
    if (fAtLineStart) {
        for (int i = 0; i < fIndentation; i++) {
            fOut->writeText("    ");
        }
    }
    fOut->writeText(s);
    fAtLineStart = false;
}

void MetalCodeGenerator::writeLine(const char* s) {
    this->write(s);
    fOut->writeText(fLineEnding);
    fAtLineStart = true;
}

void MetalCodeGenerator::write(const String& s) {
    this->write(s.c_str());
}

void MetalCodeGenerator::writeLine(const String& s) {
    this->writeLine(s.c_str());
}

void MetalCodeGenerator::writeLine() {
    this->writeLine("");
}

void MetalCodeGenerator::writeExtension(const Extension& ext) {
    this->writeLine("#extension " + ext.fName + " : enable");
}

String MetalCodeGenerator::typeName(const Type& type) {
    switch (type.kind()) {
        case Type::kVector_Kind:
            return this->typeName(type.componentType()) + to_string(type.columns());
        case Type::kMatrix_Kind:
            return this->typeName(type.componentType()) + to_string(type.columns()) + "x" +
                                  to_string(type.rows());
        case Type::kSampler_Kind:
            return "texture2d<float>"; // FIXME - support other texture types;
        default:
            if (type == *fContext.fHalf_Type) {
                // FIXME - Currently only supporting floats in MSL to avoid type coercion issues.
                return fContext.fFloat_Type->name();
            } else if (type == *fContext.fByte_Type) {
                return "char";
            } else if (type == *fContext.fUByte_Type) {
                return "uchar";
            } else {
                return type.name();
            }
    }
}

void MetalCodeGenerator::writeType(const Type& type) {
    if (type.kind() == Type::kStruct_Kind) {
        for (const Type* search : fWrittenStructs) {
            if (*search == type) {
                // already written
                this->write(type.name());
                return;
            }
        }
        fWrittenStructs.push_back(&type);
        this->writeLine("struct " + type.name() + " {");
        fIndentation++;
        this->writeFields(type.fields(), type.fOffset);
        fIndentation--;
        this->write("}");
    } else {
        this->write(this->typeName(type));
    }
}

void MetalCodeGenerator::writeExpression(const Expression& expr, Precedence parentPrecedence) {
    switch (expr.fKind) {
        case Expression::kBinary_Kind:
            this->writeBinaryExpression((BinaryExpression&) expr, parentPrecedence);
            break;
        case Expression::kBoolLiteral_Kind:
            this->writeBoolLiteral((BoolLiteral&) expr);
            break;
        case Expression::kConstructor_Kind:
            this->writeConstructor((Constructor&) expr, parentPrecedence);
            break;
        case Expression::kIntLiteral_Kind:
            this->writeIntLiteral((IntLiteral&) expr);
            break;
        case Expression::kFieldAccess_Kind:
            this->writeFieldAccess(((FieldAccess&) expr));
            break;
        case Expression::kFloatLiteral_Kind:
            this->writeFloatLiteral(((FloatLiteral&) expr));
            break;
        case Expression::kFunctionCall_Kind:
            this->writeFunctionCall((FunctionCall&) expr);
            break;
        case Expression::kPrefix_Kind:
            this->writePrefixExpression((PrefixExpression&) expr, parentPrecedence);
            break;
        case Expression::kPostfix_Kind:
            this->writePostfixExpression((PostfixExpression&) expr, parentPrecedence);
            break;
        case Expression::kSetting_Kind:
            this->writeSetting((Setting&) expr);
            break;
        case Expression::kSwizzle_Kind:
            this->writeSwizzle((Swizzle&) expr);
            break;
        case Expression::kVariableReference_Kind:
            this->writeVariableReference((VariableReference&) expr);
            break;
        case Expression::kTernary_Kind:
            this->writeTernaryExpression((TernaryExpression&) expr, parentPrecedence);
            break;
        case Expression::kIndex_Kind:
            this->writeIndexExpression((IndexExpression&) expr);
            break;
        default:
#ifdef SK_DEBUG
            ABORT("unsupported expression: %s", expr.description().c_str());
#endif
            break;
    }
}

void MetalCodeGenerator::writeIntrinsicCall(const FunctionCall& c) {
    auto i = fIntrinsicMap.find(c.fFunction.fName);
    SkASSERT(i != fIntrinsicMap.end());
    Intrinsic intrinsic = i->second;
    int32_t intrinsicId = intrinsic.second;
    switch (intrinsic.first) {
        case kSpecial_IntrinsicKind:
            return this->writeSpecialIntrinsic(c, (SpecialIntrinsic) intrinsicId);
            break;
        case kMetal_IntrinsicKind:
            this->writeExpression(*c.fArguments[0], kSequence_Precedence);
            switch ((MetalIntrinsic) intrinsicId) {
                case kEqual_MetalIntrinsic:
                    this->write(" == ");
                    break;
                case kNotEqual_MetalIntrinsic:
                    this->write(" != ");
                    break;
                case kLessThan_MetalIntrinsic:
                    this->write(" < ");
                    break;
                case kLessThanEqual_MetalIntrinsic:
                    this->write(" <= ");
                    break;
                case kGreaterThan_MetalIntrinsic:
                    this->write(" > ");
                    break;
                case kGreaterThanEqual_MetalIntrinsic:
                    this->write(" >= ");
                    break;
                default:
                    ABORT("unsupported metal intrinsic kind");
            }
            this->writeExpression(*c.fArguments[1], kSequence_Precedence);
            break;
        default:
            ABORT("unsupported intrinsic kind");
    }
}

void MetalCodeGenerator::writeFunctionCall(const FunctionCall& c) {
    const auto& entry = fIntrinsicMap.find(c.fFunction.fName);
    if (entry != fIntrinsicMap.end()) {
        this->writeIntrinsicCall(c);
        return;
    }
    if (c.fFunction.fBuiltin && "atan" == c.fFunction.fName && 2 == c.fArguments.size()) {
        this->write("atan2");
    } else if (c.fFunction.fBuiltin && "inversesqrt" == c.fFunction.fName) {
        this->write("rsqrt");
    } else if (c.fFunction.fBuiltin && "inverse" == c.fFunction.fName) {
        SkASSERT(c.fArguments.size() == 1);
        this->writeInverseHack(*c.fArguments[0]);
    } else if (c.fFunction.fBuiltin && "dFdx" == c.fFunction.fName) {
        this->write("dfdx");
    } else if (c.fFunction.fBuiltin && "dFdy" == c.fFunction.fName) {
        // Flipping Y also negates the Y derivatives.
        this->write((fProgram.fSettings.fFlipY) ? "-dfdy" : "dfdy");
    } else {
        this->writeName(c.fFunction.fName);
    }
    this->write("(");
    const char* separator = "";
    if (this->requirements(c.fFunction) & kInputs_Requirement) {
        this->write("_in");
        separator = ", ";
    }
    if (this->requirements(c.fFunction) & kOutputs_Requirement) {
        this->write(separator);
        this->write("_out");
        separator = ", ";
    }
    if (this->requirements(c.fFunction) & kUniforms_Requirement) {
        this->write(separator);
        this->write("_uniforms");
        separator = ", ";
    }
    if (this->requirements(c.fFunction) & kGlobals_Requirement) {
        this->write(separator);
        this->write("_globals");
        separator = ", ";
    }
    if (this->requirements(c.fFunction) & kFragCoord_Requirement) {
        this->write(separator);
        this->write("_fragCoord");
        separator = ", ";
    }
    for (size_t i = 0; i < c.fArguments.size(); ++i) {
        const Expression& arg = *c.fArguments[i];
        this->write(separator);
        separator = ", ";
        if (c.fFunction.fParameters[i]->fModifiers.fFlags & Modifiers::kOut_Flag) {
            this->write("&");
        }
        this->writeExpression(arg, kSequence_Precedence);
    }
    this->write(")");
}

void MetalCodeGenerator::writeInverseHack(const Expression& mat) {
    String typeName = mat.fType.name();
    String name = typeName + "_inverse";
    if (mat.fType == *fContext.fFloat2x2_Type || mat.fType == *fContext.fHalf2x2_Type) {
        if (fWrittenIntrinsics.find(name) == fWrittenIntrinsics.end()) {
            fWrittenIntrinsics.insert(name);
            fExtraFunctions.writeText((
                typeName + " " + name + "(" + typeName + " m) {"
                "    return float2x2(m[1][1], -m[0][1], -m[1][0], m[0][0]) * (1/determinant(m));"
                "}"
            ).c_str());
        }
    }
    else if (mat.fType == *fContext.fFloat3x3_Type || mat.fType == *fContext.fHalf3x3_Type) {
        if (fWrittenIntrinsics.find(name) == fWrittenIntrinsics.end()) {
            fWrittenIntrinsics.insert(name);
            fExtraFunctions.writeText((
                typeName + " " +  name + "(" + typeName + " m) {"
                "    float a00 = m[0][0], a01 = m[0][1], a02 = m[0][2];"
                "    float a10 = m[1][0], a11 = m[1][1], a12 = m[1][2];"
                "    float a20 = m[2][0], a21 = m[2][1], a22 = m[2][2];"
                "    float b01 = a22 * a11 - a12 * a21;"
                "    float b11 = -a22 * a10 + a12 * a20;"
                "    float b21 = a21 * a10 - a11 * a20;"
                "    float det = a00 * b01 + a01 * b11 + a02 * b21;"
                "    return " + typeName +
                "                   (b01, (-a22 * a01 + a02 * a21), (a12 * a01 - a02 * a11),"
                "                    b11, (a22 * a00 - a02 * a20), (-a12 * a00 + a02 * a10),"
                "                    b21, (-a21 * a00 + a01 * a20), (a11 * a00 - a01 * a10)) * "
                "                   (1/det);"
                "}"
            ).c_str());
        }
    }
    else if (mat.fType == *fContext.fFloat4x4_Type || mat.fType == *fContext.fHalf4x4_Type) {
        if (fWrittenIntrinsics.find(name) == fWrittenIntrinsics.end()) {
            fWrittenIntrinsics.insert(name);
            fExtraFunctions.writeText((
                typeName + " " +  name + "(" + typeName + " m) {"
                "    float a00 = m[0][0], a01 = m[0][1], a02 = m[0][2], a03 = m[0][3];"
                "    float a10 = m[1][0], a11 = m[1][1], a12 = m[1][2], a13 = m[1][3];"
                "    float a20 = m[2][0], a21 = m[2][1], a22 = m[2][2], a23 = m[2][3];"
                "    float a30 = m[3][0], a31 = m[3][1], a32 = m[3][2], a33 = m[3][3];"
                "    float b00 = a00 * a11 - a01 * a10;"
                "    float b01 = a00 * a12 - a02 * a10;"
                "    float b02 = a00 * a13 - a03 * a10;"
                "    float b03 = a01 * a12 - a02 * a11;"
                "    float b04 = a01 * a13 - a03 * a11;"
                "    float b05 = a02 * a13 - a03 * a12;"
                "    float b06 = a20 * a31 - a21 * a30;"
                "    float b07 = a20 * a32 - a22 * a30;"
                "    float b08 = a20 * a33 - a23 * a30;"
                "    float b09 = a21 * a32 - a22 * a31;"
                "    float b10 = a21 * a33 - a23 * a31;"
                "    float b11 = a22 * a33 - a23 * a32;"
                "    float det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - "
                "                b04 * b07 + b05 * b06;"
                "    return " + typeName + "(a11 * b11 - a12 * b10 + a13 * b09,"
                "                            a02 * b10 - a01 * b11 - a03 * b09,"
                "                            a31 * b05 - a32 * b04 + a33 * b03,"
                "                            a22 * b04 - a21 * b05 - a23 * b03,"
                "                            a12 * b08 - a10 * b11 - a13 * b07,"
                "                            a00 * b11 - a02 * b08 + a03 * b07,"
                "                            a32 * b02 - a30 * b05 - a33 * b01,"
                "                            a20 * b05 - a22 * b02 + a23 * b01,"
                "                            a10 * b10 - a11 * b08 + a13 * b06,"
                "                            a01 * b08 - a00 * b10 - a03 * b06,"
                "                            a30 * b04 - a31 * b02 + a33 * b00,"
                "                            a21 * b02 - a20 * b04 - a23 * b00,"
                "                            a11 * b07 - a10 * b09 - a12 * b06,"
                "                            a00 * b09 - a01 * b07 + a02 * b06,"
                "                            a31 * b01 - a30 * b03 - a32 * b00,"
                "                            a20 * b03 - a21 * b01 + a22 * b00) / det;"
                "}"
            ).c_str());
        }
    }
    this->write(name);
}

void MetalCodeGenerator::writeSpecialIntrinsic(const FunctionCall & c, SpecialIntrinsic kind) {
    switch (kind) {
        case kTexture_SpecialIntrinsic:
            this->writeExpression(*c.fArguments[0], kSequence_Precedence);
            this->write(".sample(");
            this->writeExpression(*c.fArguments[0], kSequence_Precedence);
            this->write(SAMPLER_SUFFIX);
            this->write(", ");
            if (c.fArguments[1]->fType == *fContext.fFloat3_Type) {
                // have to store the vector in a temp variable to avoid double evaluating it
                String tmpVar = "tmpCoord" + to_string(fVarCount++);
                this->fFunctionHeader += "    " + this->typeName(c.fArguments[1]->fType) + " " +
                                         tmpVar + ";\n";
                this->write("(" + tmpVar + " = ");
                this->writeExpression(*c.fArguments[1], kSequence_Precedence);
                this->write(", " + tmpVar + ".xy / " + tmpVar + ".z))");
            } else {
                SkASSERT(c.fArguments[1]->fType == *fContext.fFloat2_Type);
                this->writeExpression(*c.fArguments[1], kSequence_Precedence);
                this->write(")");
            }
            break;
        case kMod_SpecialIntrinsic: {
            // fmod(x, y) in metal calculates x - y * trunc(x / y) instead of x - y * floor(x / y)
            String tmpX = "tmpX" + to_string(fVarCount++);
            String tmpY = "tmpY" + to_string(fVarCount++);
            this->fFunctionHeader += "    " + this->typeName(c.fArguments[0]->fType) + " " + tmpX +
                                     ", " + tmpY + ";\n";
            this->write("(" + tmpX + " = ");
            this->writeExpression(*c.fArguments[0], kSequence_Precedence);
            this->write(", " + tmpY + " = ");
            this->writeExpression(*c.fArguments[1], kSequence_Precedence);
            this->write(", " + tmpX + " - " + tmpY + " * floor(" + tmpX + " / " + tmpY + "))");
            break;
        }
        default:
            ABORT("unsupported special intrinsic kind");
    }
}

// If it hasn't already been written, writes a constructor for 'matrix' which takes a single value
// of type 'arg'.
String MetalCodeGenerator::getMatrixConstructHelper(const Type& matrix, const Type& arg) {
    String key = matrix.name() + arg.name();
    auto found = fHelpers.find(key);
    if (found != fHelpers.end()) {
        return found->second;
    }
    String name;
    int columns = matrix.columns();
    int rows = matrix.rows();
    if (arg.isNumber()) {
        // creating a matrix from a single scalar value
        name = "float" + to_string(columns) + "x" + to_string(rows) + "_from_float";
        fExtraFunctions.printf("float%dx%d %s(float x) {\n",
                               columns, rows, name.c_str());
        fExtraFunctions.printf("    return float%dx%d(", columns, rows);
        for (int i = 0; i < columns; ++i) {
            if (i > 0) {
                fExtraFunctions.writeText(", ");
            }
            fExtraFunctions.printf("float%d(", rows);
            for (int j = 0; j < rows; ++j) {
                if (j > 0) {
                    fExtraFunctions.writeText(", ");
                }
                if (i == j) {
                    fExtraFunctions.writeText("x");
                } else {
                    fExtraFunctions.writeText("0");
                }
            }
            fExtraFunctions.writeText(")");
        }
        fExtraFunctions.writeText(");\n}\n");
    } else if (arg.kind() == Type::kMatrix_Kind) {
        // creating a matrix from another matrix
        int argColumns = arg.columns();
        int argRows = arg.rows();
        name = "float" + to_string(columns) + "x" + to_string(rows) + "_from_float" +
               to_string(argColumns) + "x" + to_string(argRows);
        fExtraFunctions.printf("float%dx%d %s(float%dx%d m) {\n",
                               columns, rows, name.c_str(), argColumns, argRows);
        fExtraFunctions.printf("    return float%dx%d(", columns, rows);
        for (int i = 0; i < columns; ++i) {
            if (i > 0) {
                fExtraFunctions.writeText(", ");
            }
            fExtraFunctions.printf("float%d(", rows);
            for (int j = 0; j < rows; ++j) {
                if (j > 0) {
                    fExtraFunctions.writeText(", ");
                }
                if (i < argColumns && j < argRows) {
                    fExtraFunctions.printf("m[%d][%d]", i, j);
                } else {
                    fExtraFunctions.writeText("0");
                }
            }
            fExtraFunctions.writeText(")");
        }
        fExtraFunctions.writeText(");\n}\n");
    } else if (matrix.rows() == 2 && matrix.columns() == 2 && arg == *fContext.fFloat4_Type) {
        // float2x2(float4) doesn't work, need to split it into float2x2(float2, float2)
        name = "float2x2_from_float4";
        fExtraFunctions.printf(
            "float2x2 %s(float4 v) {\n"
            "    return float2x2(float2(v[0], v[1]), float2(v[2], v[3]));\n"
            "}\n",
            name.c_str()
        );
    } else {
        SkASSERT(false);
        name = "<error>";
    }
    fHelpers[key] = name;
    return name;
}

bool MetalCodeGenerator::canCoerce(const Type& t1, const Type& t2) {
    if (t1.columns() != t2.columns() || t1.rows() != t2.rows()) {
        return false;
    }
    if (t1.columns() > 1) {
        return this->canCoerce(t1.componentType(), t2.componentType());
    }
    return t1.isFloat() && t2.isFloat();
}

void MetalCodeGenerator::writeConstructor(const Constructor& c, Precedence parentPrecedence) {
    if (c.fArguments.size() == 1 && this->canCoerce(c.fType, c.fArguments[0]->fType)) {
        this->writeExpression(*c.fArguments[0], parentPrecedence);
        return;
    }
    if (c.fType.kind() == Type::kMatrix_Kind && c.fArguments.size() == 1) {
        const Expression& arg = *c.fArguments[0];
        String name = this->getMatrixConstructHelper(c.fType, arg.fType);
        this->write(name);
        this->write("(");
        this->writeExpression(arg, kSequence_Precedence);
        this->write(")");
    } else {
        this->writeType(c.fType);
        this->write("(");
        const char* separator = "";
        int scalarCount = 0;
        for (const auto& arg : c.fArguments) {
            this->write(separator);
            separator = ", ";
            if (Type::kMatrix_Kind == c.fType.kind() && arg->fType.columns() != c.fType.rows()) {
                // merge scalars and smaller vectors together
                if (!scalarCount) {
                    this->writeType(c.fType.componentType());
                    this->write(to_string(c.fType.rows()));
                    this->write("(");
                }
                scalarCount += arg->fType.columns();
            }
            this->writeExpression(*arg, kSequence_Precedence);
            if (scalarCount && scalarCount == c.fType.rows()) {
                this->write(")");
                scalarCount = 0;
            }
        }
        this->write(")");
    }
}

void MetalCodeGenerator::writeFragCoord() {
    if (fRTHeightName.length()) {
        this->write("float4(_fragCoord.x, ");
        this->write(fRTHeightName.c_str());
        this->write(" - _fragCoord.y, 0.0, _fragCoord.w)");
    } else {
        this->write("float4(_fragCoord.x, _fragCoord.y, 0.0, _fragCoord.w)");
    }
}

void MetalCodeGenerator::writeVariableReference(const VariableReference& ref) {
    switch (ref.fVariable.fModifiers.fLayout.fBuiltin) {
        case SK_FRAGCOLOR_BUILTIN:
            this->write("_out->sk_FragColor");
            break;
        case SK_FRAGCOORD_BUILTIN:
            this->writeFragCoord();
            break;
        case SK_VERTEXID_BUILTIN:
            this->write("sk_VertexID");
            break;
        case SK_INSTANCEID_BUILTIN:
            this->write("sk_InstanceID");
            break;
        case SK_CLOCKWISE_BUILTIN:
            // We'd set the front facing winding in the MTLRenderCommandEncoder to be counter
            // clockwise to match Skia convention.
            this->write(fProgram.fSettings.fFlipY ? "_frontFacing" : "(!_frontFacing)");
            break;
        default:
            if (Variable::kGlobal_Storage == ref.fVariable.fStorage) {
                if (ref.fVariable.fModifiers.fFlags & Modifiers::kIn_Flag) {
                    this->write("_in.");
                } else if (ref.fVariable.fModifiers.fFlags & Modifiers::kOut_Flag) {
                    this->write("_out->");
                } else if (ref.fVariable.fModifiers.fFlags & Modifiers::kUniform_Flag &&
                           ref.fVariable.fType.kind() != Type::kSampler_Kind) {
                    this->write("_uniforms.");
                } else {
                    this->write("_globals->");
                }
            }
            this->writeName(ref.fVariable.fName);
    }
}

void MetalCodeGenerator::writeIndexExpression(const IndexExpression& expr) {
    this->writeExpression(*expr.fBase, kPostfix_Precedence);
    this->write("[");
    this->writeExpression(*expr.fIndex, kTopLevel_Precedence);
    this->write("]");
}

void MetalCodeGenerator::writeFieldAccess(const FieldAccess& f) {
    const Type::Field* field = &f.fBase->fType.fields()[f.fFieldIndex];
    if (FieldAccess::kDefault_OwnerKind == f.fOwnerKind) {
        this->writeExpression(*f.fBase, kPostfix_Precedence);
        this->write(".");
    }
    switch (field->fModifiers.fLayout.fBuiltin) {
        case SK_CLIPDISTANCE_BUILTIN:
            this->write("gl_ClipDistance");
            break;
        case SK_POSITION_BUILTIN:
            this->write("_out->sk_Position");
            break;
        default:
            if (field->fName == "sk_PointSize") {
                this->write("_out->sk_PointSize");
            } else {
                if (FieldAccess::kAnonymousInterfaceBlock_OwnerKind == f.fOwnerKind) {
                    this->write("_globals->");
                    this->write(fInterfaceBlockNameMap[fInterfaceBlockMap[field]]);
                    this->write("->");
                }
                this->writeName(field->fName);
            }
    }
}

void MetalCodeGenerator::writeSwizzle(const Swizzle& swizzle) {
    int last = swizzle.fComponents.back();
    if (last == SKSL_SWIZZLE_0 || last == SKSL_SWIZZLE_1) {
        this->writeType(swizzle.fType);
        this->write("(");
    }
    this->writeExpression(*swizzle.fBase, kPostfix_Precedence);
    this->write(".");
    for (int c : swizzle.fComponents) {
        if (c >= 0) {
            this->write(&("x\0y\0z\0w\0"[c * 2]));
        }
    }
    if (last == SKSL_SWIZZLE_0) {
        this->write(", 0)");
    }
    else if (last == SKSL_SWIZZLE_1) {
        this->write(", 1)");
    }
}

MetalCodeGenerator::Precedence MetalCodeGenerator::GetBinaryPrecedence(Token::Kind op) {
    switch (op) {
        case Token::STAR:         // fall through
        case Token::SLASH:        // fall through
        case Token::PERCENT:      return MetalCodeGenerator::kMultiplicative_Precedence;
        case Token::PLUS:         // fall through
        case Token::MINUS:        return MetalCodeGenerator::kAdditive_Precedence;
        case Token::SHL:          // fall through
        case Token::SHR:          return MetalCodeGenerator::kShift_Precedence;
        case Token::LT:           // fall through
        case Token::GT:           // fall through
        case Token::LTEQ:         // fall through
        case Token::GTEQ:         return MetalCodeGenerator::kRelational_Precedence;
        case Token::EQEQ:         // fall through
        case Token::NEQ:          return MetalCodeGenerator::kEquality_Precedence;
        case Token::BITWISEAND:   return MetalCodeGenerator::kBitwiseAnd_Precedence;
        case Token::BITWISEXOR:   return MetalCodeGenerator::kBitwiseXor_Precedence;
        case Token::BITWISEOR:    return MetalCodeGenerator::kBitwiseOr_Precedence;
        case Token::LOGICALAND:   return MetalCodeGenerator::kLogicalAnd_Precedence;
        case Token::LOGICALXOR:   return MetalCodeGenerator::kLogicalXor_Precedence;
        case Token::LOGICALOR:    return MetalCodeGenerator::kLogicalOr_Precedence;
        case Token::EQ:           // fall through
        case Token::PLUSEQ:       // fall through
        case Token::MINUSEQ:      // fall through
        case Token::STAREQ:       // fall through
        case Token::SLASHEQ:      // fall through
        case Token::PERCENTEQ:    // fall through
        case Token::SHLEQ:        // fall through
        case Token::SHREQ:        // fall through
        case Token::LOGICALANDEQ: // fall through
        case Token::LOGICALXOREQ: // fall through
        case Token::LOGICALOREQ:  // fall through
        case Token::BITWISEANDEQ: // fall through
        case Token::BITWISEXOREQ: // fall through
        case Token::BITWISEOREQ:  return MetalCodeGenerator::kAssignment_Precedence;
        case Token::COMMA:        return MetalCodeGenerator::kSequence_Precedence;
        default: ABORT("unsupported binary operator");
    }
}

void MetalCodeGenerator::writeMatrixTimesEqualHelper(const Type& left, const Type& right,
                                                     const Type& result) {
    String key = "TimesEqual" + left.name() + right.name();
    if (fHelpers.find(key) == fHelpers.end()) {
        fExtraFunctions.printf("%s operator*=(thread %s& left, thread const %s& right) {\n"
                               "    left = left * right;\n"
                               "    return left;\n"
                               "}", result.name().c_str(), left.name().c_str(),
                                    right.name().c_str());
    }
}

void MetalCodeGenerator::writeBinaryExpression(const BinaryExpression& b,
                                               Precedence parentPrecedence) {
    Precedence precedence = GetBinaryPrecedence(b.fOperator);
    bool needParens = precedence >= parentPrecedence;
    switch (b.fOperator) {
        case Token::EQEQ:
            if (b.fLeft->fType.kind() == Type::kVector_Kind) {
                this->write("all");
                needParens = true;
            }
            break;
        case Token::NEQ:
            if (b.fLeft->fType.kind() == Type::kVector_Kind) {
                this->write("any");
                needParens = true;
            }
            break;
        default:
            break;
    }
    if (needParens) {
        this->write("(");
    }
    if (Compiler::IsAssignment(b.fOperator) &&
        Expression::kVariableReference_Kind == b.fLeft->fKind &&
        Variable::kParameter_Storage == ((VariableReference&) *b.fLeft).fVariable.fStorage &&
        (((VariableReference&) *b.fLeft).fVariable.fModifiers.fFlags & Modifiers::kOut_Flag)) {
        // writing to an out parameter. Since we have to turn those into pointers, we have to
        // dereference it here.
        this->write("*");
    }
    if (b.fOperator == Token::STAREQ && b.fLeft->fType.kind() == Type::kMatrix_Kind &&
        b.fRight->fType.kind() == Type::kMatrix_Kind) {
        this->writeMatrixTimesEqualHelper(b.fLeft->fType, b.fRight->fType, b.fType);
    }
    this->writeExpression(*b.fLeft, precedence);
    if (b.fOperator != Token::EQ && Compiler::IsAssignment(b.fOperator) &&
        Expression::kSwizzle_Kind == b.fLeft->fKind && !b.fLeft->hasSideEffects()) {
        // This doesn't compile in Metal:
        // float4 x = float4(1);
        // x.xy *= float2x2(...);
        // with the error message "non-const reference cannot bind to vector element",
        // but switching it to x.xy = x.xy * float2x2(...) fixes it. We perform this tranformation
        // as long as the LHS has no side effects, and hope for the best otherwise.
        this->write(" = ");
        this->writeExpression(*b.fLeft, kAssignment_Precedence);
        this->write(" ");
        String op = Compiler::OperatorName(b.fOperator);
        SkASSERT(op.endsWith("="));
        this->write(op.substr(0, op.size() - 1).c_str());
        this->write(" ");
    } else {
        this->write(String(" ") + Compiler::OperatorName(b.fOperator) + " ");
    }
    this->writeExpression(*b.fRight, precedence);
    if (needParens) {
        this->write(")");
    }
}

void MetalCodeGenerator::writeTernaryExpression(const TernaryExpression& t,
                                               Precedence parentPrecedence) {
    if (kTernary_Precedence >= parentPrecedence) {
        this->write("(");
    }
    this->writeExpression(*t.fTest, kTernary_Precedence);
    this->write(" ? ");
    this->writeExpression(*t.fIfTrue, kTernary_Precedence);
    this->write(" : ");
    this->writeExpression(*t.fIfFalse, kTernary_Precedence);
    if (kTernary_Precedence >= parentPrecedence) {
        this->write(")");
    }
}

void MetalCodeGenerator::writePrefixExpression(const PrefixExpression& p,
                                              Precedence parentPrecedence) {
    if (kPrefix_Precedence >= parentPrecedence) {
        this->write("(");
    }
    this->write(Compiler::OperatorName(p.fOperator));
    this->writeExpression(*p.fOperand, kPrefix_Precedence);
    if (kPrefix_Precedence >= parentPrecedence) {
        this->write(")");
    }
}

void MetalCodeGenerator::writePostfixExpression(const PostfixExpression& p,
                                               Precedence parentPrecedence) {
    if (kPostfix_Precedence >= parentPrecedence) {
        this->write("(");
    }
    this->writeExpression(*p.fOperand, kPostfix_Precedence);
    this->write(Compiler::OperatorName(p.fOperator));
    if (kPostfix_Precedence >= parentPrecedence) {
        this->write(")");
    }
}

void MetalCodeGenerator::writeBoolLiteral(const BoolLiteral& b) {
    this->write(b.fValue ? "true" : "false");
}

void MetalCodeGenerator::writeIntLiteral(const IntLiteral& i) {
    if (i.fType == *fContext.fUInt_Type) {
        this->write(to_string(i.fValue & 0xffffffff) + "u");
    } else {
        this->write(to_string((int32_t) i.fValue));
    }
}

void MetalCodeGenerator::writeFloatLiteral(const FloatLiteral& f) {
    this->write(to_string(f.fValue));
}

void MetalCodeGenerator::writeSetting(const Setting& s) {
    ABORT("internal error; setting was not folded to a constant during compilation\n");
}

void MetalCodeGenerator::writeFunction(const FunctionDefinition& f) {
    fRTHeightName = fProgram.fInputs.fRTHeight ? "_globals->_anonInterface0->u_skRTHeight" : "";
    const char* separator = "";
    if ("main" == f.fDeclaration.fName) {
        switch (fProgram.fKind) {
            case Program::kFragment_Kind:
                this->write("fragment Outputs fragmentMain");
                break;
            case Program::kVertex_Kind:
                this->write("vertex Outputs vertexMain");
                break;
            default:
                SkASSERT(false);
        }
        this->write("(Inputs _in [[stage_in]]");
        if (-1 != fUniformBuffer) {
            this->write(", constant Uniforms& _uniforms [[buffer(" +
                        to_string(fUniformBuffer) + ")]]");
        }
        for (const auto& e : fProgram) {
            if (ProgramElement::kVar_Kind == e.fKind) {
                VarDeclarations& decls = (VarDeclarations&) e;
                if (!decls.fVars.size()) {
                    continue;
                }
                for (const auto& stmt: decls.fVars) {
                    VarDeclaration& var = (VarDeclaration&) *stmt;
                    if (var.fVar->fType.kind() == Type::kSampler_Kind) {
                        this->write(", texture2d<float> "); // FIXME - support other texture types
                        this->writeName(var.fVar->fName);
                        this->write("[[texture(");
                        this->write(to_string(var.fVar->fModifiers.fLayout.fBinding));
                        this->write(")]]");
                        this->write(", sampler ");
                        this->writeName(var.fVar->fName);
                        this->write(SAMPLER_SUFFIX);
                        this->write("[[sampler(");
                        this->write(to_string(var.fVar->fModifiers.fLayout.fBinding));
                        this->write(")]]");
                    }
                }
            } else if (ProgramElement::kInterfaceBlock_Kind == e.fKind) {
                InterfaceBlock& intf = (InterfaceBlock&) e;
                if ("sk_PerVertex" == intf.fTypeName) {
                    continue;
                }
                this->write(", constant ");
                this->writeType(intf.fVariable.fType);
                this->write("& " );
                this->write(fInterfaceBlockNameMap[&intf]);
                this->write(" [[buffer(");
                this->write(to_string(intf.fVariable.fModifiers.fLayout.fBinding));
                this->write(")]]");
            }
        }
        if (fProgram.fKind == Program::kFragment_Kind) {
            if (fProgram.fInputs.fRTHeight && fInterfaceBlockNameMap.empty()) {
                this->write(", constant sksl_synthetic_uniforms& _anonInterface0 [[buffer(1)]]");
                fRTHeightName = "_anonInterface0.u_skRTHeight";
            }
            this->write(", bool _frontFacing [[front_facing]]");
            this->write(", float4 _fragCoord [[position]]");
        } else if (fProgram.fKind == Program::kVertex_Kind) {
            this->write(", uint sk_VertexID [[vertex_id]], uint sk_InstanceID [[instance_id]]");
        }
        separator = ", ";
    } else {
        this->writeType(f.fDeclaration.fReturnType);
        this->write(" ");
        this->writeName(f.fDeclaration.fName);
        this->write("(");
        Requirements requirements = this->requirements(f.fDeclaration);
        if (requirements & kInputs_Requirement) {
            this->write("Inputs _in");
            separator = ", ";
        }
        if (requirements & kOutputs_Requirement) {
            this->write(separator);
            this->write("thread Outputs* _out");
            separator = ", ";
        }
        if (requirements & kUniforms_Requirement) {
            this->write(separator);
            this->write("Uniforms _uniforms");
            separator = ", ";
        }
        if (requirements & kGlobals_Requirement) {
            this->write(separator);
            this->write("thread Globals* _globals");
            separator = ", ";
        }
        if (requirements & kFragCoord_Requirement) {
            this->write(separator);
            this->write("float4 _fragCoord");
            separator = ", ";
        }
    }
    for (const auto& param : f.fDeclaration.fParameters) {
        this->write(separator);
        separator = ", ";
        this->writeModifiers(param->fModifiers, false);
        std::vector<int> sizes;
        const Type* type = &param->fType;
        while (Type::kArray_Kind == type->kind()) {
            sizes.push_back(type->columns());
            type = &type->componentType();
        }
        this->writeType(*type);
        if (param->fModifiers.fFlags & Modifiers::kOut_Flag) {
            this->write("*");
        }
        this->write(" ");
        this->writeName(param->fName);
        for (int s : sizes) {
            if (s <= 0) {
                this->write("[]");
            } else {
                this->write("[" + to_string(s) + "]");
            }
        }
    }
    this->writeLine(") {");

    SkASSERT(!fProgram.fSettings.fFragColorIsInOut);

    if ("main" == f.fDeclaration.fName) {
        if (fNeedsGlobalStructInit) {
            this->writeLine("    Globals globalStruct{");
            const char* separator = "";
            for (const auto& intf: fInterfaceBlockNameMap) {
                const auto& intfName = intf.second;
                this->write(separator);
                separator = ", ";
                this->write("&");
                this->writeName(intfName);
            }
            for (const auto& var: fInitNonConstGlobalVars) {
                this->write(separator);
                separator = ", ";
                this->writeVarInitializer(*var->fVar, *var->fValue);
            }
            for (const auto& texture: fTextures) {
                this->write(separator);
                separator = ", ";
                this->writeName(texture->fName);
                this->write(separator);
                this->writeName(texture->fName);
                this->write(SAMPLER_SUFFIX);
            }
            this->writeLine("};");
            this->writeLine("    thread Globals* _globals = &globalStruct;");
            this->writeLine("    (void)_globals;");
        }
        this->writeLine("    Outputs _outputStruct;");
        this->writeLine("    thread Outputs* _out = &_outputStruct;");
    }
    fFunctionHeader = "";
    OutputStream* oldOut = fOut;
    StringStream buffer;
    fOut = &buffer;
    fIndentation++;
    this->writeStatements(((Block&) *f.fBody).fStatements);
    if ("main" == f.fDeclaration.fName) {
        switch (fProgram.fKind) {
            case Program::kFragment_Kind:
                this->writeLine("return *_out;");
                break;
            case Program::kVertex_Kind:
                this->writeLine("_out->sk_Position.y = -_out->sk_Position.y;");
                this->writeLine("return *_out;"); // FIXME - detect if function already has return
                break;
            default:
                SkASSERT(false);
        }
    }
    fIndentation--;
    this->writeLine("}");

    fOut = oldOut;
    this->write(fFunctionHeader);
    this->write(buffer.str());
}

void MetalCodeGenerator::writeModifiers(const Modifiers& modifiers,
                                       bool globalContext) {
    if (modifiers.fFlags & Modifiers::kOut_Flag) {
        this->write("thread ");
    }
    if (modifiers.fFlags & Modifiers::kConst_Flag) {
        this->write("constant ");
    }
}

void MetalCodeGenerator::writeInterfaceBlock(const InterfaceBlock& intf) {
    if ("sk_PerVertex" == intf.fTypeName) {
        return;
    }
    this->writeModifiers(intf.fVariable.fModifiers, true);
    this->write("struct ");
    this->writeLine(intf.fTypeName + " {");
    const Type* structType = &intf.fVariable.fType;
    fWrittenStructs.push_back(structType);
    while (Type::kArray_Kind == structType->kind()) {
        structType = &structType->componentType();
    }
    fIndentation++;
    writeFields(structType->fields(), structType->fOffset, &intf);
    if (fProgram.fInputs.fRTHeight) {
        this->writeLine("float u_skRTHeight;");
    }
    fIndentation--;
    this->write("}");
    if (intf.fInstanceName.size()) {
        this->write(" ");
        this->write(intf.fInstanceName);
        for (const auto& size : intf.fSizes) {
            this->write("[");
            if (size) {
                this->writeExpression(*size, kTopLevel_Precedence);
            }
            this->write("]");
        }
        fInterfaceBlockNameMap[&intf] = intf.fInstanceName;
    } else {
        fInterfaceBlockNameMap[&intf] = "_anonInterface" +  to_string(fAnonInterfaceCount++);
    }
    this->writeLine(";");
}

void MetalCodeGenerator::writeFields(const std::vector<Type::Field>& fields, int parentOffset,
                                     const InterfaceBlock* parentIntf) {
    MemoryLayout memoryLayout(MemoryLayout::kMetal_Standard);
    int currentOffset = 0;
    for (const auto& field: fields) {
        int fieldOffset = field.fModifiers.fLayout.fOffset;
        const Type* fieldType = field.fType;
        if (fieldOffset != -1) {
            if (currentOffset > fieldOffset) {
                fErrors.error(parentOffset,
                                "offset of field '" + field.fName + "' must be at least " +
                                to_string((int) currentOffset));
            } else if (currentOffset < fieldOffset) {
                this->write("char pad");
                this->write(to_string(fPaddingCount++));
                this->write("[");
                this->write(to_string(fieldOffset - currentOffset));
                this->writeLine("];");
                currentOffset = fieldOffset;
            }
            int alignment = memoryLayout.alignment(*fieldType);
            if (fieldOffset % alignment) {
                fErrors.error(parentOffset,
                              "offset of field '" + field.fName + "' must be a multiple of " +
                              to_string((int) alignment));
            }
        }
        currentOffset += memoryLayout.size(*fieldType);
        std::vector<int> sizes;
        while (fieldType->kind() == Type::kArray_Kind) {
            sizes.push_back(fieldType->columns());
            fieldType = &fieldType->componentType();
        }
        this->writeModifiers(field.fModifiers, false);
        this->writeType(*fieldType);
        this->write(" ");
        this->writeName(field.fName);
        for (int s : sizes) {
            if (s <= 0) {
                this->write("[]");
            } else {
                this->write("[" + to_string(s) + "]");
            }
        }
        this->writeLine(";");
        if (parentIntf) {
            fInterfaceBlockMap[&field] = parentIntf;
        }
    }
}

void MetalCodeGenerator::writeVarInitializer(const Variable& var, const Expression& value) {
    this->writeExpression(value, kTopLevel_Precedence);
}

void MetalCodeGenerator::writeName(const String& name) {
    if (fReservedWords.find(name) != fReservedWords.end()) {
        this->write("_"); // adding underscore before name to avoid conflict with reserved words
    }
    this->write(name);
}

void MetalCodeGenerator::writeVarDeclarations(const VarDeclarations& decl, bool global) {
    SkASSERT(decl.fVars.size() > 0);
    bool wroteType = false;
    for (const auto& stmt : decl.fVars) {
        VarDeclaration& var = (VarDeclaration&) *stmt;
        if (global && !(var.fVar->fModifiers.fFlags & Modifiers::kConst_Flag)) {
            continue;
        }
        if (wroteType) {
            this->write(", ");
        } else {
            this->writeModifiers(var.fVar->fModifiers, global);
            this->writeType(decl.fBaseType);
            this->write(" ");
            wroteType = true;
        }
        this->writeName(var.fVar->fName);
        for (const auto& size : var.fSizes) {
            this->write("[");
            if (size) {
                this->writeExpression(*size, kTopLevel_Precedence);
            }
            this->write("]");
        }
        if (var.fValue) {
            this->write(" = ");
            this->writeVarInitializer(*var.fVar, *var.fValue);
        }
    }
    if (wroteType) {
        this->write(";");
    }
}

void MetalCodeGenerator::writeStatement(const Statement& s) {
    switch (s.fKind) {
        case Statement::kBlock_Kind:
            this->writeBlock((Block&) s);
            break;
        case Statement::kExpression_Kind:
            this->writeExpression(*((ExpressionStatement&) s).fExpression, kTopLevel_Precedence);
            this->write(";");
            break;
        case Statement::kReturn_Kind:
            this->writeReturnStatement((ReturnStatement&) s);
            break;
        case Statement::kVarDeclarations_Kind:
            this->writeVarDeclarations(*((VarDeclarationsStatement&) s).fDeclaration, false);
            break;
        case Statement::kIf_Kind:
            this->writeIfStatement((IfStatement&) s);
            break;
        case Statement::kFor_Kind:
            this->writeForStatement((ForStatement&) s);
            break;
        case Statement::kWhile_Kind:
            this->writeWhileStatement((WhileStatement&) s);
            break;
        case Statement::kDo_Kind:
            this->writeDoStatement((DoStatement&) s);
            break;
        case Statement::kSwitch_Kind:
            this->writeSwitchStatement((SwitchStatement&) s);
            break;
        case Statement::kBreak_Kind:
            this->write("break;");
            break;
        case Statement::kContinue_Kind:
            this->write("continue;");
            break;
        case Statement::kDiscard_Kind:
            this->write("discard_fragment();");
            break;
        case Statement::kNop_Kind:
            this->write(";");
            break;
        default:
#ifdef SK_DEBUG
            ABORT("unsupported statement: %s", s.description().c_str());
#endif
            break;
    }
}

void MetalCodeGenerator::writeStatements(const std::vector<std::unique_ptr<Statement>>& statements) {
    for (const auto& s : statements) {
        if (!s->isEmpty()) {
            this->writeStatement(*s);
            this->writeLine();
        }
    }
}

void MetalCodeGenerator::writeBlock(const Block& b) {
    this->writeLine("{");
    fIndentation++;
    this->writeStatements(b.fStatements);
    fIndentation--;
    this->write("}");
}

void MetalCodeGenerator::writeIfStatement(const IfStatement& stmt) {
    this->write("if (");
    this->writeExpression(*stmt.fTest, kTopLevel_Precedence);
    this->write(") ");
    this->writeStatement(*stmt.fIfTrue);
    if (stmt.fIfFalse) {
        this->write(" else ");
        this->writeStatement(*stmt.fIfFalse);
    }
}

void MetalCodeGenerator::writeForStatement(const ForStatement& f) {
    this->write("for (");
    if (f.fInitializer && !f.fInitializer->isEmpty()) {
        this->writeStatement(*f.fInitializer);
    } else {
        this->write("; ");
    }
    if (f.fTest) {
        this->writeExpression(*f.fTest, kTopLevel_Precedence);
    }
    this->write("; ");
    if (f.fNext) {
        this->writeExpression(*f.fNext, kTopLevel_Precedence);
    }
    this->write(") ");
    this->writeStatement(*f.fStatement);
}

void MetalCodeGenerator::writeWhileStatement(const WhileStatement& w) {
    this->write("while (");
    this->writeExpression(*w.fTest, kTopLevel_Precedence);
    this->write(") ");
    this->writeStatement(*w.fStatement);
}

void MetalCodeGenerator::writeDoStatement(const DoStatement& d) {
    this->write("do ");
    this->writeStatement(*d.fStatement);
    this->write(" while (");
    this->writeExpression(*d.fTest, kTopLevel_Precedence);
    this->write(");");
}

void MetalCodeGenerator::writeSwitchStatement(const SwitchStatement& s) {
    this->write("switch (");
    this->writeExpression(*s.fValue, kTopLevel_Precedence);
    this->writeLine(") {");
    fIndentation++;
    for (const auto& c : s.fCases) {
        if (c->fValue) {
            this->write("case ");
            this->writeExpression(*c->fValue, kTopLevel_Precedence);
            this->writeLine(":");
        } else {
            this->writeLine("default:");
        }
        fIndentation++;
        for (const auto& stmt : c->fStatements) {
            this->writeStatement(*stmt);
            this->writeLine();
        }
        fIndentation--;
    }
    fIndentation--;
    this->write("}");
}

void MetalCodeGenerator::writeReturnStatement(const ReturnStatement& r) {
    this->write("return");
    if (r.fExpression) {
        this->write(" ");
        this->writeExpression(*r.fExpression, kTopLevel_Precedence);
    }
    this->write(";");
}

void MetalCodeGenerator::writeHeader() {
    this->write("#include <metal_stdlib>\n");
    this->write("#include <simd/simd.h>\n");
    this->write("using namespace metal;\n");
}

void MetalCodeGenerator::writeUniformStruct() {
    for (const auto& e : fProgram) {
        if (ProgramElement::kVar_Kind == e.fKind) {
            VarDeclarations& decls = (VarDeclarations&) e;
            if (!decls.fVars.size()) {
                continue;
            }
            const Variable& first = *((VarDeclaration&) *decls.fVars[0]).fVar;
            if (first.fModifiers.fFlags & Modifiers::kUniform_Flag &&
                first.fType.kind() != Type::kSampler_Kind) {
                if (-1 == fUniformBuffer) {
                    this->write("struct Uniforms {\n");
                    fUniformBuffer = first.fModifiers.fLayout.fSet;
                    if (-1 == fUniformBuffer) {
                        fErrors.error(decls.fOffset, "Metal uniforms must have 'layout(set=...)'");
                    }
                } else if (first.fModifiers.fLayout.fSet != fUniformBuffer) {
                    if (-1 == fUniformBuffer) {
                        fErrors.error(decls.fOffset, "Metal backend requires all uniforms to have "
                                    "the same 'layout(set=...)'");
                    }
                }
                this->write("    ");
                this->writeType(first.fType);
                this->write(" ");
                for (const auto& stmt : decls.fVars) {
                    VarDeclaration& var = (VarDeclaration&) *stmt;
                    this->writeName(var.fVar->fName);
                }
                this->write(";\n");
            }
        }
    }
    if (-1 != fUniformBuffer) {
        this->write("};\n");
    }
}

void MetalCodeGenerator::writeInputStruct() {
    this->write("struct Inputs {\n");
    for (const auto& e : fProgram) {
        if (ProgramElement::kVar_Kind == e.fKind) {
            VarDeclarations& decls = (VarDeclarations&) e;
            if (!decls.fVars.size()) {
                continue;
            }
            const Variable& first = *((VarDeclaration&) *decls.fVars[0]).fVar;
            if (first.fModifiers.fFlags & Modifiers::kIn_Flag &&
                -1 == first.fModifiers.fLayout.fBuiltin) {
                this->write("    ");
                this->writeType(first.fType);
                this->write(" ");
                for (const auto& stmt : decls.fVars) {
                    VarDeclaration& var = (VarDeclaration&) *stmt;
                    this->writeName(var.fVar->fName);
                    if (-1 != var.fVar->fModifiers.fLayout.fLocation) {
                        if (fProgram.fKind == Program::kVertex_Kind) {
                            this->write("  [[attribute(" +
                                        to_string(var.fVar->fModifiers.fLayout.fLocation) + ")]]");
                        } else if (fProgram.fKind == Program::kFragment_Kind) {
                            this->write("  [[user(locn" +
                                        to_string(var.fVar->fModifiers.fLayout.fLocation) + ")]]");
                        }
                    }
                }
                this->write(";\n");
            }
        }
    }
    this->write("};\n");
}

void MetalCodeGenerator::writeOutputStruct() {
    this->write("struct Outputs {\n");
    if (fProgram.fKind == Program::kVertex_Kind) {
        this->write("    float4 sk_Position [[position]];\n");
    } else if (fProgram.fKind == Program::kFragment_Kind) {
        this->write("    float4 sk_FragColor [[color(0)]];\n");
    }
    for (const auto& e : fProgram) {
        if (ProgramElement::kVar_Kind == e.fKind) {
            VarDeclarations& decls = (VarDeclarations&) e;
            if (!decls.fVars.size()) {
                continue;
            }
            const Variable& first = *((VarDeclaration&) *decls.fVars[0]).fVar;
            if (first.fModifiers.fFlags & Modifiers::kOut_Flag &&
                -1 == first.fModifiers.fLayout.fBuiltin) {
                this->write("    ");
                this->writeType(first.fType);
                this->write(" ");
                for (const auto& stmt : decls.fVars) {
                    VarDeclaration& var = (VarDeclaration&) *stmt;
                    this->writeName(var.fVar->fName);
                    if (fProgram.fKind == Program::kVertex_Kind) {
                        this->write("  [[user(locn" +
                                    to_string(var.fVar->fModifiers.fLayout.fLocation) + ")]]");
                    } else if (fProgram.fKind == Program::kFragment_Kind) {
                        this->write(" [[color(" +
                                    to_string(var.fVar->fModifiers.fLayout.fLocation) +")");
                        int colorIndex = var.fVar->fModifiers.fLayout.fIndex;
                        if (colorIndex) {
                            this->write(", index(" + to_string(colorIndex) + ")");
                        }
                        this->write("]]");
                    }
                }
                this->write(";\n");
            }
        }
    }
    if (fProgram.fKind == Program::kVertex_Kind) {
        this->write("    float sk_PointSize;\n");
    }
    this->write("};\n");
}

void MetalCodeGenerator::writeInterfaceBlocks() {
    bool wroteInterfaceBlock = false;
    for (const auto& e : fProgram) {
        if (ProgramElement::kInterfaceBlock_Kind == e.fKind) {
            this->writeInterfaceBlock((InterfaceBlock&) e);
            wroteInterfaceBlock = true;
        }
    }
    if (!wroteInterfaceBlock && fProgram.fInputs.fRTHeight) {
        this->writeLine("struct sksl_synthetic_uniforms {");
        this->writeLine("    float u_skRTHeight;");
        this->writeLine("};");
    }
}

void MetalCodeGenerator::writeGlobalStruct() {
    bool wroteStructDecl = false;
    for (const auto& intf : fInterfaceBlockNameMap) {
        if (!wroteStructDecl) {
            this->write("struct Globals {\n");
            wroteStructDecl = true;
        }
        fNeedsGlobalStructInit = true;
        const auto& intfType = intf.first;
        const auto& intfName = intf.second;
        this->write("    constant ");
        this->write(intfType->fTypeName);
        this->write("* ");
        this->writeName(intfName);
        this->write(";\n");
    }
    for (const auto& e : fProgram) {
        if (ProgramElement::kVar_Kind == e.fKind) {
            VarDeclarations& decls = (VarDeclarations&) e;
            if (!decls.fVars.size()) {
                continue;
            }
            const Variable& first = *((VarDeclaration&) *decls.fVars[0]).fVar;
            if ((!first.fModifiers.fFlags && -1 == first.fModifiers.fLayout.fBuiltin) ||
                first.fType.kind() == Type::kSampler_Kind) {
                if (!wroteStructDecl) {
                    this->write("struct Globals {\n");
                    wroteStructDecl = true;
                }
                fNeedsGlobalStructInit = true;
                this->write("    ");
                this->writeType(first.fType);
                this->write(" ");
                for (const auto& stmt : decls.fVars) {
                    VarDeclaration& var = (VarDeclaration&) *stmt;
                    this->writeName(var.fVar->fName);
                    if (var.fVar->fType.kind() == Type::kSampler_Kind) {
                        fTextures.push_back(var.fVar);
                        this->write(";\n");
                        this->write("    sampler ");
                        this->writeName(var.fVar->fName);
                        this->write(SAMPLER_SUFFIX);
                    }
                    if (var.fValue) {
                        fInitNonConstGlobalVars.push_back(&var);
                    }
                }
                this->write(";\n");
            }
        }
    }
    if (wroteStructDecl) {
        this->write("};\n");
    }
}

void MetalCodeGenerator::writeProgramElement(const ProgramElement& e) {
    switch (e.fKind) {
        case ProgramElement::kExtension_Kind:
            break;
        case ProgramElement::kVar_Kind: {
            VarDeclarations& decl = (VarDeclarations&) e;
            if (decl.fVars.size() > 0) {
                int builtin = ((VarDeclaration&) *decl.fVars[0]).fVar->fModifiers.fLayout.fBuiltin;
                if (-1 == builtin) {
                    // normal var
                    this->writeVarDeclarations(decl, true);
                    this->writeLine();
                } else if (SK_FRAGCOLOR_BUILTIN == builtin) {
                    // ignore
                }
            }
            break;
        }
        case ProgramElement::kInterfaceBlock_Kind:
            // handled in writeInterfaceBlocks, do nothing
            break;
        case ProgramElement::kFunction_Kind:
            this->writeFunction((FunctionDefinition&) e);
            break;
        case ProgramElement::kModifiers_Kind:
            this->writeModifiers(((ModifiersDeclaration&) e).fModifiers, true);
            this->writeLine(";");
            break;
        default:
#ifdef SK_DEBUG
            ABORT("unsupported program element: %s\n", e.description().c_str());
#endif
            break;
    }
}

MetalCodeGenerator::Requirements MetalCodeGenerator::requirements(const Expression& e) {
    switch (e.fKind) {
        case Expression::kFunctionCall_Kind: {
            const FunctionCall& f = (const FunctionCall&) e;
            Requirements result = this->requirements(f.fFunction);
            for (const auto& e : f.fArguments) {
                result |= this->requirements(*e);
            }
            return result;
        }
        case Expression::kConstructor_Kind: {
            const Constructor& c = (const Constructor&) e;
            Requirements result = kNo_Requirements;
            for (const auto& e : c.fArguments) {
                result |= this->requirements(*e);
            }
            return result;
        }
        case Expression::kFieldAccess_Kind: {
            const FieldAccess& f = (const FieldAccess&) e;
            if (FieldAccess::kAnonymousInterfaceBlock_OwnerKind == f.fOwnerKind) {
                return kGlobals_Requirement;
            }
            return this->requirements(*((const FieldAccess&) e).fBase);
        }
        case Expression::kSwizzle_Kind:
            return this->requirements(*((const Swizzle&) e).fBase);
        case Expression::kBinary_Kind: {
            const BinaryExpression& b = (const BinaryExpression&) e;
            return this->requirements(*b.fLeft) | this->requirements(*b.fRight);
        }
        case Expression::kIndex_Kind: {
            const IndexExpression& idx = (const IndexExpression&) e;
            return this->requirements(*idx.fBase) | this->requirements(*idx.fIndex);
        }
        case Expression::kPrefix_Kind:
            return this->requirements(*((const PrefixExpression&) e).fOperand);
        case Expression::kPostfix_Kind:
            return this->requirements(*((const PostfixExpression&) e).fOperand);
        case Expression::kTernary_Kind: {
            const TernaryExpression& t = (const TernaryExpression&) e;
            return this->requirements(*t.fTest) | this->requirements(*t.fIfTrue) |
                   this->requirements(*t.fIfFalse);
        }
        case Expression::kVariableReference_Kind: {
            const VariableReference& v = (const VariableReference&) e;
            Requirements result = kNo_Requirements;
            if (v.fVariable.fModifiers.fLayout.fBuiltin == SK_FRAGCOORD_BUILTIN) {
                result = kGlobals_Requirement | kFragCoord_Requirement;
            } else if (Variable::kGlobal_Storage == v.fVariable.fStorage) {
                if (v.fVariable.fModifiers.fFlags & Modifiers::kIn_Flag) {
                    result = kInputs_Requirement;
                } else if (v.fVariable.fModifiers.fFlags & Modifiers::kOut_Flag) {
                    result = kOutputs_Requirement;
                } else if (v.fVariable.fModifiers.fFlags & Modifiers::kUniform_Flag &&
                           v.fVariable.fType.kind() != Type::kSampler_Kind) {
                    result = kUniforms_Requirement;
                } else {
                    result = kGlobals_Requirement;
                }
            }
            return result;
        }
        default:
            return kNo_Requirements;
    }
}

MetalCodeGenerator::Requirements MetalCodeGenerator::requirements(const Statement& s) {
    switch (s.fKind) {
        case Statement::kBlock_Kind: {
            Requirements result = kNo_Requirements;
            for (const auto& child : ((const Block&) s).fStatements) {
                result |= this->requirements(*child);
            }
            return result;
        }
        case Statement::kVarDeclaration_Kind: {
            Requirements result = kNo_Requirements;
            const VarDeclaration& var = (const VarDeclaration&) s;
            if (var.fValue) {
                result = this->requirements(*var.fValue);
            }
            return result;
        }
        case Statement::kVarDeclarations_Kind: {
            Requirements result = kNo_Requirements;
            const VarDeclarations& decls = *((const VarDeclarationsStatement&) s).fDeclaration;
            for (const auto& stmt : decls.fVars) {
                result |= this->requirements(*stmt);
            }
            return result;
        }
        case Statement::kExpression_Kind:
            return this->requirements(*((const ExpressionStatement&) s).fExpression);
        case Statement::kReturn_Kind: {
            const ReturnStatement& r = (const ReturnStatement&) s;
            if (r.fExpression) {
                return this->requirements(*r.fExpression);
            }
            return kNo_Requirements;
        }
        case Statement::kIf_Kind: {
            const IfStatement& i = (const IfStatement&) s;
            return this->requirements(*i.fTest) |
                   this->requirements(*i.fIfTrue) |
                   (i.fIfFalse ? this->requirements(*i.fIfFalse) : 0);
        }
        case Statement::kFor_Kind: {
            const ForStatement& f = (const ForStatement&) s;
            return this->requirements(*f.fInitializer) |
                   this->requirements(*f.fTest) |
                   this->requirements(*f.fNext) |
                   this->requirements(*f.fStatement);
        }
        case Statement::kWhile_Kind: {
            const WhileStatement& w = (const WhileStatement&) s;
            return this->requirements(*w.fTest) |
                   this->requirements(*w.fStatement);
        }
        case Statement::kDo_Kind: {
            const DoStatement& d = (const DoStatement&) s;
            return this->requirements(*d.fTest) |
                   this->requirements(*d.fStatement);
        }
        case Statement::kSwitch_Kind: {
            const SwitchStatement& sw = (const SwitchStatement&) s;
            Requirements result = this->requirements(*sw.fValue);
            for (const auto& c : sw.fCases) {
                for (const auto& st : c->fStatements) {
                    result |= this->requirements(*st);
                }
            }
            return result;
        }
        default:
            return kNo_Requirements;
    }
}

MetalCodeGenerator::Requirements MetalCodeGenerator::requirements(const FunctionDeclaration& f) {
    if (f.fBuiltin) {
        return kNo_Requirements;
    }
    auto found = fRequirements.find(&f);
    if (found == fRequirements.end()) {
        fRequirements[&f] = kNo_Requirements;
        for (const auto& e : fProgram) {
            if (ProgramElement::kFunction_Kind == e.fKind) {
                const FunctionDefinition& def = (const FunctionDefinition&) e;
                if (&def.fDeclaration == &f) {
                    Requirements reqs = this->requirements(*def.fBody);
                    fRequirements[&f] = reqs;
                    return reqs;
                }
            }
        }
    }
    return found->second;
}

bool MetalCodeGenerator::generateCode() {
    OutputStream* rawOut = fOut;
    fOut = &fHeader;
    fProgramKind = fProgram.fKind;
    this->writeHeader();
    this->writeUniformStruct();
    this->writeInputStruct();
    this->writeOutputStruct();
    this->writeInterfaceBlocks();
    this->writeGlobalStruct();
    StringStream body;
    fOut = &body;
    for (const auto& e : fProgram) {
        this->writeProgramElement(e);
    }
    fOut = rawOut;

    write_stringstream(fHeader, *rawOut);
    write_stringstream(fExtraFunctions, *rawOut);
    write_stringstream(body, *rawOut);
    return true;
}

}
