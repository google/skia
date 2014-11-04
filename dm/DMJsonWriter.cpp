/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "DMJsonWriter.h"

#include "SkCommonFlags.h"
#include "SkJSONCPP.h"
#include "SkOSFile.h"
#include "SkStream.h"
#include "SkTArray.h"
#include "SkThread.h"

namespace DM {

SkTArray<JsonWriter::BitmapResult> gBitmapResults;
SK_DECLARE_STATIC_MUTEX(gBitmapResultLock);

void JsonWriter::AddBitmapResult(const BitmapResult& result) {
    SkAutoMutexAcquire lock(&gBitmapResultLock);
    gBitmapResults.push_back(result);
}

void JsonWriter::DumpJson() {
    if (FLAGS_writePath.isEmpty()) {
        return;
    }

    Json::Value root;

    for (int i = 1; i < FLAGS_properties.count(); i += 2) {
        root[FLAGS_properties[i-1]] = FLAGS_properties[i];
    }
    for (int i = 1; i < FLAGS_key.count(); i += 2) {
        root["key"][FLAGS_key[i-1]] = FLAGS_key[i];
    }

    {
        SkAutoMutexAcquire lock(&gBitmapResultLock);
        for (int i = 0; i < gBitmapResults.count(); i++) {
            Json::Value result;
            result["key"]["name"]            = gBitmapResults[i].name.c_str();
            result["key"]["config"]          = gBitmapResults[i].config.c_str();
            result["key"]["mode"]            = gBitmapResults[i].mode.c_str();
            result["options"]["source_type"] = gBitmapResults[i].sourceType.c_str();
            result["md5"]                    = gBitmapResults[i].md5.c_str();

            root["results"].append(result);
        }
    }

    SkString path = SkOSPath::Join(FLAGS_writePath[0], "dm.json");
    SkFILEWStream stream(path.c_str());
    stream.writeText(Json::StyledWriter().write(root).c_str());
    stream.flush();
}

} // namespace DM
