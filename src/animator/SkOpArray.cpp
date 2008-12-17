#include "SkOpArray.h"

SkOpArray::SkOpArray() : fType(SkOperand2::kNoType) {
}

SkOpArray::SkOpArray(SkOperand2::OpType type) : fType(type) {
}

bool SkOpArray::getIndex(int index, SkOperand2* operand) {
	if (index >= count()) {
		SkASSERT(0);
		return false;
	}
	*operand = begin()[index];
	return true;
}
