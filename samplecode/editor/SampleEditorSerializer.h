/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SampleEditorSerializer_DEFINED
#define SampleEditorSerializer_DEFINED

#include "SampleEditorCommon.h"
#include "SampleEditorData.h"

#include "SkJSON.h"
#include "SkJSONWriter.h"
#include "SkStream.h"

/*
 * The serializer reads and writes JSON files. The serialization and deserialization are handled
 * by the data objects themselves.
 */
class EditorSerializer {
public:
    static void Serialize(EditorAnimation* animation, const char* filename) {
        // Create the write stream and writer.
        SkFILEWStream wstream(filename);
        SkJSONWriter writer(&wstream);

        writer.beginObject();

        // Width, height, and speed.
        writer.appendName("width");
        writer.appendFloat(animation->fSize.width());
        writer.appendName("height");
        writer.appendFloat(animation->fSize.height());
        writer.appendName("speed");
        writer.appendFloat(animation->fSpeed);

        writer.appendName("bones");
        writer.beginArray();

        // Bones.
        for (const EditorBoneData& bone : animation->fBones) {
            writer.beginObject();
            bone.serialize(writer);
            writer.endObject();
        }

        writer.endArray();

        writer.endObject();
    }

    static EditorAnimation Deserialize(const char* filename) {
        using namespace skjson;

        // Read the file.
        SkFILEStream rstream(filename);
        std::vector<char> buffer;
        buffer.resize(rstream.getLength());
        rstream.read(buffer.data(), buffer.size());

        // Read the JSON.
        DOM dom(buffer.data(), buffer.size());

        EditorAnimation animation;
        ObjectValue animationJson = dom.root().as<ObjectValue>();

        // Width, height, and speed.
        const NumberValue* widthJson = animationJson["width"];
        const NumberValue* heightJson = animationJson["height"];
        const NumberValue* speedJson = animationJson["speed"];
        animation.fSize.set(static_cast<int>(**widthJson), static_cast<int>(**heightJson));
        animation.fSpeed = **speedJson;

        // Bones.
        const ArrayValue* bonesJson = animationJson["bones"];
        for (size_t i = 0; i < bonesJson->size(); i ++) {
            const ObjectValue* boneJson = (*bonesJson)[i];
            animation.fBones.push_back(EditorBoneData::Deserialize(*boneJson));
        }

        return animation;
    }
};

#endif
