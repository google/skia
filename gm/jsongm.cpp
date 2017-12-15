/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkParsePath.h"
#include "SkPath.h"
#include "SkJSONCPP.h"
#include "Resources.h"

static const char* name_type(Json::ValueType t) {
    const char* gTypes[] = {
        "null", "int", "uint", "real", "string", "bool", "array", "object",
    };
    return gTypes[t];
}

static void dump(Json::Value v, int tabs = 0) {
    for (int i = 0; i < tabs; ++i) {
        SkDebugf("\t");
    }
    switch (v.type()) {
        case Json::nullValue:    SkDebugf("null\n"); break;
        case Json::intValue:     SkDebugf("int %d\n", v.asInt()); break;
        case Json::uintValue:    SkDebugf("uint %x\n", v.asInt()); break;
        case Json::realValue:    SkDebugf("real %g\n", v.asFloat()); break;
        case Json::stringValue:  SkDebugf("string %s\n", v.asCString()); break;
        case Json::booleanValue: SkDebugf("bool %d\n", v.asBool()); break;
        case Json::arrayValue: {
            int count = v.size();
            SkDebugf("array [%d]\n", count);
            for (int i = 0; i < count; ++i) {
                dump(v[i], tabs + 1);
            }
            break;
        }
        case Json::objectValue: {
            SkDebugf("object\n");
            auto iter = v.begin();
            auto stop = v.end();
            while (iter != stop) {
                dump(*iter, tabs + 1);
                ++iter;
            }
            break;
        }
    }
}

DEF_SIMPLE_GM(json, canvas, 500, 600) {
    auto data = GetResourceAsData("test.json");
    const char* str = (const char*)data->data();
    const char* end = str + data->size();

    Json::Reader reader;
    Json::Value root;

    if (!reader.parse(str, end, root) || !reader.good()) {
        std::string err = reader.getFormattedErrorMessages();
        SkDebugf("failed to parse json: %s\n", err.c_str());
        return;
    }

    SkDebugf("root is a %s\n", name_type(root.type()));
    dump(root);
}
