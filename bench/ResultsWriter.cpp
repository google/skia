/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Helper functions for result writing operations.
 */

#include "ResultsWriter.h"
#include "SkString.h"
#include "SkTArray.h"

Json::Value* SkFindNamedNode(Json::Value* root, const char name[]) {
    Json::Value* search_results = NULL;
    for(Json::Value::iterator iter = root->begin();
            iter!= root->end(); ++iter) {
        if(SkString(name).equals((*iter)["name"].asCString())) {
            search_results = &(*iter);
            break;
        }
    }

    if(search_results != NULL) {
        return search_results;
    } else {
        Json::Value* new_val = &(root->append(Json::Value()));
        (*new_val)["name"] = name;
        return new_val;
    }
}

Json::Value SkMakeBuilderJSON(const SkString &builderName) {
    static const int kNumKeys = 6;
    static const char* kKeys[kNumKeys] = {
        "role", "os", "model", "gpu", "arch", "configuration"};
    Json::Value builderData;

    if (!builderName.isEmpty()) {
        SkTArray<SkString> splitBuilder;
        SkStrSplit(builderName.c_str(), "-", &splitBuilder);
        SkASSERT(splitBuilder.count() >= kNumKeys);
        for (int i = 0; i < kNumKeys && i < splitBuilder.count(); ++i) {
            builderData[kKeys[i]] = splitBuilder[i].c_str();
        }
        builderData["builderName"] = builderName.c_str();
        if (kNumKeys < splitBuilder.count()) {
            SkString extras;
            for (int i = kNumKeys; i < splitBuilder.count(); ++i) {
                extras.append(splitBuilder[i]);
                if (i != splitBuilder.count() - 1) {
                    extras.append("-");
                }
            }
            builderData["badParams"] = extras.c_str();
        }
    } 
    return builderData;
}
