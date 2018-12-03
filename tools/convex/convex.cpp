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
    int fMoves;
    int fLines;
    int fQuads;
    int fConics;
    int fCubics;
    int fCloses;
    int fCount;
    bool fIsConvex;
    bool fIsRect;
};

int main(int argc, char** const argv) {
    string pathFileName = FLAGS_paths.count() ? FLAGS_paths[0] :
        "C://Users/Cary Clark/Downloads/skpout.txt";
    assert(file_exists(pathFileName));
    map<size_t, Path> pathMap;
    map<string, int> convexTypes;
    string line;
    string pathString;
    SkPath path;
    std::ifstream infile(pathFileName);
    int moves = 0;
    int lines = 0;
    int quads = 0;
    int conics = 0;
    int cubics = 0;
    int closes = 0;
    int total = 0;
    auto add_path = [&]() {
        if (!pathString.empty()) {
            if (path.isConvex()) {
                string key = "m" + std::to_string(moves)
                        + "l" + std::to_string(lines)
                        + "q" + std::to_string(quads)
                        + "k" + std::to_string(conics)
                        + "c" + std::to_string(cubics)
                        + "z" + std::to_string(closes);
                auto convexIter = convexTypes.find(key);
                if (convexTypes.end() == convexIter) {
                    convexTypes[key] = 1;
                } else {
                    convexIter->second++;
                }
            }
            size_t pathHash = std::hash<string>{}(pathString);
            auto iter = pathMap.find(pathHash);
            if (pathMap.end() == iter) {
                Path entry = { pathString, path, moves, lines, quads, conics, cubics, closes,
                        1, path.isConvex(), path.isRect(nullptr) };
                pathMap[pathHash] = entry;
            } else {
                iter->second.fCount += 1;
            }
            path.reset();
        }
    };
    while (std::getline(infile, line)) {
        assert("path." == line.substr(0, 5));
        string part = line.substr(5);
        if (evenOdd == part || winding == part) {
            add_path();
            path.setFillType(evenOdd == part ?
                    SkPath::kEvenOdd_FillType : SkPath::kWinding_FillType);
            pathString = line;
            moves = lines = quads = conics = cubics = closes = 0;
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
                    ++moves;
                } else if ("lineTo(" == verb.first) {
                    check_params(params, 2);
                    path.lineTo(params[0], params[1]);
                    ++lines;
                } else if ("quadTo(" == verb.first) {
                    check_params(params, 4);
                    path.quadTo(params[0], params[1], params[2], params[3]);
                    ++quads;
                } else if ("conicTo(" == verb.first) {
                    check_params(params, 5);
                    path.conicTo(params[0], params[1], params[2], params[3], params[4]);
                    ++conics;
                } else if ("cubicTo(" == verb.first) {
                    check_params(params, 6);
                    path.cubicTo(params[0], params[1], params[2], params[3], params[4], params[5]);
                    ++cubics;
                } else if ("close(" == verb.first) {
                    check_params(params, 0);
                    path.close();
                    ++closes;
                    SkDEBUGCODE(sawClose = true);
                } else {
                    assert(0);
                }
                break;
            }
        }
        assert(sawClose || "));  //" == part.substr(0, 7));
    }
    add_path();
    int convex = 0;
    int concave = 0;
    int rects = 0;
    int max = 0;
    for (auto& onePath : pathMap) {
        if (onePath.second.fIsRect) {
            rects += 1;
        }
        if (onePath.second.fIsConvex) {
            convex += 1;
        } else {
            concave += 1;
        }
        max = std::max(max, onePath.second.fCount);
    }
    SkDebugf("\ntotal = %d\n", total);
    SkDebugf("convex = %d\n", convex);
    SkDebugf("  rects = %d\n", rects);
    SkDebugf("concave = %d\n", concave);
    SkDebugf("max = %d\n\n", max);
    vector<pair<string, int> > sorted;
    for (auto convexType : convexTypes) {
        sorted.push_back(convexType);
    }
    std::sort(sorted.begin(), sorted.end(), [=](pair<string, int>& a, pair<string, int>& b) {
        return a.second > b.second;
    } );
    for (auto entry : sorted) {
        string key = entry.first;
        int index = 0;
        SkDebugf("[%d] ", entry.second);
        for (char c : "mlqkcz") {
            assert(c == key[index++]);
            string count;
            while (isdigit(key[index])) {
                count += key[index++];
            }
            switch (c) {
                case 'm':
                    if ("1" != count) SkDebugf("moves: %s  ", count.c_str());
                    break;
                case 'l':
                    if ("0" != count) SkDebugf("lines: %s  ", count.c_str());
                    break;
                case 'q':
                    if ("0" != count) SkDebugf("quads: %s  ", count.c_str());
                    break;
                case 'k':
                    if ("0" != count) SkDebugf("conics: %s  ", count.c_str());
                    break;
                case 'c':
                    if ("0" != count) SkDebugf("cubics: %s  ", count.c_str());
                    break;
                case 'z':
                    if ("1" != count) SkDebugf("closes: %s  ", count.c_str());
                    break;
                case '\0':
                    break;
                default:
                    assert(0);
            }
        }
        SkDebugf("\n");
    }
}
