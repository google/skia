/* libs/graphics/animator/SkScriptDecompile.cpp
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#include "SkScript2.h"

#ifdef SK_DEBUG

#define TypeOpName(op) {SkScriptEngine2::op, #op }

static const struct OpName {
    SkScriptEngine2::TypeOp fOp;
    const char* fName;
} gOpNames[] = {
    TypeOpName(kNop), // should never get generated
    TypeOpName(kAccumulatorPop),
    TypeOpName(kAccumulatorPush),
    TypeOpName(kAddInt),
    TypeOpName(kAddScalar),
    TypeOpName(kAddString), // string concat
    TypeOpName(kArrayIndex),
    TypeOpName(kArrayParam),
    TypeOpName(kArrayToken),
    TypeOpName(kBitAndInt),
    TypeOpName(kBitNotInt),
    TypeOpName(kBitOrInt),
    TypeOpName(kBoxToken),
    TypeOpName(kCallback),
    TypeOpName(kDivideInt),
    TypeOpName(kDivideScalar),
    TypeOpName(kDotOperator),
    TypeOpName(kElseOp),
    TypeOpName(kEnd),
    TypeOpName(kEqualInt),
    TypeOpName(kEqualScalar),
    TypeOpName(kEqualString),
    TypeOpName(kFunctionCall),
    TypeOpName(kFlipOpsOp),
    TypeOpName(kFunctionToken),
    TypeOpName(kGreaterEqualInt),
    TypeOpName(kGreaterEqualScalar),
    TypeOpName(kGreaterEqualString),
    TypeOpName(kIfOp),
    TypeOpName(kIntToScalar),
    TypeOpName(kIntToScalar2),
    TypeOpName(kIntToString),
    TypeOpName(kIntToString2),
    TypeOpName(kIntegerAccumulator),
    TypeOpName(kIntegerOperand),
    TypeOpName(kLogicalAndInt),
    TypeOpName(kLogicalNotInt),
    TypeOpName(kLogicalOrInt),
    TypeOpName(kMemberOp),
    TypeOpName(kMinusInt),
    TypeOpName(kMinusScalar),
    TypeOpName(kModuloInt),
    TypeOpName(kModuloScalar),
    TypeOpName(kMultiplyInt),
    TypeOpName(kMultiplyScalar),
    TypeOpName(kPropertyOp),
    TypeOpName(kScalarAccumulator),
    TypeOpName(kScalarOperand),
    TypeOpName(kScalarToInt),
    TypeOpName(kScalarToInt2),
    TypeOpName(kScalarToString),
    TypeOpName(kScalarToString2),
    TypeOpName(kShiftLeftInt),
    TypeOpName(kShiftRightInt), // signed
    TypeOpName(kStringAccumulator),
    TypeOpName(kStringOperand),
    TypeOpName(kStringToInt),
    TypeOpName(kStringToScalar),
    TypeOpName(kStringToScalar2),
    TypeOpName(kStringTrack),
    TypeOpName(kSubtractInt),
    TypeOpName(kSubtractScalar),
    TypeOpName(kToBool),
    TypeOpName(kUnboxToken),
    TypeOpName(kUnboxToken2),
    TypeOpName(kXorInt)
};

static size_t gOpNamesSize = sizeof(gOpNames) / sizeof(gOpNames[0]);

#define OperandName(op) {SkOperand2::op, #op }

static const struct OperName {
    SkOperand2::OpType fType;
    const char* fName;
} gOperandNames[] = {
    OperandName(kNoType),
    OperandName(kS32),
    OperandName(kScalar),
    OperandName(kString),
    OperandName(kArray),
    OperandName(kObject)
};  

static size_t gOperandNamesSize = sizeof(gOperandNames) / sizeof(gOperandNames[0]);

// check to see that there are no missing or duplicate entries
void SkScriptEngine2::ValidateDecompileTable() {
    SkScriptEngine2::TypeOp op = SkScriptEngine2::kNop;
    size_t index;
    for (index = 0; index < gOpNamesSize; index++) {
        SkASSERT(gOpNames[index].fOp == op);
        op = (SkScriptEngine2::TypeOp) (op + 1);
    }
    index = 0;
    SkOperand2::OpType type = SkOperand2::kNoType;
    SkASSERT(gOperandNames[index].fType == type);
    for (; index < gOperandNamesSize - 1; ) {
        type = (SkOperand2::OpType) (1 << index);
        SkASSERT(gOperandNames[++index].fType == type);
    }
}

void SkScriptEngine2::decompile(const unsigned char* start, size_t length) {
    SkASSERT(length > 0);
    const unsigned char* opCode = start;
    do {
        SkASSERT((size_t)(opCode - start) < length);
        SkScriptEngine2::TypeOp op = (SkScriptEngine2::TypeOp) *opCode++;
        SkASSERT((size_t)op < gOpNamesSize);
        SkDebugf("%d: %s", opCode - start - 1, gOpNames[op].fName);
        switch (op) {
        case SkScriptEngine2::kCallback: {
            int index;
            memcpy(&index, opCode, sizeof(index));
            opCode += sizeof(index);
            SkDebugf(" index: %d", index);
            } break;
        case SkScriptEngine2::kFunctionCall: 
        case SkScriptEngine2::kMemberOp:
        case SkScriptEngine2::kPropertyOp: {
            size_t ref;
            memcpy(&ref, opCode, sizeof(ref));
            opCode += sizeof(ref);
            SkDebugf(" ref: %d", ref);
            } break;
        case SkScriptEngine2::kIntegerAccumulator:
        case SkScriptEngine2::kIntegerOperand: {
            int32_t integer;
            memcpy(&integer, opCode, sizeof(integer));
            opCode += sizeof(int32_t);
            SkDebugf(" integer: %d", integer);
            } break;
        case SkScriptEngine2::kScalarAccumulator:
        case SkScriptEngine2::kScalarOperand: {
            SkScalar scalar;
            memcpy(&scalar, opCode, sizeof(scalar));
            opCode += sizeof(SkScalar);
#ifdef SK_CAN_USE_FLOAT
            SkDebugf(" scalar: %g", SkScalarToFloat(scalar));
#else
            SkDebugf(" scalar: %x", scalar);
#endif
            } break;
        case SkScriptEngine2::kStringAccumulator:
        case SkScriptEngine2::kStringOperand: {
            int size;
            SkString* strPtr = new SkString();
            memcpy(&size, opCode, sizeof(size));
            opCode += sizeof(size);
            strPtr->set((char*) opCode, size);
            opCode += size;
            SkDebugf(" string: %s", strPtr->c_str());
            delete strPtr;
            } break;
        case SkScriptEngine2::kBoxToken: {
            SkOperand2::OpType type;
            memcpy(&type, opCode, sizeof(type));
            opCode += sizeof(type);
            size_t index = 0;
            if (type == 0)
                SkDebugf(" type: %s", gOperandNames[index].fName);
            else {
                while (type != 0) {
                    SkASSERT(index + 1 < gOperandNamesSize);
                    if (type & (1 << index)) {
                        type = (SkOperand2::OpType) (type & ~(1 << index));
                        SkDebugf(" type: %s", gOperandNames[index + 1].fName);
                    }
                    index++;
                }
            }
            } break;
        case SkScriptEngine2::kIfOp:
        case SkScriptEngine2::kLogicalAndInt:
        case SkScriptEngine2::kElseOp:
        case SkScriptEngine2::kLogicalOrInt: {
            int size;
            memcpy(&size, opCode, sizeof(size));
            opCode += sizeof(size);
            SkDebugf(" offset (address): %d (%d)", size, opCode - start + size);
            } break;
        case SkScriptEngine2::kEnd:
            goto done;
        case SkScriptEngine2::kNop:
                SkASSERT(0);
        default:
            break;
    }
    SkDebugf("\n");
    } while (true);
done:
    SkDebugf("\n");
}

#endif
