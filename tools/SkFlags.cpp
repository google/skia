/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFlags.h"

SkFlagInfo* SkFlags::gHead;
SkString SkFlags::gUsage;

void SkFlags::SetUsage(const char* usage) {
    gUsage.set(usage);
}

// Maximum line length for the help message.
#define LINE_LENGTH 80

void SkFlags::ParseCommandLine(int argc, char** argv) {
    // Only allow calling this function once.
    static bool gOnce;
    if (gOnce) {
        SkDebugf("ParseCommandLine should only be called once at the beginning"
                 " of main!\n");
        SkASSERT(false);
        return;
    }
    gOnce = true;

    bool helpPrinted = false;
    // Loop over argv, starting with 1, since the first is just the name of the program.
    for (int i = 1; i < argc; i++) {
        if (0 == strcmp("-h", argv[i]) || 0 == strcmp("--h", argv[i])
                || 0 == strcmp("-help", argv[i]) || 0 == strcmp("--help", argv[i])) {
            // Print help message.
            SkDebugf("%s\n%s\n", argv[0], gUsage.c_str());
            SkDebugf("Flags:\n");
            SkFlagInfo* flag = SkFlags::gHead;
            while (flag != NULL) {
                SkDebugf("\t--%s:\ttype: %s", flag->name().c_str(),
                          flag->typeAsString().c_str());
                if (flag->defaultValue().size() > 0) {
                    SkDebugf("\tdefault: %s", flag->defaultValue().c_str());
                }
                SkDebugf("\n");
                const SkString& help = flag->help();
                size_t length = help.size();
                const char* currLine = help.c_str();
                const char* stop = currLine + length;
                while (currLine < stop) {
                    if (strlen(currLine) < LINE_LENGTH) {
                        // Only one line length's worth of text left.
                        SkDebugf("\t\t%s\n", currLine);
                        break;
                    }
                    int lineBreak = SkStrFind(currLine, "\n");
                    if (lineBreak < 0 || lineBreak > LINE_LENGTH) {
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
                        SkDebugf("\t\t%.*s\n", spaceIndex, currLine);
                        currLine += spaceIndex + gap;
                    } else {
                        // the line break is within the limit. Break there.
                        lineBreak++;
                        SkDebugf("\t\t%.*s", lineBreak, currLine);
                        currLine += lineBreak;
                    }
                }
                SkDebugf("\n");
                flag = flag->next();
            }
            helpPrinted = true;
        }
        if (!helpPrinted) {
            bool flagMatched = false;
            SkFlagInfo* flag = gHead;
            while (flag != NULL) {
                if (flag->match(argv[i])) {
                    flagMatched = true;
                    switch (flag->getFlagType()) {
                        case SkFlagInfo::kBool_FlagType:
                            // Handled by match, above
                            break;
                        case SkFlagInfo::kString_FlagType:
                            flag->resetStrings();
                            // Add all arguments until another flag is reached.
                            while (i+1 < argc && !SkStrStartsWith(argv[i+1], '-')) {
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
                            SkASSERT(!"Invalid flag type");
                    }
                    break;
                }
                flag = flag->next();
            }
            if (!flagMatched) {
                SkDebugf("skipping unknown flag %s\n", argv[i]);
            }
        }
    }
    // Since all of the flags have been set, release the memory used by each
    // flag. FLAGS_x can still be used after this.
    SkFlagInfo* flag = gHead;
    gHead = NULL;
    while (flag != NULL) {
        SkFlagInfo* next = flag->next();
        SkDELETE(flag);
        flag = next;
    }
    if (helpPrinted) {
        exit(0);
    }
}
