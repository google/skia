//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/EmulatePrecision.h"

namespace
{

static void writeVectorPrecisionEmulationHelpers(
    TInfoSinkBase& sink, ShShaderOutput outputLanguage, unsigned int size)
{
    std::stringstream vecTypeStrStr;
    if (outputLanguage == SH_ESSL_OUTPUT)
        vecTypeStrStr << "highp ";
    vecTypeStrStr << "vec" << size;
    std::string vecType = vecTypeStrStr.str();

    sink <<
    vecType << " angle_frm(in " << vecType << " v) {\n"
    "    v = clamp(v, -65504.0, 65504.0);\n"
    "    " << vecType << " exponent = floor(log2(abs(v) + 1e-30)) - 10.0;\n"
    "    bvec" << size << " isNonZero = greaterThanEqual(exponent, vec" << size << "(-25.0));\n"
    "    v = v * exp2(-exponent);\n"
    "    v = sign(v) * floor(abs(v));\n"
    "    return v * exp2(exponent) * vec" << size << "(isNonZero);\n"
    "}\n";

    sink <<
    vecType << " angle_frl(in " << vecType << " v) {\n"
    "    v = clamp(v, -2.0, 2.0);\n"
    "    v = v * 256.0;\n"
    "    v = sign(v) * floor(abs(v));\n"
    "    return v * 0.00390625;\n"
    "}\n";
}

static void writeMatrixPrecisionEmulationHelper(
    TInfoSinkBase& sink, ShShaderOutput outputLanguage, unsigned int size, const char *functionName)
{
    std::stringstream matTypeStrStr;
    if (outputLanguage == SH_ESSL_OUTPUT)
        matTypeStrStr << "highp ";
    matTypeStrStr << "mat" << size;
    std::string matType = matTypeStrStr.str();

    sink << matType << " " << functionName << "(in " << matType << " m) {\n"
            "    " << matType << " rounded;\n";

    for (unsigned int i = 0; i < size; ++i)
    {
        sink << "    rounded[" << i << "] = " << functionName << "(m[" << i << "]);\n";
    }

    sink << "    return rounded;\n"
            "}\n";
}

static void writeCommonPrecisionEmulationHelpers(TInfoSinkBase& sink, ShShaderOutput outputLanguage)
{
    // Write the angle_frm functions that round floating point numbers to
    // half precision, and angle_frl functions that round them to minimum lowp
    // precision.

    // Unoptimized version of angle_frm for single floats:
    //
    // int webgl_maxNormalExponent(in int exponentBits) {
    //     int possibleExponents = int(exp2(float(exponentBits)));
    //     int exponentBias = possibleExponents / 2 - 1;
    //     int allExponentBitsOne = possibleExponents - 1;
    //     return (allExponentBitsOne - 1) - exponentBias;
    // }
    //
    // float angle_frm(in float x) {
    //     int mantissaBits = 10;
    //     int exponentBits = 5;
    //     float possibleMantissas = exp2(float(mantissaBits));
    //     float mantissaMax = 2.0 - 1.0 / possibleMantissas;
    //     int maxNE = webgl_maxNormalExponent(exponentBits);
    //     float max = exp2(float(maxNE)) * mantissaMax;
    //     if (x > max) {
    //         return max;
    //     }
    //     if (x < -max) {
    //         return -max;
    //     }
    //     float exponent = floor(log2(abs(x)));
    //     if (abs(x) == 0.0 || exponent < -float(maxNE)) {
    //         return 0.0 * sign(x)
    //     }
    //     x = x * exp2(-(exponent - float(mantissaBits)));
    //     x = sign(x) * floor(abs(x));
    //     return x * exp2(exponent - float(mantissaBits));
    // }

    // All numbers with a magnitude less than 2^-15 are subnormal, and are
    // flushed to zero.

    // Note the constant numbers below:
    // a) 65504 is the maximum possible mantissa (1.1111111111 in binary) times
    //    2^15, the maximum normal exponent.
    // b) 10.0 is the number of mantissa bits.
    // c) -25.0 is the minimum normal half-float exponent -15.0 minus the number
    //    of mantissa bits.
    // d) + 1e-30 is to make sure the argument of log2() won't be zero. It can
    //    only affect the result of log2 on x where abs(x) < 1e-22. Since these
    //    numbers will be flushed to zero either way (2^-15 is the smallest
    //    normal positive number), this does not introduce any error.

    std::string floatType = "float";
    if (outputLanguage == SH_ESSL_OUTPUT)
        floatType = "highp float";

    sink <<
    floatType << " angle_frm(in " << floatType << " x) {\n"
    "    x = clamp(x, -65504.0, 65504.0);\n"
    "    " << floatType << " exponent = floor(log2(abs(x) + 1e-30)) - 10.0;\n"
    "    bool isNonZero = (exponent >= -25.0);\n"
    "    x = x * exp2(-exponent);\n"
    "    x = sign(x) * floor(abs(x));\n"
    "    return x * exp2(exponent) * float(isNonZero);\n"
    "}\n";

    sink <<
    floatType << " angle_frl(in " << floatType << " x) {\n"
    "    x = clamp(x, -2.0, 2.0);\n"
    "    x = x * 256.0;\n"
    "    x = sign(x) * floor(abs(x));\n"
    "    return x * 0.00390625;\n"
    "}\n";

    writeVectorPrecisionEmulationHelpers(sink, outputLanguage, 2);
    writeVectorPrecisionEmulationHelpers(sink, outputLanguage, 3);
    writeVectorPrecisionEmulationHelpers(sink, outputLanguage, 4);
    for (unsigned int size = 2; size <= 4; ++size)
    {
        writeMatrixPrecisionEmulationHelper(sink, outputLanguage, size, "angle_frm");
        writeMatrixPrecisionEmulationHelper(sink, outputLanguage, size, "angle_frl");
    }
}

static void writeCompoundAssignmentPrecisionEmulation(
    TInfoSinkBase& sink, ShShaderOutput outputLanguage,
    const char *lType, const char *rType, const char *opStr, const char *opNameStr)
{
    std::string lTypeStr = lType;
    std::string rTypeStr = rType;
    if (outputLanguage == SH_ESSL_OUTPUT)
    {
        std::stringstream lTypeStrStr;
        lTypeStrStr << "highp " << lType;
        lTypeStr = lTypeStrStr.str();
        std::stringstream rTypeStrStr;
        rTypeStrStr << "highp " << rType;
        rTypeStr = rTypeStrStr.str();
    }

    // Note that y should be passed through angle_frm at the function call site,
    // but x can't be passed through angle_frm there since it is an inout parameter.
    // So only pass x and the result through angle_frm here.
    sink <<
    lTypeStr << " angle_compound_" << opNameStr << "_frm(inout " << lTypeStr << " x, in " << rTypeStr << " y) {\n"
    "    x = angle_frm(angle_frm(x) " << opStr << " y);\n"
    "    return x;\n"
    "}\n";
    sink <<
    lTypeStr << " angle_compound_" << opNameStr << "_frl(inout " << lTypeStr << " x, in " << rTypeStr << " y) {\n"
    "    x = angle_frl(angle_frm(x) " << opStr << " y);\n"
    "    return x;\n"
    "}\n";
}

const char *getFloatTypeStr(const TType& type)
{
    switch (type.getNominalSize())
    {
      case 1:
        return "float";
      case 2:
        switch(type.getSecondarySize())
        {
          case 1:
            return "vec2";
          case 2:
            return "mat2";
          case 3:
            return "mat2x3";
          case 4:
            return "mat2x4";
          default:
            UNREACHABLE();
            return NULL;
        }
      case 3:
        switch(type.getSecondarySize())
        {
          case 1:
            return "vec3";
          case 2:
            return "mat3x2";
          case 3:
            return "mat3";
          case 4:
            return "mat3x4";
          default:
            UNREACHABLE();
            return NULL;
        }
      case 4:
        switch(type.getSecondarySize())
        {
          case 1:
            return "vec4";
          case 2:
            return "mat4x2";
          case 3:
            return "mat4x3";
          case 4:
            return "mat4";
          default:
            UNREACHABLE();
            return NULL;
        }
      default:
        UNREACHABLE();
        return NULL;
    }
}

bool canRoundFloat(const TType &type)
{
    return type.getBasicType() == EbtFloat && !type.isNonSquareMatrix() && !type.isArray() &&
        (type.getPrecision() == EbpLow || type.getPrecision() == EbpMedium);
}

TIntermAggregate *createInternalFunctionCallNode(TString name, TIntermNode *child)
{
    TIntermAggregate *callNode = new TIntermAggregate();
    callNode->setOp(EOpFunctionCall);
    TName nameObj(TFunction::mangleName(name));
    nameObj.setInternal(true);
    callNode->setNameObj(nameObj);
    callNode->getSequence()->push_back(child);
    return callNode;
}

TIntermAggregate *createRoundingFunctionCallNode(TIntermTyped *roundedChild)
{
    TString roundFunctionName;
    if (roundedChild->getPrecision() == EbpMedium)
        roundFunctionName = "angle_frm";
    else
        roundFunctionName = "angle_frl";
    return createInternalFunctionCallNode(roundFunctionName, roundedChild);
}

TIntermAggregate *createCompoundAssignmentFunctionCallNode(TIntermTyped *left, TIntermTyped *right, const char *opNameStr)
{
    std::stringstream strstr;
    if (left->getPrecision() == EbpMedium)
        strstr << "angle_compound_" << opNameStr << "_frm";
    else
        strstr << "angle_compound_" << opNameStr << "_frl";
    TString functionName = strstr.str().c_str();
    TIntermAggregate *callNode = createInternalFunctionCallNode(functionName, left);
    callNode->getSequence()->push_back(right);
    return callNode;
}

bool parentUsesResult(TIntermNode* parent, TIntermNode* node)
{
    if (!parent)
    {
        return false;
    }

    TIntermAggregate *aggParent = parent->getAsAggregate();
    // If the parent's op is EOpSequence, the result is not assigned anywhere,
    // so rounding it is not needed. In particular, this can avoid a lot of
    // unnecessary rounding of unused return values of assignment.
    if (aggParent && aggParent->getOp() == EOpSequence)
    {
        return false;
    }
    if (aggParent && aggParent->getOp() == EOpComma && (aggParent->getSequence()->back() != node))
    {
        return false;
    }
    return true;
}

}  // namespace anonymous

