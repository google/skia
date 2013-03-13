#include "SkStream.h"
#include "SkString.h"
#include "SkTDArray.h"
#include <stdio.h>

static bool replace(const char* fun, const char* dir, const char* filename, const char* marker,
        const char* marker2, const char* replace, size_t replaceLen) {
    SkString outFileStr(dir);
    outFileStr.append(filename);
    SkFILEStream opStreamIn(outFileStr.c_str());
    if (!opStreamIn.isValid()) {
        SkDebugf("%s couldn't find %s\n", fun, outFileStr.c_str());
        return false;
    }
    SkTDArray<char> opData;
    opData.setCount(opStreamIn.getLength());
    size_t opLen = opData.count();
    opStreamIn.read(opData.begin(), opLen);
    opStreamIn.setPath(NULL);
    SkFILEWStream opStreamOut(outFileStr.c_str());
    if (!opStreamOut.isValid()) {
        SkDebugf("%s couldn't open for writing %s\n", fun, outFileStr.c_str());
        return false;
    }

    char* opInsert = strstr(opData.begin(), marker);
    if (!opInsert) {
        SkDebugf("%s missing marker in %s\n", fun, outFileStr.c_str());
        opStreamOut.write(opData.begin(), opLen);
        opStreamOut.flush();
        return false;
    }
    const char* opInsertEnd = opInsert + strlen(marker);
    if (marker2) {
        char* opInsert2 = strstr(opInsert, marker2);
        if (!opInsert2) {
            SkDebugf("%s missing marker second half in %s\n", fun, outFileStr.c_str());
            opStreamOut.write(opData.begin(), opLen);
            opStreamOut.flush();
            return false;
        }
        opInsertEnd = opInsert2 + strlen(marker2);
    }
    opStreamOut.write(opData.begin(), opInsert - opData.begin());
    opStreamOut.write(replace, replaceLen);
    opStreamOut.write(opInsertEnd, opLen - (opInsertEnd - opData.begin()));
    opStreamOut.flush();
    return true;
}

int main (int argc, char * const argv[]) {
    if (argc != 2) {
        SkDebugf("%s expected filename\n", argv[0]);
        return 0;
    }
    const char* dir = "../../experimental/Intersection/";
    SkString inFileStr;
    if (argv[1][0] != '/') {
        inFileStr.append(dir);
    }
    inFileStr.append(argv[1]);
    SkFILEStream inFile(inFileStr.c_str());
    if (!inFile.isValid()) {
        SkDebugf("%s couldn't find %s\n", argv[0], argv[1]);
        return 0;
    }
    SkTDArray<char> inData;
    inData.setCount(inFile.getLength());
    size_t inLen = inData.count();
    inFile.read(inData.begin(), inLen);
    inFile.setPath(NULL);
    char* insert = strstr(inData.begin(), "\n\n\n");
    if (!insert) {
        SkDebugf("%s missing two blank line delimiter in %s\n", argv[0], argv[1]);
        return 0;
    }
    insert += 1; // include first blank line
    const char opMarker[] =
            "</div>" "\n"
            "\n"
            "<script type=\"text/javascript\">" "\n"
            "\n"
            "var testDivs = ["  "\n"
            ;
    if (!replace(argv[0], dir, "op.htm", opMarker, NULL, inData.begin(),
            insert - inData.begin())) {
        return 0;
    }
    const char newMarker[] =
            "static void (*firstTest)() = "
            ;
    const char newMarker2[] =
            ";"  "\n"
            "\n"
            "static struct {"  "\n"
            "    void (*fun)();"  "\n"
            "    const char* str;"  "\n"
            "} tests[] = {"  "\n"
            ;
    if (!replace(argv[0], dir, "SimplifyNew_Test.cpp", newMarker, newMarker2, insert + 2,
            inLen - (insert - inData.begin()) - 2)) {
        return 0;
    }
    const char forceReleaseMarker[] =
            "#define FORCE_RELEASE 1  // set force release to 1 for multiple thread -- no debugging"
            ;
    const char forceReleaseReplace[] =
            "#define FORCE_RELEASE 0  // set force release to 1 for multiple thread -- no debugging"
            ;
    if (!replace(argv[0], dir, "DataTypes.h", forceReleaseMarker, NULL, forceReleaseReplace,
            sizeof(forceReleaseReplace) - 1)) {
        return 0;
    }
    return 0;
}
