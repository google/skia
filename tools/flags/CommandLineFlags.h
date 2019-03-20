/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SK_COMMAND_LINE_FLAGS_H
#define SK_COMMAND_LINE_FLAGS_H

#include "../private/SkTArray.h"
#include "../private/SkTDArray.h"
#include "SkString.h"

/**
 *  Including this file (and compiling CommandLineFlags.cpp) provides command line
 *  parsing. In order to use it, use the following macros in global
 *  namespace:
 *
 *  DEFINE_bool(name, defaultValue, helpString);
 *  DEFINE_string(name, defaultValue, helpString);
 *  DEFINE_int32(name, defaultValue, helpString);
 *  DEFINE_double(name, defaultValue, helpString);
 *
 *  Then, in main, call CommandLineFlags::SetUsage() to describe usage and call
 *  CommandLineFlags::Parse() to parse the flags. Henceforth, each flag can
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
 *  to false. FLAGS_boolean can also be set using "--boolean=true" or
 *  "--boolean true" (where "true" can be replaced by "false", "TRUE", "FALSE",
 *  "1" or "0").
 *
 *  The helpString will be printed if the help flag (-h or -help) is used.
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
 *  and
 *
 *  DEFINE_uint32(unsigned, ...);
 *
 *  will create
 *
 *  uint32_t FLAGS_unsigned;
 *
 *  These flags can be set by specifying, for example, "--integer 7" and
 *  "--real 3.14" on the command line. Unsigned integers are parsed from the
 *  command line using strtoul() so will detect the base (0 for octal, and
 *  0x or 0X for hex, otherwise assumes decimal).
 *
 *  Unlike the others, the line
 *
 *  DEFINE_string(args, .., ..);
 *
 *  creates an array:
 *
 *  CommandLineFlags::StringArray FLAGS_args;
 *
 *  If the default value is the empty string, FLAGS_args will default to a size
 *  of zero. Otherwise it will default to a size of 1 with the default string
 *  as its value. All strings that follow the flag on the command line (until
 *  a string that begins with '-') will be entries in the array.
 *
 *  DEFINE_extended_string(args, .., .., extendedHelpString);
 *
 *  creates a similar string array flag as DEFINE_string. The flag will have extended help text
 *  (extendedHelpString) that can the user can see with '--help <args>' flag.
 *
 *  Any flag can be referenced from another file after using the following:
 *
 *  DECLARE_x(name);
 *
 *  (where 'x' is the type specified in the DEFINE).
 *
 *  Inspired by gflags (https://code.google.com/p/gflags/). Is not quite as
 *  robust as gflags, but suits our purposes. For example, allows creating
 *  a flag -h or -help which will never be used, since CommandLineFlags handles it.
 *  CommandLineFlags will also allow creating --flag and --noflag. Uses the same input
 *  format as gflags and creates similarly named variables (i.e. FLAGS_name).
 *  Strings are handled differently (resulting variable will be an array of
 *  strings) so that a flag can be followed by multiple parameters.
 */

class SkFlagInfo;

class CommandLineFlags {
public:
    /**
     *  Call to set the help message to be displayed. Should be called before
     *  Parse.
     */
    static void SetUsage(const char* usage);

    /**
     *  Call this to display the help message. Should be called after SetUsage.
     */
    static void PrintUsage();

    /**
     *  Call at the beginning of main to parse flags created by DEFINE_x, above.
     *  Must only be called once.
     */
    static void Parse(int argc, const char* const* argv);

    /**
     *  Custom class for holding the arguments for a string flag.
     *  Publicly only has accessors so the strings cannot be modified.
     */
    class StringArray {
    public:
        StringArray() {}
        explicit StringArray(const SkTArray<SkString>& strings) : fStrings(strings) {}
        const char* operator[](int i) const {
            SkASSERT(i >= 0 && i < fStrings.count());
            return fStrings[i].c_str();
        }

        int count() const { return fStrings.count(); }

        bool isEmpty() const { return this->count() == 0; }

        /**
         * Returns true iff string is equal to one of the strings in this array.
         */
        bool contains(const char* string) const {
            for (int i = 0; i < fStrings.count(); i++) {
                if (fStrings[i].equals(string)) {
                    return true;
                }
            }
            return false;
        }

        void set(int i, const char* str) {
            if (i >= fStrings.count()) {
                this->append(str);
                return;
            }
            fStrings[i].set(str);
        }

        const SkString* begin() const { return fStrings.begin(); }
        const SkString* end() const { return fStrings.end(); }

    private:
        void reset() { fStrings.reset(); }

        void append(const char* string) { fStrings.push_back().set(string); }

        void append(const char* string, size_t length) { fStrings.push_back().set(string, length); }

