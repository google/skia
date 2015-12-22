/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCommandLineFlags.h"
#include "SkTDArray.h"
#include "SkTSort.h"

#include <stdlib.h>

#if defined(GOOGLE3) && (defined(SK_BUILD_FOR_ANDROID) || defined(SK_BUILD_FOR_IOS))
    // I don't know why, but this is defined by //base only for non-Linux.
    DECLARE_bool(undefok)
#else
    DEFINE_bool(undefok, false, "Silently ignore unknown flags instead of crashing.");
#endif

template <typename T> static void ignore_result(const T&) {}

bool SkFlagInfo::CreateStringFlag(const char* name, const char* shortName,
                                  SkCommandLineFlags::StringArray* pStrings,
                                  const char* defaultValue, const char* helpString,
                                  const char* extendedHelpString) {
    SkFlagInfo* info = new SkFlagInfo(name, shortName, kString_FlagType, helpString,
                                      extendedHelpString);
    info->fDefaultString.set(defaultValue);

    info->fStrings = pStrings;
    SetDefaultStrings(pStrings, defaultValue);
    return true;
}

void SkFlagInfo::SetDefaultStrings(SkCommandLineFlags::StringArray* pStrings,
                                   const char* defaultValue) {
    pStrings->reset();
    if (nullptr == defaultValue) {
        return;
    }
    // If default is "", leave the array empty.
    size_t defaultLength = strlen(defaultValue);
    if (defaultLength > 0) {
        const char* const defaultEnd = defaultValue + defaultLength;
        const char* begin = defaultValue;
        while (true) {
            while (begin < defaultEnd && ' ' == *begin) {
                begin++;
            }
            if (begin < defaultEnd) {
                const char* end = begin + 1;
                while (end < defaultEnd && ' ' != *end) {
                    end++;
                }
                size_t length = end - begin;
                pStrings->append(begin, length);
                begin = end + 1;
            } else {
                break;
            }
        }
    }
}

static bool string_is_in(const char* target, const char* set[], size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (0 == strcmp(target, set[i])) {
            return true;
        }
    }
    return false;
}

/**
 *  Check to see whether string represents a boolean value.
 *  @param string C style string to parse.
 *  @param result Pointer to a boolean which will be set to the value in the string, if the
 *      string represents a boolean.
 *  @param boolean True if the string represents a boolean, false otherwise.
 */
static bool parse_bool_arg(const char* string, bool* result) {
    static const char* trueValues[] = { "1", "TRUE", "true" };
    if (string_is_in(string, trueValues, SK_ARRAY_COUNT(trueValues))) {
        *result = true;
        return true;
    }
    static const char* falseValues[] = { "0", "FALSE", "false" };
    if (string_is_in(string, falseValues, SK_ARRAY_COUNT(falseValues))) {
        *result = false;
        return true;
    }
    SkDebugf("Parameter \"%s\" not supported.\n", string);
    return false;
}

bool SkFlagInfo::match(const char* string) {
    if (SkStrStartsWith(string, '-') && strlen(string) > 1) {
        string++;
        const SkString* compareName;
        if (SkStrStartsWith(string, '-') && strlen(string) > 1) {
            string++;
            // There were two dashes. Compare against full name.
            compareName = &fName;
        } else {
            // One dash. Compare against the short name.
            compareName = &fShortName;
        }
        if (kBool_FlagType == fFlagType) {
            // In this case, go ahead and set the value.
            if (compareName->equals(string)) {
                *fBoolValue = true;
                return true;
            }
            if (SkStrStartsWith(string, "no") && strlen(string) > 2) {
                string += 2;
                // Only allow "no" to be prepended to the full name.
                if (fName.equals(string)) {
                    *fBoolValue = false;
                    return true;
                }
                return false;
            }
            int equalIndex = SkStrFind(string, "=");
            if (equalIndex > 0) {
                // The string has an equal sign. Check to see if the string matches.
                SkString flag(string, equalIndex);
                if (flag.equals(*compareName)) {
                    // Check to see if the remainder beyond the equal sign is true or false:
                    string += equalIndex + 1;
                    parse_bool_arg(string, fBoolValue);
                    return true;
                } else {
                    return false;
                }
            }
        }
        return compareName->equals(string);
    } else {
        // Has no dash
        return false;
    }
    return false;
}

SkFlagInfo* SkCommandLineFlags::gHead;
SkString SkCommandLineFlags::gUsage;

