/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPathOpsDebug.h"
#include "SkPath.h"

#if defined SK_DEBUG || !FORCE_RELEASE

int SkPathOpsDebug::gMaxWindSum = SK_MaxS32;
int SkPathOpsDebug::gMaxWindValue = SK_MaxS32;

const char* SkPathOpsDebug::kLVerbStr[] = {"", "line", "quad", "cubic"};
int SkPathOpsDebug::gContourID;
int SkPathOpsDebug::gSegmentID;

#if DEBUG_SORT || DEBUG_SWAP_TOP
int SkPathOpsDebug::gSortCountDefault = SK_MaxS32;
int SkPathOpsDebug::gSortCount;
#endif

#if DEBUG_ACTIVE_OP
const char* SkPathOpsDebug::kPathOpStr[] = {"diff", "sect", "union", "xor"};
#endif

void SkPathOpsDebug::MathematicaIze(char* str, size_t bufferLen) {
    size_t len = strlen(str);
    bool num = false;
    for (size_t idx = 0; idx < len; ++idx) {
        if (num && str[idx] == 'e') {
            if (len + 2 >= bufferLen) {
                return;
            }
            memmove(&str[idx + 2], &str[idx + 1], len - idx);
            str[idx] = '*';
            str[idx + 1] = '^';
            ++len;
        }
        num = str[idx] >= '0' && str[idx] <= '9';
    }
}

bool SkPathOpsDebug::ValidWind(int wind) {
    return wind > SK_MinS32 + 0xFFFF && wind < SK_MaxS32 - 0xFFFF;
}

void SkPathOpsDebug::WindingPrintf(int wind) {
    if (wind == SK_MinS32) {
        SkDebugf("?");
    } else {
        SkDebugf("%d", wind);
    }
}

#if DEBUG_SHOW_TEST_NAME
void* SkPathOpsDebug::CreateNameStr() {
    return SkNEW_ARRAY(char, DEBUG_FILENAME_STRING_LENGTH);
}

void SkPathOpsDebug::DeleteNameStr(void* v) {
    SkDELETE_ARRAY(reinterpret_cast<char* >(v));
}

void SkPathOpsDebug::BumpTestName(char* test) {
    char* num = test + strlen(test);
    while (num[-1] >= '0' && num[-1] <= '9') {
        --num;
    }
    if (num[0] == '\0') {
        return;
    }
    int dec = atoi(num);
    if (dec == 0) {
        return;
    }
    ++dec;
    SK_SNPRINTF(num, DEBUG_FILENAME_STRING_LENGTH - (num - test), "%d", dec);
}
#endif

#include "SkOpSegment.h"

void SkPathOpsDebug::DumpAngles(const SkTArray<SkOpAngle, true>& angles) {
    int count = angles.count();
    for (int index = 0; index < count; ++index) {
        angles[index].dump();
    }
}

void SkPathOpsDebug::DumpAngles(const SkTArray<SkOpAngle* , true>& angles) {
    int count = angles.count();
    for (int index = 0; index < count; ++index) {
        angles[index]->dump();
    }
}
#endif  // SK_DEBUG || !FORCE_RELEASE

#ifdef SK_DEBUG
void SkOpSpan::dump() const {
    SkDebugf("t=");
    DebugDumpDouble(fT);
    SkDebugf(" pt=");
    SkDPoint::dump(fPt);
    SkDebugf(" other.fID=%d", fOther->debugID());
    SkDebugf(" [%d] otherT=", fOtherIndex);
    DebugDumpDouble(fOtherT);
    SkDebugf(" windSum=");
    SkPathOpsDebug::WindingPrintf(fWindSum);
    if (SkPathOpsDebug::ValidWind(fOppSum) || fOppValue != 0) {
        SkDebugf(" oppSum=");
        SkPathOpsDebug::WindingPrintf(fOppSum);
    }
    SkDebugf(" windValue=%d", fWindValue);
    if (SkPathOpsDebug::ValidWind(fOppSum) || fOppValue != 0) {
        SkDebugf(" oppValue=%d", fOppValue);
    }
    if (fDone) {
        SkDebugf(" done");
    }
    if (fUnsortableStart) {
        SkDebugf("  unsortable-start");
    }
    if (fUnsortableEnd) {
        SkDebugf(" unsortable-end");
    }
    if (fTiny) {
        SkDebugf(" tiny");
    } else if (fSmall) {
        SkDebugf(" small");
    }
    if (fLoop) {
        SkDebugf(" loop");
    }
    SkDebugf("\n");
}

void Dump(const SkTArray<class SkOpAngle, true>& angles) {
    SkPathOpsDebug::DumpAngles(angles);
}

void Dump(const SkTArray<class SkOpAngle* , true>& angles) {
    SkPathOpsDebug::DumpAngles(angles);
}

void Dump(const SkTArray<class SkOpAngle, true>* angles) {
    SkPathOpsDebug::DumpAngles(*angles);
}

void Dump(const SkTArray<class SkOpAngle* , true>* angles) {
    SkPathOpsDebug::DumpAngles(*angles);
}

#endif

#if !FORCE_RELEASE && 0  // enable when building without extended test
void SkPathOpsDebug::ShowPath(const SkPath& one, const SkPath& two, SkPathOp op, const char* name) {
}
#endif