EmulatePrecision::EmulatePrecision(const TSymbolTable &symbolTable, int shaderVersion)
    : TLValueTrackingTraverser(true, true, true, symbolTable, shaderVersion),
      mDeclaringVariables(false)
{}

void EmulatePrecision::visitSymbol(TIntermSymbol *node)
{
    if (canRoundFloat(node->getType()) && !mDeclaringVariables && !isLValueRequiredHere())
    {
        TIntermNode *parent = getParentNode();
        TIntermNode *replacement = createRoundingFunctionCallNode(node);
        mReplacements.push_back(NodeUpdateEntry(parent, node, replacement, true));
    }
}


bool EmulatePrecision::visitBinary(Visit visit, TIntermBinary *node)
{
    bool visitChildren = true;

    TOperator op = node->getOp();

    // RHS of initialize is not being declared.
    if (op == EOpInitialize && visit == InVisit)
        mDeclaringVariables = false;

    if ((op == EOpIndexDirectStruct || op == EOpVectorSwizzle) && visit == InVisit)
        visitChildren = false;

    if (visit != PreVisit)
        return visitChildren;

    const TType& type = node->getType();
    bool roundFloat = canRoundFloat(type);

    if (roundFloat) {
        switch (op) {
          // Math operators that can result in a float may need to apply rounding to the return
          // value. Note that in the case of assignment, the rounding is applied to its return
          // value here, not the value being assigned.
          case EOpAssign:
          case EOpAdd:
          case EOpSub:
          case EOpMul:
          case EOpDiv:
          case EOpVectorTimesScalar:
          case EOpVectorTimesMatrix:
          case EOpMatrixTimesVector:
          case EOpMatrixTimesScalar:
          case EOpMatrixTimesMatrix:
          {
            TIntermNode *parent = getParentNode();
            if (!parentUsesResult(parent, node))
            {
                break;
            }
            TIntermNode *replacement = createRoundingFunctionCallNode(node);
            mReplacements.push_back(NodeUpdateEntry(parent, node, replacement, true));
            break;
          }

          // Compound assignment cases need to replace the operator with a function call.
          case EOpAddAssign:
          {
            mEmulateCompoundAdd.insert(TypePair(getFloatTypeStr(type), getFloatTypeStr(node->getRight()->getType())));
            TIntermNode *parent = getParentNode();
            TIntermNode *replacement = createCompoundAssignmentFunctionCallNode(node->getLeft(), node->getRight(), "add");
            mReplacements.push_back(NodeUpdateEntry(parent, node, replacement, false));
            break;
          }
          case EOpSubAssign:
          {
            mEmulateCompoundSub.insert(TypePair(getFloatTypeStr(type), getFloatTypeStr(node->getRight()->getType())));
            TIntermNode *parent = getParentNode();
            TIntermNode *replacement = createCompoundAssignmentFunctionCallNode(node->getLeft(), node->getRight(), "sub");
            mReplacements.push_back(NodeUpdateEntry(parent, node, replacement, false));
            break;
          }
          case EOpMulAssign:
          case EOpVectorTimesMatrixAssign:
          case EOpVectorTimesScalarAssign:
          case EOpMatrixTimesScalarAssign:
          case EOpMatrixTimesMatrixAssign:
          {
            mEmulateCompoundMul.insert(TypePair(getFloatTypeStr(type), getFloatTypeStr(node->getRight()->getType())));
            TIntermNode *parent = getParentNode();
            TIntermNode *replacement = createCompoundAssignmentFunctionCallNode(node->getLeft(), node->getRight(), "mul");
            mReplacements.push_back(NodeUpdateEntry(parent, node, replacement, false));
            break;
          }
          case EOpDivAssign:
          {
            mEmulateCompoundDiv.insert(TypePair(getFloatTypeStr(type), getFloatTypeStr(node->getRight()->getType())));
            TIntermNode *parent = getParentNode();
            TIntermNode *replacement = createCompoundAssignmentFunctionCallNode(node->getLeft(), node->getRight(), "div");
            mReplacements.push_back(NodeUpdateEntry(parent, node, replacement, false));
            break;
          }
          default:
            // The rest of the binary operations should not need precision emulation.
            break;
        }
    }
    return visitChildren;
}

