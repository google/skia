/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "path_utils.h"
#include "SkPath.h"
#include "SkStream.h"

namespace sk_tools {
    static int gCurPathID = 0;

    void dump_path_prefix(SkFILEWStream* pathStream) {
        if (NULL == pathStream) {
            return;
        }

        pathStream->writeText("#include \"SkScalar.h\"\n");
        pathStream->writeText("#include \"SkPoint.h\"\n");
        pathStream->writeText("#include \"SkBitmap.h\"\n");
        pathStream->writeText("#include \"SkDevice.h\"\n");
        pathStream->writeText("#include \"SkString.h\"\n");
        pathStream->writeText("#include \"SkImageEncoder.h\"\n");
    }

    void dump_path(SkFILEWStream* pathStream, const SkPath& path) {
        if (NULL == pathStream) {
            return;
        }

        static const int kMaxPts = 200;
        static const int kMaxVerbs = 200;

        int numPts = path.countPoints();
        int numVerbs = path.countVerbs();

        SkASSERT(numPts <= kMaxPts);
        SkASSERT(numVerbs <= kMaxVerbs);

        SkPoint pts[kMaxPts];
        uint8_t verbs[kMaxVerbs];

        path.getPoints(pts, kMaxPts);
        path.getVerbs(verbs, kMaxVerbs);

        const char* gStrs[] = {
            "kMove_Verb",
            "kLine_Verb",
            "kQuad_Verb",
            "kCubic_Verb",
            "kClose_Verb",
            "kDone_Verb"
        };

        pathStream->writeText("static const int numPts");
        pathStream->writeDecAsText(gCurPathID);
        pathStream->writeText(" = ");
        pathStream->writeDecAsText(numPts);
        pathStream->writeText(";\n");

        pathStream->writeText("SkPoint pts");
        pathStream->writeDecAsText(gCurPathID);
        pathStream->writeText("[] = {\n");

        for (int i = 0; i < numPts; ++i) {
            SkString temp;

            pathStream->writeText("      { ");
            temp.appendScalar(pts[i].fX);
            temp.append("f, ");
            temp.appendScalar(pts[i].fY);
            temp.append("f },\n");
            pathStream->writeText(temp.c_str());
        }
        pathStream->writeText("};\n");

        pathStream->writeText("static const int numVerbs");
        pathStream->writeDecAsText(gCurPathID);
        pathStream->writeText(" = ");
        pathStream->writeDecAsText(numVerbs);
        pathStream->writeText(";\n");

        pathStream->writeText("uint8_t verbs");
        pathStream->writeDecAsText(gCurPathID);
        pathStream->writeText("[] = {\n");

        for (int i = 0; i < numVerbs; ++i) {
            pathStream->writeText("\tSkPath::");
            pathStream->writeText(gStrs[verbs[i]]);
            pathStream->writeText(",\n");
        }
        pathStream->writeText("};\n");

        gCurPathID++;
    }

    void dump_path_suffix(SkFILEWStream* pathStream) {
        if (NULL == pathStream) {
            return;
        }

        pathStream->writeText("int numPaths = ");
        pathStream->writeDecAsText(gCurPathID);
        pathStream->writeText(";\n");

        pathStream->writeText("int sizes[] = {\n");
        for (int i = 0; i < gCurPathID; ++i) {
            pathStream->writeText("\t numPts");
            pathStream->writeDecAsText(i);
            pathStream->writeText(", numVerbs");
            pathStream->writeDecAsText(i);
            pathStream->writeText(",\n");
        }
        pathStream->writeText("};\n");

        pathStream->writeText("const SkPoint* points[] = {\n");
        for (int i = 0; i < gCurPathID; ++i) {
            pathStream->writeText("\tpts");
            pathStream->writeDecAsText(i);
            pathStream->writeText(",\n");
        }
        pathStream->writeText("};\n");

        pathStream->writeText("const uint8_t* verbs[] = {\n");
        for (int i = 0; i < gCurPathID; ++i) {
            pathStream->writeText("\t(const uint8_t*)verbs");
            pathStream->writeDecAsText(i);
            pathStream->writeText(",\n");
        }
        pathStream->writeText("};\n");
    }
}