        SkTArray<SkString> fStrings;

        friend class SkFlagInfo;
    };

    /* Takes a list of the form [~][^]match[$]
     ~ causes a matching test to always be skipped
     ^ requires the start of the test to match
     $ requires the end of the test to match
     ^ and $ requires an exact match
     If a test does not match any list entry, it is skipped unless some list entry starts with ~
    */
    static bool ShouldSkip(const SkTDArray<const char*>& strings, const char* name);
    static bool ShouldSkip(const StringArray& strings, const char* name);

private:
    static SkFlagInfo* gHead;
    static SkString    gUsage;

    // For access to gHead.
    friend class SkFlagInfo;
};

#define TO_STRING2(s) #s
#define TO_STRING(s) TO_STRING2(s)

#define DEFINE_bool(name, defaultValue, helpString)                   \
    bool                  FLAGS_##name;                               \
    SK_UNUSED static bool unused_##name = SkFlagInfo::CreateBoolFlag( \
            TO_STRING(name), nullptr, &FLAGS_##name, defaultValue, helpString)

// bool 2 allows specifying a short name. No check is done to ensure that shortName
// is actually shorter than name.
#define DEFINE_bool2(name, shortName, defaultValue, helpString)       \
    bool                  FLAGS_##name;                               \
    SK_UNUSED static bool unused_##name = SkFlagInfo::CreateBoolFlag( \
            TO_STRING(name), TO_STRING(shortName), &FLAGS_##name, defaultValue, helpString)

#define DECLARE_bool(name) extern bool FLAGS_##name;

#define DEFINE_string(name, defaultValue, helpString)                           \
    CommandLineFlags::StringArray FLAGS_##name;                                 \
    SK_UNUSED static bool         unused_##name = SkFlagInfo::CreateStringFlag( \
            TO_STRING(name), nullptr, &FLAGS_##name, defaultValue, helpString, nullptr)
#define DEFINE_extended_string(name, defaultValue, helpString, extendedHelpString) \
    CommandLineFlags::StringArray FLAGS_##name;                                    \
    SK_UNUSED static bool         unused_##name = SkFlagInfo::CreateStringFlag(    \
            TO_STRING(name), nullptr, &FLAGS_##name, defaultValue, helpString, extendedHelpString)

// string2 allows specifying a short name. There is an assert that shortName
// is only 1 character.
#define DEFINE_string2(name, shortName, defaultValue, helpString)                                    \
    CommandLineFlags::StringArray FLAGS_##name;                                                      \
    SK_UNUSED static bool         unused_##name = SkFlagInfo::CreateStringFlag(TO_STRING(name),      \
                                                                       TO_STRING(shortName), \
                                                                       &FLAGS_##name,        \
                                                                       defaultValue,         \
                                                                       helpString,           \
                                                                       nullptr)

#define DECLARE_string(name) extern CommandLineFlags::StringArray FLAGS_##name;