bool EmulatePrecision::visitAggregate(Visit visit, TIntermAggregate *node)
{
    bool visitChildren = true;
    switch (node->getOp())
    {
      case EOpSequence:
      case EOpConstructStruct:
      case EOpFunction:
        break;
      case EOpPrototype:
        visitChildren = false;
        break;
      case EOpParameters:
        visitChildren = false;
        break;
      case EOpInvariantDeclaration:
        visitChildren = false;
        break;
      case EOpDeclaration:
        // Variable declaration.
        if (visit == PreVisit)
        {
            mDeclaringVariables = true;
        }
        else if (visit == InVisit)
        {
            mDeclaringVariables = true;
        }
        else
        {
            mDeclaringVariables = false;
        }
        break;
      case EOpFunctionCall:
      {
        // Function call.
        if (visit == PreVisit)
        {
            // User-defined function return values are not rounded, this relies on that
            // calculations producing the value were rounded.
            TIntermNode *parent = getParentNode();
            if (canRoundFloat(node->getType()) && !isInFunctionMap(node) &&
                parentUsesResult(parent, node))
            {
                TIntermNode *replacement = createRoundingFunctionCallNode(node);
                mReplacements.push_back(NodeUpdateEntry(parent, node, replacement, true));
            }
        }
        break;
      }
      default:
        TIntermNode *parent = getParentNode();
        if (canRoundFloat(node->getType()) && visit == PreVisit && parentUsesResult(parent, node))
        {
            TIntermNode *replacement = createRoundingFunctionCallNode(node);
            mReplacements.push_back(NodeUpdateEntry(parent, node, replacement, true));
        }
        break;
    }
    return visitChildren;
}

