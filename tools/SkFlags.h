/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SK_FLAGS_H
#define SK_FLAGS_H

#include "SkString.h"
#include "SkTDArray.h"

/**
 *  Including this file (and compiling SkFlags.cpp) provides command line
 *  parsing. In order to use it, use the following macros in global
 *  namespace:
 *
 *  DEFINE_bool(name, defaultValue, helpString);
 *  DEFINE_string(name, defaultValue, helpString);
 *  DEFINE_int32(name, defaultValue, helpString);
 *  DEFINE_double(name, defaultValue, helpString);
 *
 *  Then, in main, call SkFlags::SetUsage() to describe usage and call
 *  SkFlags::ParseCommandLine() to parse the flags. Henceforth, each flag can
 *  be referenced using
 *
 *  FLAGS_name
 *
 *  For example, the line
 *
 *  DEFINE_bool(boolean, false, "The variable boolean does such and such");
 *
 *  will create the following variable:
 *
 *  bool FLAGS_boolean;
 *
 *  which will initially be set to false, and can be set to true by using the
 *  flag "--boolean" on the commandline. "--noboolean" will set FLAGS_boolean
 *  to false. (Single dashes are also permitted for this and other flags.) The
 *  helpString will be printed if the help flag (-h or -help) is used.
 *
 *  Similarly, the line
 *
 *  DEFINE_int32(integer, .., ..);
 *
 *  will create
 *
 *  int32_t FLAGS_integer;
 *
 *  and
 *
 *  DEFINE_double(real, .., ..);
 *
 *  will create
 *
 *  double FLAGS_real;
 *
 *  These flags can be set by specifying, for example, "--integer 7" and
 *  "--real 3.14" on the command line.
 *
 *  Unlike the others, the line
 *
 *  DEFINE_string(args, .., ..);
 *
 *  creates an array:
 *
 *  SkTDArray<const char*> FLAGS_args;
 *
 *  If the default value is the empty string, FLAGS_args will default to a size
 *  of zero. Otherwise it will default to a size of 1 with the default string
 *  as its value. All strings that follow the flag on the command line (until
 *  a string that begins with '-') will be entries in the array.
 *
 *  Any flag can be referenced from another file after using the following:
 *
 *  DECLARE_x(name);
 *
 *  (where 'x' is the type specified in the DEFINE).
 *
 *  Inspired by gflags (https://code.google.com/p/gflags/). Is not quite as
 *  robust as gflags, but suits our purposes. For example, allows creating
 *  a flag -h or -help which will never be used, since SkFlags handles it.
 *  SkFlags will also allow creating --flag and --noflag. Uses the same input
 *  format as gflags and creates similarly named variables (i.e. FLAGS_name).
 *  Strings are handled differently (resulting variable will be an array of
 *  strings) so that a flag can be followed by multiple parameters.
 */


class SkFlagInfo;

class SkFlags {

public:
    /**
     *  Call to set the help message to be displayed. Should be called before
     *  ParseCommandLine.
     */
    static void SetUsage(const char* usage);

    /**
     *  Call at the beginning of main to parse flags created by DEFINE_x, above.
     *  Must only be called once.
     */
    static void ParseCommandLine(int argc, char** argv);

private:
    static SkFlagInfo* gHead;
    static SkString    gUsage;

    // For access to gHead.
    friend class SkFlagInfo;
};

#define TO_STRING2(s) #s
#define TO_STRING(s) TO_STRING2(s)

#define DEFINE_bool(name, defaultValue, helpString)                         \
bool FLAGS_##name;                                                          \
static bool unused_##name = SkFlagInfo::CreateBoolFlag(TO_STRING(name),     \
                                                       &FLAGS_##name,       \
                                                       defaultValue,        \
                                                       helpString)

#define DECLARE_bool(name) extern bool FLAGS_##name;

#define DEFINE_string(name, defaultValue, helpString)                       \
SkTDArray<const char*> FLAGS_##name;                                        \
static bool unused_##name = SkFlagInfo::CreateStringFlag(TO_STRING(name),   \
                                                         &FLAGS_##name,     \
                                                         defaultValue,      \
                                                         helpString)

#define DECLARE_string(name) extern SkTDArray<const char*> FLAGS_##name;

#define DEFINE_int32(name, defaultValue, helpString)                        \
int32_t FLAGS_##name;                                                       \
static bool unused_##name = SkFlagInfo::CreateIntFlag(TO_STRING(name),      \
                                                      &FLAGS_##name,        \
                                                      defaultValue,         \
                                                      helpString)

#define DECLARE_int32(name) extern int32_t FLAGS_##name;

#define DEFINE_double(name, defaultValue, helpString)                       \
double FLAGS_##name;                                                        \
static bool unused_##name = SkFlagInfo::CreateDoubleFlag(TO_STRING(name),   \
                                                         &FLAGS_##name,     \
                                                         defaultValue,      \
                                                         helpString)

#define DECLARE_double(name) extern double FLAGS_##name;

class SkFlagInfo {

public:
    enum FlagTypes {
        kBool_FlagType,
        kString_FlagType,
        kInt_FlagType,
        kDouble_FlagType,
    };

    // Create flags of the desired type, and append to the list.
    static bool CreateBoolFlag(const char* name, bool* pBool,
                               bool defaultValue, const char* helpString) {
        SkFlagInfo* info = SkNEW_ARGS(SkFlagInfo, (name, kBool_FlagType, helpString));
        info->fBoolValue = pBool;
        *info->fBoolValue = info->fDefaultBool = defaultValue;
        return true;
    }

