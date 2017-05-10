/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_SETTINGVALUE
#define SKSL_SETTINGVALUE

namespace SkSL {

struct SettingValue {
    enum Type {
        kBool_Type,
        kInt_Type,
        kFloat_Type,
        kVec2_Type,
        kVec4_Type,
        kMat4_Type
    };

    SettingValue(bool value)
    : fType(kBool_Type) {
        fValue.fBool = value;
    }

    SettingValue(int value)
    : fType(kInt_Type) {
        fValue.fInt = value;
    }

    SettingValue(float value)
    : fType(kFloat_Type) {
        fValue.fFloat = value;
    }

    SettingValue(Type type, float values[])
    : fType(type) {
        switch (type) {
            case kVec2_Type:
                memcpy(fValue.fVec2, values, sizeof(fValue.fVec2));
                break;
            case kVec4_Type:
                memcpy(fValue.fVec4, values, sizeof(fValue.fVec4));
                break;
            case kMat4_Type:
                memcpy(fValue.fMat4, values, sizeof(fValue.fMat4));
                break;
            default:
                abort();
        }
    }

    Type fType;

    union {
        bool fBool;
        int64_t fInt;
        float fFloat;
        float fVec2[4];
        float fVec4[4];
        float fMat4[16];
    } fValue;
};

} // namespace

#endif
