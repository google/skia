/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <algorithm>
#include <assert.h>
#include <cmath>
#include <cctype>
#include <cstring>
#include <map>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using std::string;
using std::map;
using std::pair;
using std::vector;

#include "SkCommandLineFlags.h"
#include "SkPath.h"

DEFINE_string2(paths, p, "", "File containing paths to test for convexity");

bool file_exists(string fileName)
{
    std::ifstream infile(fileName);
    return infile.good();
}

void check_params(float params[6], int count) {
    for (int index = 0; index < 6; ++index) {
        assert((index >= count) == SkScalarIsNaN(params[index]));
    }
}

const string evenOdd = "setFillType(SkPath::kEvenOdd_FillType);";
const string winding = "setFillType(SkPath::kWinding_FillType);";
const vector<std::pair<string, int>> verbs = {
    {"moveTo(", 2},
    {"lineTo(", 2},
    {"quadTo(", 4},
    {"conicTo(", 5},
    {"cubicTo(", 6},
    {"close(", 0},
};

struct Path {
    string fString;
    SkPath fPath;
    int fCount;
    bool fIsConvex;
};

int main(int argc, char** const argv) {
    string pathFileName = FLAGS_paths.count() ? FLAGS_paths[0] :
        "C://Users/Cary Clark/Downloads/skpout.txt";
    assert(file_exists(pathFileName));
    map<size_t, Path> pathMap;
    string line;
    string pathString;
    SkPath path;
    std::ifstream infile(pathFileName);
    int total = 0;
    while (std::getline(infile, line)) {
        assert("path." == line.substr(0, 5));
        string part = line.substr(5);
        auto add_path = [](map<size_t, Path>& pathMap, SkPath& path, string pathString) {
            if (!pathString.empty()) {
                size_t pathHash = std::hash<string>{}(pathString);
                auto iter = pathMap.find(pathHash);
                if (pathMap.end() == iter) {
                    Path entry = { pathString, path, 1, path.isConvex() };
                    pathMap[pathHash] = entry;
                } else {
                    iter->second.fCount += 1;
                }
                path.reset();
            }
        };
        if (evenOdd == part) {
            add_path(pathMap, path, pathString);
            path.setFillType(SkPath::kEvenOdd_FillType);
            pathString = line;
            ++total;
            continue;
        }
        if (winding == part) {
            add_path(pathMap, path, pathString);
            path.setFillType(SkPath::kWinding_FillType);
            pathString = line;
            ++total;
            continue;
        }
        pathString += line;
        SkDEBUGCODE(bool sawClose = false);
        for (auto verb : verbs) {
            if (verb.first == part.substr(0, verb.first.length())) {
                part = part.substr(verb.first.length());
                float params[6] = { SK_ScalarNaN, SK_ScalarNaN, SK_ScalarNaN,
                                    SK_ScalarNaN, SK_ScalarNaN, SK_ScalarNaN };
                for (int index = 0; index < verb.second; ++index) {
                    const string bits = "SkBits2Float(";
                    assert(bits == part.substr(0, bits.length()));
                    part = part.substr(bits.length());
                    size_t eaten = 0;
                    unsigned int hex = std::stoul(part, &eaten, 16);
                    assert(10 == eaten);
                    params[index] = SkBits2Float(hex);
                    part = part.substr(eaten);
                    assert(')' == part[0]);
                    if (index + 1 < verb.second) {
                        assert(", " == part.substr(1, 2));
                        part = part.substr(3);
                    }
                }
                if ("moveTo(" == verb.first) {
                    check_params(params, 2);
                    path.moveTo(params[0], params[1]);
                } else if ("lineTo(" == verb.first) {
                    check_params(params, 2);
                    path.lineTo(params[0], params[1]);
                } else if ("quadTo(" == verb.first) {
                    check_params(params, 4);
                    path.quadTo(params[0], params[1], params[2], params[3]);
                } else if ("conicTo(" == verb.first) {
                    check_params(params, 5);
                    path.conicTo(params[0], params[1], params[2], params[3], params[4]);
                } else if ("cubicTo(" == verb.first) {
                    check_params(params, 6);
                    path.cubicTo(params[0], params[1], params[2], params[3], params[4], params[5]);
                } else if ("close(" == verb.first) {
                    check_params(params, 0);
                    path.close();
                    SkDEBUGCODE(sawClose = true);
                } else {
                    assert(0);
                }
                break;
            }
        }
        assert(sawClose || "));  //" == part.substr(0, 7));
    }
    int convex = 0;
    int concave = 0;
    int max = 0;
    for (auto& onePath : pathMap) {
        if (onePath.second.fIsConvex) {
            convex += 1;
        } else {
            concave += 1;
        }
        max = std::max(max, onePath.second.fCount);
    }
    SkDebugf("\ntotal = %d\n", total);
    SkDebugf("convex = %d\n", convex);
    SkDebugf("concave = %d\n", concave);
    SkDebugf("max = %d\n", max);
}
