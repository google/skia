/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "dm/DMJsonWriter.h"

#include "include/core/SkData.h"
#include "include/core/SkStream.h"
#include "include/private/SkMutex.h"
#include "include/private/SkTArray.h"
#include "src/core/SkOSFile.h"
#include "src/utils/SkJSON.h"
#include "src/utils/SkJSONWriter.h"
#include "src/utils/SkOSPath.h"
#include "tools/ProcStats.h"

namespace DM {

SkTArray<JsonWriter::BitmapResult> gBitmapResults;
SK_DECLARE_STATIC_MUTEX(gBitmapResultLock);

void JsonWriter::AddBitmapResult(const BitmapResult& result) {
    SkAutoMutexAcquire lock(&gBitmapResultLock);
    gBitmapResults.push_back(result);
}

void JsonWriter::DumpJson(const char* dir,
                          CommandLineFlags::StringArray key,
                          CommandLineFlags::StringArray properties) {
    if (0 == strcmp(dir, "")) {
        return;
    }

    SkString path = SkOSPath::Join(dir, "dm.json");
    sk_mkdir(dir);
    SkFILEWStream stream(path.c_str());
    SkJSONWriter writer(&stream, SkJSONWriter::Mode::kPretty);

    writer.beginObject(); // root

    for (int i = 1; i < properties.count(); i += 2) {
        writer.appendString(properties[i-1], properties[i]);
    }

    writer.beginObject("key");
    for (int i = 1; i < key.count(); i += 2) {
        writer.appendString(key[i-1], key[i]);
    }
    writer.endObject();

    int maxResidentSetSizeMB = sk_tools::getMaxResidentSetSizeMB();
    if (maxResidentSetSizeMB != -1) {
        writer.appendS32("max_rss_MB", maxResidentSetSizeMB);
    }

    {
        SkAutoMutexAcquire lock(&gBitmapResultLock);
        writer.beginArray("results");
        for (int i = 0; i < gBitmapResults.count(); i++) {
            writer.beginObject();

            writer.beginObject("key");
            writer.appendString("name"       , gBitmapResults[i].name.c_str());
            writer.appendString("config"     , gBitmapResults[i].config.c_str());
            writer.appendString("source_type", gBitmapResults[i].sourceType.c_str());

            // Source options only need to be part of the key if they exist.
            // Source type by source type, we either always set options or never set options.
            if (!gBitmapResults[i].sourceOptions.isEmpty()) {
                writer.appendString("source_options", gBitmapResults[i].sourceOptions.c_str());
            }
            writer.endObject(); // key

            writer.beginObject("options");
            writer.appendString("ext"  ,       gBitmapResults[i].ext.c_str());
            writer.appendString("gamut",       gBitmapResults[i].gamut.c_str());
            writer.appendString("transfer_fn", gBitmapResults[i].transferFn.c_str());
            writer.appendString("color_type",  gBitmapResults[i].colorType.c_str());
            writer.appendString("alpha_type",  gBitmapResults[i].alphaType.c_str());
            writer.appendString("color_depth", gBitmapResults[i].colorDepth.c_str());
            writer.endObject(); // options

            writer.appendString("md5", gBitmapResults[i].md5.c_str());

            writer.endObject(); // 1 result
        }
        writer.endArray(); // results
    }

    writer.endObject(); // root
    writer.flush();
    stream.flush();
}

using namespace skjson;

bool JsonWriter::ReadJson(const char* path, void(*callback)(BitmapResult)) {
    sk_sp<SkData> json(SkData::MakeFromFileName(path));
    if (!json) {
        return false;
    }

    DOM dom((const char*)json->data(), json->size());
    const ObjectValue* root = dom.root();
    if (!root) {
        return false;
    }

    const ArrayValue* results = (*root)["results"];
    if (!results) {
        return false;
    }

    BitmapResult br;
    for (const ObjectValue* r : *results) {
        const ObjectValue& key = (*r)["key"].as<ObjectValue>();
        const ObjectValue& options = (*r)["options"].as<ObjectValue>();

        br.name         = key["name"].as<StringValue>().begin();
        br.config       = key["config"].as<StringValue>().begin();
        br.sourceType   = key["source_type"].as<StringValue>().begin();
        br.ext          = options["ext"].as<StringValue>().begin();
        br.gamut        = options["gamut"].as<StringValue>().begin();
        br.transferFn   = options["transfer_fn"].as<StringValue>().begin();
        br.colorType    = options["color_type"].as<StringValue>().begin();
        br.alphaType    = options["alpha_type"].as<StringValue>().begin();
        br.colorDepth   = options["color_depth"].as<StringValue>().begin();
        br.md5          = (*r)["md5"].as<StringValue>().begin();

        if (const StringValue* so = key["source_options"]) {
            br.sourceOptions = so->begin();
        }
        callback(br);
    }
    return true;
}

} // namespace DM
