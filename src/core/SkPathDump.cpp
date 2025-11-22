/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkSpan.h"
#include "include/core/SkStream.h"
#include "src/base/SkFloatBits.h"
#include "src/core/SkStringUtils.h"

#include <functional>

char const * const gFillTypeStrs[] = {
    "Winding",
    "EvenOdd",
    "InverseWinding",
    "InverseEvenOdd",
};

constexpr float kSentinelConicWeight = -12345;  // not a valid weight

static void append_params(SkString* str, const char label[], SkSpan<const SkPoint> pts,
                          SkScalarAsStringType strType, bool useSemicolon,
                          SkScalar conicWeight = kSentinelConicWeight) {
    str->append(label);
    str->append("(");

    const SkScalar* values = &pts[0].fX;
    size_t count = pts.size() * 2;

    for (size_t i = 0; i < count; ++i) {
        SkAppendScalar(str, values[i], strType);
        if (i < count - 1) {
            str->append(", ");
        }
    }
    if (conicWeight != kSentinelConicWeight) {
        str->append(", ");
        SkAppendScalar(str, conicWeight, strType);
    }
    str->append(useSemicolon ? ");" : ")");
    if (kHex_SkScalarAsStringType == strType) {
        str->append("  // ");
        for (size_t i = 0; i < count; ++i) {
            SkAppendScalarDec(str, values[i]);
            if (i < count - 1) {
                str->append(", ");
            }
        }
        if (conicWeight >= 0) {
            str->append(", ");
            SkAppendScalarDec(str, conicWeight);
        }
    }
    str->append("\n");
}

static void dump_iter(SkPathIter iter, SkString* builder, const char cmdPrefix[],
                      SkScalarAsStringType strType, bool useSemicolon,
                      std::function<void()> postVerbProc) {
    while (auto rec = iter.next()) {
        SkString cmd(cmdPrefix);
        SkSpan<const SkPoint> pts;
        float cw = kSentinelConicWeight;

        switch (rec->fVerb) {
            case SkPathVerb::kMove:
                cmd.append(".moveTo");
                pts = {&rec->fPoints[0], 1};
                break;
            case SkPathVerb::kLine:
                cmd.append(".lineTo");
                pts = {&rec->fPoints[1], 1};
                break;
            case SkPathVerb::kQuad:
                cmd.append(".quadTo");
                pts = {&rec->fPoints[1], 2};
                break;
            case SkPathVerb::kConic:
                cmd.append(".conicTo");
                pts = {&rec->fPoints[1], 2};
                cw = rec->conicWeight();
                break;
            case SkPathVerb::kCubic:
                cmd.append(".cubicTo");
                pts = {&rec->fPoints[1], 3};
                break;
            case SkPathVerb::kClose:
                cmd.append(".close()");
                if (useSemicolon) {
                    cmd.append(";");
                }
                cmd.append("\n");
                builder->append(cmd.c_str());
                break;
        }
        // we don't do this for kClose
        if (pts.size()) {
            append_params(builder, cmd.c_str(), pts, strType, useSemicolon, cw);
        }
        postVerbProc();
    }
}

void SkPath::dump(SkWStream* wStream, bool dumpAsHex) const {
    SkScalarAsStringType asType = dumpAsHex ? kHex_SkScalarAsStringType : kDec_SkScalarAsStringType;

    SkString builder;
    builder.printf("path.setFillType(SkPathFillType::k%s);\n",
            gFillTypeStrs[(int) this->getFillType()]);

    dump_iter(this->iter(), &builder, "path", asType, true, [&]() {
        if (!wStream && builder.size()) {
            SkDebugf("%s", builder.c_str());
            builder.reset();
        }
    });
    if (wStream) {
        wStream->writeText(builder.c_str());
    }
}

SkString SkPathBuilder::dumpToString(DumpFormat format) const {
    SkScalarAsStringType asType = format == DumpFormat::kHex ? kHex_SkScalarAsStringType
                                                             : kDec_SkScalarAsStringType;

    SkString builder;
    builder.printf("SkPathBuilder(SkPathFillType::k%s)\n",
                   gFillTypeStrs[(int) this->fillType()]);

    dump_iter(this->iter(), &builder, "", asType, false, [](){});

    return builder;
}

void SkPathBuilder::dump(DumpFormat format) const {
    SkDebugf("%s", dumpToString().c_str());
}