bool EmulatePrecision::visitUnary(Visit visit, TIntermUnary *node)
{
    switch (node->getOp())
    {
      case EOpNegative:
      case EOpVectorLogicalNot:
      case EOpLogicalNot:
      case EOpPostIncrement:
      case EOpPostDecrement:
      case EOpPreIncrement:
      case EOpPreDecrement:
        break;
      default:
        if (canRoundFloat(node->getType()) && visit == PreVisit)
        {
            TIntermNode *parent = getParentNode();
            TIntermNode *replacement = createRoundingFunctionCallNode(node);
            mReplacements.push_back(NodeUpdateEntry(parent, node, replacement, true));
        }
        break;
    }

    return true;
}

void EmulatePrecision::writeEmulationHelpers(TInfoSinkBase& sink, ShShaderOutput outputLanguage)
{
    // Other languages not yet supported
    ASSERT(outputLanguage == SH_GLSL_COMPATIBILITY_OUTPUT ||
           IsGLSL130OrNewer(outputLanguage) ||
           outputLanguage == SH_ESSL_OUTPUT);
    writeCommonPrecisionEmulationHelpers(sink, outputLanguage);

    EmulationSet::const_iterator it;
    for (it = mEmulateCompoundAdd.begin(); it != mEmulateCompoundAdd.end(); it++)
        writeCompoundAssignmentPrecisionEmulation(sink, outputLanguage, it->lType, it->rType, "+", "add");
    for (it = mEmulateCompoundSub.begin(); it != mEmulateCompoundSub.end(); it++)
        writeCompoundAssignmentPrecisionEmulation(sink, outputLanguage, it->lType, it->rType, "-", "sub");
    for (it = mEmulateCompoundDiv.begin(); it != mEmulateCompoundDiv.end(); it++)
        writeCompoundAssignmentPrecisionEmulation(sink, outputLanguage, it->lType, it->rType, "/", "div");
    for (it = mEmulateCompoundMul.begin(); it != mEmulateCompoundMul.end(); it++)
        writeCompoundAssignmentPrecisionEmulation(sink, outputLanguage, it->lType, it->rType, "*", "mul");
}