void SkCommandLineFlags::SetUsage(const char* usage) {
    gUsage.set(usage);
}

// Maximum line length for the help message.
#define LINE_LENGTH 72

static void print_indented(const SkString& text) {
    size_t length = text.size();
    const char* currLine = text.c_str();
    const char* stop = currLine + length;
    while (currLine < stop) {
        int lineBreak = SkStrFind(currLine, "\n");
        if (lineBreak < 0) {
            lineBreak = static_cast<int>(strlen(currLine));
        }
        if (lineBreak > LINE_LENGTH) {
            // No line break within line length. Will need to insert one.
            // Find a space before the line break.
            int spaceIndex = LINE_LENGTH - 1;
            while (spaceIndex > 0 && currLine[spaceIndex] != ' ') {
                spaceIndex--;
            }
            int gap;
            if (0 == spaceIndex) {
                // No spaces on the entire line. Go ahead and break mid word.
                spaceIndex = LINE_LENGTH;
                gap = 0;
            } else {
                // Skip the space on the next line
                gap = 1;
            }
            SkDebugf("        %.*s\n", spaceIndex, currLine);
            currLine += spaceIndex + gap;
        } else {
            // the line break is within the limit. Break there.
            lineBreak++;
            SkDebugf("        %.*s", lineBreak, currLine);
            currLine += lineBreak;
        }
    }
}

static void print_help_for_flag(const SkFlagInfo* flag) {
    SkDebugf("    --%s", flag->name().c_str());
    const SkString& shortName = flag->shortName();
    if (shortName.size() > 0) {
        SkDebugf(" or -%s", shortName.c_str());
    }
    SkDebugf(":\ttype: %s", flag->typeAsString().c_str());
    if (flag->defaultValue().size() > 0) {
        SkDebugf("\tdefault: %s", flag->defaultValue().c_str());
    }
    SkDebugf("\n");
    const SkString& help = flag->help();
    print_indented(help);
    SkDebugf("\n");
}
static void print_extended_help_for_flag(const SkFlagInfo* flag) {
    print_help_for_flag(flag);
    print_indented(flag->extendedHelp());
    SkDebugf("\n");
}

namespace {
struct CompareFlagsByName {
    bool operator()(SkFlagInfo* a, SkFlagInfo* b) const {
        return strcmp(a->name().c_str(), b->name().c_str()) < 0;
    }
};
}  // namespace

