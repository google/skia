/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Helper functions for result writing operations.
 */

#include "ResultsWriter.h"

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