#define DEFINE_int32(name, defaultValue, helpString) \
    int32_t               FLAGS_##name;              \
    SK_UNUSED static bool unused_##name =            \
            SkFlagInfo::CreateIntFlag(TO_STRING(name), &FLAGS_##name, defaultValue, helpString)

#define DEFINE_int32_2(name, shortName, defaultValue, helpString)    \
    int32_t               FLAGS_##name;                              \
    SK_UNUSED static bool unused_##name = SkFlagInfo::CreateIntFlag( \
            TO_STRING(name), TO_STRING(shortName), &FLAGS_##name, defaultValue, helpString)

#define DECLARE_int32(name) extern int32_t FLAGS_##name;

#define DEFINE_uint32(name, defaultValue, helpString) \
    uint32_t              FLAGS_##name;               \
    SK_UNUSED static bool unused_##name =             \
            SkFlagInfo::CreateUintFlag(TO_STRING(name), &FLAGS_##name, defaultValue, helpString)

#define DEFINE_uint32_2(name, shortName, defaultValue, helpString)    \
    uint32_t              FLAGS_##name;                               \
    SK_UNUSED static bool unused_##name = SkFlagInfo::CreateUintFlag( \
            TO_STRING(name), TO_STRING(shortName), &FLAGS_##name, defaultValue, helpString)

#define DECLARE_uint32(name) extern uint32_t FLAGS_##name;

#define DEFINE_double(name, defaultValue, helpString) \
    double                FLAGS_##name;               \
    SK_UNUSED static bool unused_##name =             \
            SkFlagInfo::CreateDoubleFlag(TO_STRING(name), &FLAGS_##name, defaultValue, helpString)

#define DECLARE_double(name) extern double FLAGS_##name;

class SkFlagInfo {
public:
    enum FlagTypes {
        kBool_FlagType,
        kString_FlagType,
        kInt_FlagType,
        kUint_FlagType,
        kDouble_FlagType,
    };

    /**
     *  Each Create<Type>Flag function creates an SkFlagInfo of the specified type. The SkFlagInfo
     *  object is appended to a list, which is deleted when CommandLineFlags::Parse is called.
     *  Therefore, each call should be made before the call to ::Parse. They are not intended
     *  to be called directly. Instead, use the macros described above.
     *  @param name Long version (at least 2 characters) of the name of the flag. This name can
     *      be referenced on the command line as "--name" to set the value of this flag.
     *  @param shortName Short version (one character) of the name of the flag. This name can
     *      be referenced on the command line as "-shortName" to set the value of this flag.
     *  @param p<Type> Pointer to a global variable which holds the value set by CommandLineFlags.
     *  @param defaultValue The default value of this flag. The variable pointed to by p<Type> will
     *      be set to this value initially. This is also displayed as part of the help output.
     *  @param helpString Explanation of what this flag changes in the program.
     */
    static bool CreateBoolFlag(const char* name,
                               const char* shortName,
                               bool*       pBool,
                               bool        defaultValue,
                               const char* helpString) {
        SkFlagInfo* info  = new SkFlagInfo(name, shortName, kBool_FlagType, helpString, nullptr);
        info->fBoolValue  = pBool;
        *info->fBoolValue = info->fDefaultBool = defaultValue;
        return true;
    }

    /**
     *  See comments for CreateBoolFlag.
     *  @param pStrings Unlike the others, this is a pointer to an array of values.
     *  @param defaultValue Thise default will be parsed so that strings separated by spaces
     *      will be added to pStrings.
     */
    static bool CreateStringFlag(const char*                    name,
                                 const char*                    shortName,
                                 CommandLineFlags::StringArray* pStrings,
                                 const char*                    defaultValue,
                                 const char*                    helpString,
                                 const char*                    extendedHelpString);

    /**
     *  See comments for CreateBoolFlag.
     */
    static bool CreateIntFlag(const char* name,
                              int32_t*    pInt,
                              int32_t     defaultValue,
                              const char* helpString) {
        SkFlagInfo* info = new SkFlagInfo(name, nullptr, kInt_FlagType, helpString, nullptr);
        info->fIntValue  = pInt;
        *info->fIntValue = info->fDefaultInt = defaultValue;
        return true;
    }

    static bool CreateIntFlag(const char* name,
                              const char* shortName,
                              int32_t*    pInt,
                              int32_t     defaultValue,
                              const char* helpString) {
        SkFlagInfo* info = new SkFlagInfo(name, shortName, kInt_FlagType, helpString, nullptr);
        info->fIntValue  = pInt;
        *info->fIntValue = info->fDefaultInt = defaultValue;
        return true;
    }

    /**
     *  See comments for CreateBoolFlag.
     */
    static bool CreateUintFlag(const char* name,
                               uint32_t*   pUint,
                               uint32_t    defaultValue,
                               const char* helpString) {
        SkFlagInfo* info  = new SkFlagInfo(name, nullptr, kUint_FlagType, helpString, nullptr);
        info->fUintValue  = pUint;
        *info->fUintValue = info->fDefaultUint = defaultValue;
        return true;
    }

    static bool CreateUintFlag(const char* name,
                               const char* shortName,
                               uint32_t*   pUint,
                               uint32_t    defaultValue,
                               const char* helpString) {
        SkFlagInfo* info  = new SkFlagInfo(name, shortName, kUint_FlagType, helpString, nullptr);
        info->fUintValue  = pUint;
        *info->fUintValue = info->fDefaultUint = defaultValue;
        return true;
    }

    /**
     *  See comments for CreateBoolFlag.
     */
    static bool CreateDoubleFlag(const char* name,
                                 double*     pDouble,
                                 double      defaultValue,
                                 const char* helpString) {
        SkFlagInfo* info    = new SkFlagInfo(name, nullptr, kDouble_FlagType, helpString, nullptr);
        info->fDoubleValue  = pDouble;
        *info->fDoubleValue = info->fDefaultDouble = defaultValue;
        return true;
    }

    /**
     *  Returns true if the string matches this flag.
     *  For a boolean flag, also sets the value, since a boolean flag can be set in a number of ways
     *  without looking at the following string:
     *      --name
     *      --noname
     *      --name=true
     *      --name=false
     *      --name=1
     *      --name=0
     *      --name=TRUE
     *      --name=FALSE
     */
    bool match(const char* string);

    FlagTypes getFlagType() const { return fFlagType; }

    void resetStrings() {
        if (kString_FlagType == fFlagType) {
            fStrings->reset();
        } else {
            SkDEBUGFAIL("Can only call resetStrings on kString_FlagType");
        }
    }

    void append(const char* string) {
        if (kString_FlagType == fFlagType) {
            fStrings->append(string);
        } else {
            SkDEBUGFAIL("Can only append to kString_FlagType");
        }
    }

    void setInt(int32_t value) {
        if (kInt_FlagType == fFlagType) {
            *fIntValue = value;
        } else {
            SkDEBUGFAIL("Can only call setInt on kInt_FlagType");
        }
    }

    void setUint(uint32_t value) {
        if (kUint_FlagType == fFlagType) {
            *fUintValue = value;
        } else {
            SkDEBUGFAIL("Can only call setUint on kUint_FlagType");
        }
    }

    void setDouble(double value) {
        if (kDouble_FlagType == fFlagType) {
            *fDoubleValue = value;
        } else {
            SkDEBUGFAIL("Can only call setDouble on kDouble_FlagType");
        }
    }

    void setBool(bool value) {
        if (kBool_FlagType == fFlagType) {
            *fBoolValue = value;
        } else {
            SkDEBUGFAIL("Can only call setBool on kBool_FlagType");
        }
    }

    SkFlagInfo* next() { return fNext; }

    const SkString& name() const { return fName; }

    const SkString& shortName() const { return fShortName; }

    const SkString& help() const { return fHelpString; }
    const SkString& extendedHelp() const { return fExtendedHelpString; }

    SkString defaultValue() const {
        SkString result;
        switch (fFlagType) {
            case SkFlagInfo::kBool_FlagType:
                result.printf("%s", fDefaultBool ? "true" : "false");
                break;
            case SkFlagInfo::kString_FlagType: return fDefaultString;
            case SkFlagInfo::kInt_FlagType: result.printf("%i", fDefaultInt); break;
            case SkFlagInfo::kUint_FlagType: result.printf("0x%08x", fDefaultUint); break;
            case SkFlagInfo::kDouble_FlagType: result.printf("%2.2f", fDefaultDouble); break;
            default: SkDEBUGFAIL("Invalid flag type");
        }
        return result;
    }

    SkString typeAsString() const {
        switch (fFlagType) {
            case SkFlagInfo::kBool_FlagType: return SkString("bool");
            case SkFlagInfo::kString_FlagType: return SkString("string");
            case SkFlagInfo::kInt_FlagType: return SkString("int");
            case SkFlagInfo::kUint_FlagType: return SkString("uint");
            case SkFlagInfo::kDouble_FlagType: return SkString("double");
            default: SkDEBUGFAIL("Invalid flag type"); return SkString();
        }
    }

private:
    SkFlagInfo(const char* name,
               const char* shortName,
               FlagTypes   type,
               const char* helpString,
               const char* extendedHelpString)
            : fName(name)
            , fShortName(shortName)
            , fFlagType(type)
            , fHelpString(helpString)
            , fExtendedHelpString(extendedHelpString)
            , fBoolValue(nullptr)
            , fDefaultBool(false)
            , fIntValue(nullptr)
            , fDefaultInt(0)
            , fUintValue(nullptr)
            , fDefaultUint(0)
            , fDoubleValue(nullptr)
            , fDefaultDouble(0)
            , fStrings(nullptr) {
        fNext                   = CommandLineFlags::gHead;
        CommandLineFlags::gHead = this;
        SkASSERT(name && strlen(name) > 1);
        SkASSERT(nullptr == shortName || 1 == strlen(shortName));
    }

    /**
     *  Set a StringArray to hold the values stored in defaultStrings.
     *  @param array The StringArray to modify.
     *  @param defaultStrings Space separated list of strings that should be inserted into array
     *      individually.
     */
    static void SetDefaultStrings(CommandLineFlags::StringArray* array, const char* defaultStrings);

    // Name of the flag, without initial dashes
    SkString                       fName;
    SkString                       fShortName;
    FlagTypes                      fFlagType;
    SkString                       fHelpString;
    SkString                       fExtendedHelpString;
    bool*                          fBoolValue;
    bool                           fDefaultBool;
    int32_t*                       fIntValue;
    int32_t                        fDefaultInt;
    uint32_t*                      fUintValue;
    uint32_t                       fDefaultUint;
    double*                        fDoubleValue;
    double                         fDefaultDouble;
    CommandLineFlags::StringArray* fStrings;
    // Both for the help string and in case fStrings is empty.
    SkString fDefaultString;

    // In order to keep a linked list.
    SkFlagInfo* fNext;
};
#endif  // SK_COMMAND_LINE_FLAGS_H