void SkCommandLineFlags::Parse(int argc, char** argv) {
    // Only allow calling this function once.
    static bool gOnce;
    if (gOnce) {
        SkDebugf("Parse should only be called once at the beginning of main!\n");
        SkASSERT(false);
        return;
    }
    gOnce = true;

    bool helpPrinted = false;
    // Loop over argv, starting with 1, since the first is just the name of the program.
    for (int i = 1; i < argc; i++) {
        if (0 == strcmp("-h", argv[i]) || 0 == strcmp("--help", argv[i])) {
            // Print help message.
            SkTDArray<const char*> helpFlags;
            for (int j = i + 1; j < argc; j++) {
                if (SkStrStartsWith(argv[j], '-')) {
                    break;
                }
                helpFlags.append(1, &argv[j]);
            }
            if (0 == helpFlags.count()) {
                // Only print general help message if help for specific flags is not requested.
                SkDebugf("%s\n%s\n", argv[0], gUsage.c_str());
            }
            SkDebugf("Flags:\n");

            if (0 == helpFlags.count()) {
                // If no flags followed --help, print them all
                SkTDArray<SkFlagInfo*> allFlags;
                for (SkFlagInfo* flag = SkCommandLineFlags::gHead; flag;
                     flag = flag->next()) {
                    allFlags.push(flag);
                }
                SkTQSort(&allFlags[0], &allFlags[allFlags.count() - 1],
                         CompareFlagsByName());
                for (int i = 0; i < allFlags.count(); ++i) {
                    print_help_for_flag(allFlags[i]);
                    if (allFlags[i]->extendedHelp().size() > 0) {
                        SkDebugf("        Use '--help %s' for more information.\n",
                                 allFlags[i]->name().c_str());
                    }
                }
            } else {
                for (SkFlagInfo* flag = SkCommandLineFlags::gHead; flag;
                     flag = flag->next()) {
                    for (int k = 0; k < helpFlags.count(); k++) {
                        if (flag->name().equals(helpFlags[k]) ||
                            flag->shortName().equals(helpFlags[k])) {
                            print_extended_help_for_flag(flag);
                            helpFlags.remove(k);
                            break;
                        }
                    }
                }
            }
            if (helpFlags.count() > 0) {
                SkDebugf("Requested help for unrecognized flags:\n");
                for (int k = 0; k < helpFlags.count(); k++) {
                    SkDebugf("    --%s\n", helpFlags[k]);
                }
            }
            helpPrinted = true;
        }
        if (!helpPrinted) {
            bool flagMatched = false;
            SkFlagInfo* flag = gHead;
            while (flag != nullptr) {
                if (flag->match(argv[i])) {
                    flagMatched = true;
                    switch (flag->getFlagType()) {
                        case SkFlagInfo::kBool_FlagType:
                            // Can be handled by match, above, but can also be set by the next
                            // string.
                            if (i+1 < argc && !SkStrStartsWith(argv[i+1], '-')) {
                                i++;
                                bool value;
                                if (parse_bool_arg(argv[i], &value)) {
                                    flag->setBool(value);
                                }
                            }
                            break;
                        case SkFlagInfo::kString_FlagType:
                            flag->resetStrings();
                            // Add all arguments until another flag is reached.
                            while (i+1 < argc) {
                                char* end = nullptr;
                                // Negative numbers aren't flags.
                                ignore_result(strtod(argv[i+1], &end));
                                if (end == argv[i+1] && SkStrStartsWith(argv[i+1], '-')) {
                                    break;
                                }
                                i++;
                                flag->append(argv[i]);
                            }
                            break;
                        case SkFlagInfo::kInt_FlagType:
                            i++;
                            flag->setInt(atoi(argv[i]));
                            break;
                        case SkFlagInfo::kDouble_FlagType:
                            i++;
                            flag->setDouble(atof(argv[i]));
                            break;
                        default:
                            SkDEBUGFAIL("Invalid flag type");
                    }
                    break;
                }
                flag = flag->next();
            }
            if (!flagMatched) {
#if SK_BUILD_FOR_MAC
                if (SkStrStartsWith(argv[i], "NSDocumentRevisions")
                        || SkStrStartsWith(argv[i], "-NSDocumentRevisions")) {
                    i++;  // skip YES
                } else
#endif
                if (FLAGS_undefok) {
                    SkDebugf("FYI: ignoring unknown flag '%s'.\n", argv[i]);
                } else {
                    SkDebugf("Got unknown flag '%s'. Exiting.\n", argv[i]);
                    exit(-1);
                }
            }
        }
    }
    // Since all of the flags have been set, release the memory used by each
    // flag. FLAGS_x can still be used after this.
    SkFlagInfo* flag = gHead;
    gHead = nullptr;
    while (flag != nullptr) {
        SkFlagInfo* next = flag->next();
        delete flag;
        flag = next;
    }
    if (helpPrinted) {
        exit(0);
    }
}

namespace {

template <typename Strings>
bool ShouldSkipImpl(const Strings& strings, const char* name) {
    int count = strings.count();
    size_t testLen = strlen(name);
    bool anyExclude = count == 0;
    for (int i = 0; i < strings.count(); ++i) {
        const char* matchName = strings[i];
        size_t matchLen = strlen(matchName);
        bool matchExclude, matchStart, matchEnd;
        if ((matchExclude = matchName[0] == '~')) {
            anyExclude = true;
            matchName++;
            matchLen--;
        }
        if ((matchStart = matchName[0] == '^')) {
            matchName++;
            matchLen--;
        }
        if ((matchEnd = matchName[matchLen - 1] == '$')) {
            matchLen--;
        }
        if (matchStart ? (!matchEnd || matchLen == testLen)
                && strncmp(name, matchName, matchLen) == 0
                : matchEnd ? matchLen <= testLen
                && strncmp(name + testLen - matchLen, matchName, matchLen) == 0
                : strstr(name, matchName) != 0) {
            return matchExclude;
        }
    }
    return !anyExclude;
}

}  // namespace

bool SkCommandLineFlags::ShouldSkip(const SkTDArray<const char*>& strings, const char* name) {
    return ShouldSkipImpl(strings, name);
}
bool SkCommandLineFlags::ShouldSkip(const StringArray& strings, const char* name) {
    return ShouldSkipImpl(strings, name);
}
