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

    Type fType;

    union {
        bool fBool;
        int64_t fInt;
        float fFloat;
    } fValue;
};

} // namespace

#endif