    static bool CreateStringFlag(const char* name, SkTDArray<const char*>* pStrings,
                                 const char* defaultValue, const char* helpString) {
        SkFlagInfo* info = SkNEW_ARGS(SkFlagInfo, (name, kString_FlagType, helpString));
        info->fDefaultString.set(defaultValue);

        info->fStrings = pStrings;
        info->fStrings->reset();
        // If default is "", leave the array empty.
        if (info->fDefaultString.size() > 0) {
            info->fStrings->append(1, &defaultValue);
        }
        return true;
    }

    static bool CreateIntFlag(const char* name, int32_t* pInt,
                              int32_t defaultValue, const char* helpString) {
        SkFlagInfo* info = SkNEW_ARGS(SkFlagInfo, (name, kInt_FlagType, helpString));
        info->fIntValue = pInt;
        *info->fIntValue = info->fDefaultInt = defaultValue;
        return true;
    }

    static bool CreateDoubleFlag(const char* name, double* pDouble,
                                 double defaultValue, const char* helpString) {
        SkFlagInfo* info = SkNEW_ARGS(SkFlagInfo, (name, kDouble_FlagType, helpString));
        info->fDoubleValue = pDouble;
        *info->fDoubleValue = info->fDefaultDouble = defaultValue;
        return true;
    }

    /**
     *  Returns true if the string matches this flag. For a bool, also sets the
     *  value, since a bool is specified as true or false by --name or --noname.
     */
    bool match(const char* string) {
        if (SkStrStartsWith(string, '-')) {
            string++;
            // Allow one or two dashes
            if (SkStrStartsWith(string, '-')) {
                string++;
            }
            if (kBool_FlagType == fFlagType) {
                // In this case, go ahead and set the value.
                if (fName.equals(string)) {
                    *fBoolValue = true;
                    return true;
                }
                SkString noname(fName);
                noname.prepend("no");
                if (noname.equals(string)) {
                    *fBoolValue = false;
                    return true;
                }
                return false;
            }
            return fName.equals(string);
        } else {
            // Has no dash
            return false;
        }
        return false;
    }

    FlagTypes getFlagType() const { return fFlagType; }

    void resetStrings() {
        if (kString_FlagType == fFlagType) {
            fStrings->reset();
        } else {
            SkASSERT(!"Can only call resetStrings on kString_FlagType");
        }
    }

    void append(const char* string) {
        if (kString_FlagType == fFlagType) {
            fStrings->append(1, &string);
        } else {
            SkASSERT(!"Can only append to kString_FlagType");
        }
    }

    void setInt(int32_t value) {
        if (kInt_FlagType == fFlagType) {
            *fIntValue = value;
        } else {
            SkASSERT(!"Can only call setInt on kInt_FlagType");
        }
    }

    void setDouble(double value) {
        if (kDouble_FlagType == fFlagType) {
            *fDoubleValue = value;
        } else {
            SkASSERT(!"Can only call setDouble on kDouble_FlagType");
        }
    }

    SkFlagInfo* next() { return fNext; }

    const SkString& name() const { return fName; }

    const SkString& help() const { return fHelpString; }

    SkString defaultValue() const {
        SkString result;
        switch (fFlagType) {
            case SkFlagInfo::kBool_FlagType:
                result.printf("%s", fDefaultBool ? "true" : "false");
                break;
            case SkFlagInfo::kString_FlagType:
                return fDefaultString;
            case SkFlagInfo::kInt_FlagType:
                result.printf("%i", fDefaultInt);
                break;
            case SkFlagInfo::kDouble_FlagType:
                result.printf("%2.2f", fDefaultDouble);
                break;
            default:
                SkASSERT(!"Invalid flag type");
        }
        return result;
    }

    SkString typeAsString() const {
        switch (fFlagType) {
            case SkFlagInfo::kBool_FlagType:
                return SkString("bool");
            case SkFlagInfo::kString_FlagType:
                return SkString("string");
            case SkFlagInfo::kInt_FlagType:
                return SkString("int");
            case SkFlagInfo::kDouble_FlagType:
                return SkString("double");
            default:
                SkASSERT(!"Invalid flag type");
                return SkString();
        }
    }

private:
    SkFlagInfo(const char* name, FlagTypes type, const char* helpString)
        : fName(name)
        , fFlagType(type)
        , fHelpString(helpString)
        , fBoolValue(NULL)
        , fDefaultBool(false)
        , fIntValue(NULL)
        , fDefaultInt(0)
        , fDoubleValue(NULL)
        , fDefaultDouble(0)
        , fStrings(NULL) {
        fNext = SkFlags::gHead;
        SkFlags::gHead = this;
    }
    // Name of the flag, without initial dashes
    SkString             fName;
    FlagTypes            fFlagType;
    SkString             fHelpString;
    bool*                fBoolValue;
    bool                 fDefaultBool;
    int32_t*             fIntValue;
    int32_t              fDefaultInt;
    double*              fDoubleValue;
    double               fDefaultDouble;
    SkTDArray<const char*>* fStrings;
    // Both for the help string and in case fStrings is empty.
    SkString             fDefaultString;

    // In order to keep a linked list.
    SkFlagInfo*          fNext;
};
#endif // SK_FLAGS_H
